/* BEGIN_ICS_COPYRIGHT7 ****************************************

Copyright (c) 2021, Cornelis Networks

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Cornelis Networks nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

** END_ICS_COPYRIGHT7   ****************************************/

/* [ICS VERSION STRING: unknown] */

#include <nccl_net.h>
#include <psm2.h>
#include <psm2_mq.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <unistd.h>
#include "hfi_sysclass.h"
#include "psm2_nccl_debug.h"

/**
 * Information used by send comm to connect to receive comm.
 */
typedef struct {
	psm2_epid_t epid;
	uint64_t tag;
} psm2comm_handle_t;

typedef enum {
	ReqSend = 1,
	ReqRecv = 2
} comm_req_type;

typedef struct psm2comm psm2comm_t;
/**
 * Stores info needed to test an outstanding isend, irecv for completion.
 */
typedef struct comm_req {
	int used;
	psm2comm_t *comm;
	psm2_ep_t ep;
	psm2_mq_t mq;
	psm2_mq_req_t req;
	comm_req_type type;
	int done;
	int recv_size;
} comm_req_t;

#define NUM_REQUESTS NCCL_NET_MAX_REQUESTS

typedef struct {
	psm2_ep_t ep;
	psm2_epid_t epid;
	unsigned int refcount;
} shared_ep_t;

static int use_shared_ep = 1;
static int use_gpudirect = 0;

shared_ep_t shared_ep = {0};


/**
 * Defines info needed to send to/receive from one other NCCL rank.
 * Communicators may either have their own EPs or share a PSM2 EP with all other communicators.
 */
typedef struct psm2comm {
	psm2_ep_t ep;
	psm2_epid_t epid;
	psm2_mq_t mq;
	shared_ep_t *shared_ep;
	psm2_epid_t rem_epid;
	// psm2comm is 1:1 with a remote psm2comm, so only need 1 epaddr
	psm2_epaddr_t rem_epaddr;
	uint64_t tag;
	comm_req_t requests[NCCL_NET_MAX_REQUESTS];
} psm2comm_t;



static int psm2comm_init_ep(int dev, psm2_uuid_t uuid, psm2comm_t *comm);
static void dump_comm(const psm2comm_t *comm, const char *caller);

/**
 * Used by both .listen() (creates receive EP) and .connect() (creates send EP).
 * @param [out] ep
 * @param [out] epid
 */
static int psm2comm_init(int dev, psm2_uuid_t uuid, psm2comm_t **comm);
static int psm2comm_fini(psm2comm_t *comm);

unsigned int plugin_logLevel = 0;
ncclDebugLogger_t pluginLogFunction;

// Timeouts are in nanoseconds
static const int64_t COMM_EP_CLOSE_TIMEOUT = 5e9; // 5 seconds
static const int64_t COMM_EP_CONNECT_TIMEOUT = 5e9; // 5 seconds

// MAX_DEV is here as a sanity check. 3 comes from psm/psm_config.h:HFI_MAX_RAILS
// NCCL should call .devices() to get actual count on system.
static const int MAX_DEV = 3;

static psm2_uuid_t jobkey;
static int use_nccl_dev_num = 1;

static uint64_t next_tag = 0;


static int psm2_nccl_init_logging(ncclDebugLogger_t logFunction)
{
	char *llvar = getenv("PSM2_NCCL_LOG_LEVEL");
	if (llvar) {
		char *end = NULL;
		unsigned long loglevel = strtoul(llvar, &end, 10);
		if (*end != '\0') {
			PSM_ERROR("[Error, pre-log-init] PSM2_NCCL_LOG_LEVEL \"%s\" is not an integer", llvar);
			return -1;
		} else if (loglevel > UINT_MAX) {
			PSM_ERROR("PSM2_NCCL_LOG_LEVEL (%lu) out of range. Max allowed value = %u", loglevel, UINT_MAX);
			return -2;
		}

		plugin_logLevel = (unsigned int)loglevel;
		if (plugin_logLevel > PSM2_NCCL_LOG_LEVEL_VERDBG) {
			fprintf(stderr, "[Error, pre-log-init] PSM2_NCCL_LOG_LEVEL (%u) too high. Maximum allowed value = %u",
				plugin_logLevel, PSM2_NCCL_LOG_LEVEL_VERDBG);
			plugin_logLevel = 0;
			return -3;
		}
	}

	pluginLogFunction = logFunction;
	return 0;
}

static void dump_comm(const psm2comm_t *comm, const char *caller)
{
	if (!comm) {
		PSM_DBG("[via %s] comm=%p", (caller? caller: "(nil)"), comm);
		return;
	}

	PSM_DBG("[via %s] comm=%p,epid=%"PRId64",rem_epid=%"PRId64",rem_epaddr=%p,tag=%"PRId64,
		(caller? caller: "(nil)"), comm, comm->epid, comm->rem_epid, comm->rem_epaddr, comm->tag);
}

static int psm2comm_init_ep(int dev, psm2_uuid_t uuid, psm2comm_t *comm)
{
	psm2_ep_t ep;
	psm2_epid_t epid;

	if (!use_shared_ep || !shared_ep.refcount) {
		struct psm2_ep_open_opts opts;
		if (use_nccl_dev_num)
			opts.unit = dev;

		int rc = psm2_ep_open_opts_get_defaults(&opts);
		if (rc != PSM2_OK) {
			PSM_ERROR("psm2_ep_open_opts_get_defaults(): rc=%d", rc);
			return ncclInternalError;
		}

		rc = psm2_ep_open(uuid, &opts, &ep, &epid);
		PSM_VERDBG("psm2_ep_open: rc=%d,ep=%p,epid=%"PRId64, rc, ep, epid);
		if (rc != PSM2_OK)
			return ncclInternalError;

		if (use_shared_ep) {
			shared_ep.ep = ep;
			shared_ep.epid = epid;
		}
	} else {
		ep = shared_ep.ep;
		epid = shared_ep.epid;
	}

	comm->ep = ep;
	comm->epid = epid;

	if (use_shared_ep) {
		comm->shared_ep = &shared_ep;
		shared_ep.refcount++;
	}

	return ncclSuccess;
}

static int psm2comm_init(int dev, psm2_uuid_t uuid, psm2comm_t **comm)
{
	PSM_VERDBG("dev=%d,comm=%p", dev, comm);

	assert(sizeof(psm2comm_handle_t) <= NCCL_NET_HANDLE_MAXSIZE);
	if (sizeof(psm2comm_handle_t) > NCCL_NET_HANDLE_MAXSIZE) {
		PSM_ERROR("NCCL_NET_HANDLE_MAXSIZE (%d) too small to fit psm2comm_handle_t (%zu)",
			NCCL_NET_HANDLE_MAXSIZE, sizeof(psm2comm_handle_t));
		return ncclInternalError;
	}

	*comm = calloc(1, sizeof(psm2comm_t));
	if (!*comm) {
		PSM_ERROR("malloc() comm failed");
		return ncclInternalError;
	}

	int rc = psm2comm_init_ep(dev, uuid, *comm);
	if (rc != ncclSuccess)
		goto bail;

	rc = psm2_mq_init((*comm)->ep, PSM2_MQ_ORDERMASK_NONE, NULL, 0, &(*comm)->mq);
	PSM_DBG("psm2_mq_init: rc=%d", rc);
	if (rc != PSM2_OK)
		goto bail;

	return ncclSuccess;

bail:
	psm2comm_fini(*comm);
	*comm = NULL;

	return ncclInternalError;
}

static int psm2comm_fini(psm2comm_t *comm)
{
	int64_t timeout = COMM_EP_CLOSE_TIMEOUT;
	int mqrc = psm2_mq_finalize(comm->mq);
	int eprc = PSM2_OK;
	if (mqrc != PSM2_OK)
		PSM_WARN("psm2_mq_finalize() rc != PSM2_OK; rc=%d", mqrc);

	if (comm->shared_ep)
		comm->shared_ep->refcount--;

	if (!comm->shared_ep || !comm->shared_ep->refcount) {
		eprc = psm2_ep_close(comm->ep, PSM2_EP_CLOSE_GRACEFUL, timeout);
		if (eprc != PSM2_OK)
			PSM_WARN("psm2_ep_close() rc != PSM2_OK; rc=%d", eprc);

		if (comm->shared_ep) {
			memset(comm->shared_ep, 0, sizeof(shared_ep_t));
		}
	}
	free(comm);

	return (mqrc == PSM2_OK && eprc == PSM2_OK? ncclSuccess: ncclInternalError);
}

ncclResult_t psm2_nccl_init(ncclDebugLogger_t logFunction)
{
	if (psm2_nccl_init_logging(logFunction) < 0)
		return ncclInternalError;


	memset(jobkey, 0, sizeof(psm2_uuid_t));

	char *euse_nccl_dev = getenv("PSM2_NCCL_USE_NCCL_DEV");
	if (euse_nccl_dev) {
		char *end = NULL;
		unsigned long val = strtoul(euse_nccl_dev, &end, 2);
		if (*end != '\0' || val > 1) {
			PSM_ERROR("PSM2_NCCL_USE_NCCL_DEV must be 0 or 1.");
			return ncclInternalError;
		}
		use_nccl_dev_num = val;
	}

	char *ehfi = getenv("HFI_UNIT");
	if (ehfi && use_nccl_dev_num) {
		PSM_WARN("HFI_UNIT is set but PSM2-NCCL is set to use device numbers passed in from NCCL."
			" This probably isn't what you want."
			" To let PSM2 decide which HFI to use or to use HFI_UNIT, set PSM2_NCCL_USE_NCCL_DEV=0.");
	}

	char *eshep = getenv("PSM2_NCCL_SHARED_EP");
	if (eshep) {
		char *end = NULL;
		unsigned long envval = strtoul(eshep, &end, 2);
		if (*end != '\0' || (envval != 0 && envval != 1)) {
			PSM_ERROR("PSM2_NCCL_SHARED_EP must be 0 or 1. \"%s\" is neither.", eshep);
			return ncclInternalError;
		}
		use_shared_ep = (int)envval;
	}

	char *euse_gdr = getenv("PSM2_NCCL_USE_GPUDIRECT");
	if (euse_gdr) {
		char *end = NULL;
		unsigned long val = strtoul(euse_gdr, &end, 2);
		if (*end != '\0' || val > 1) {
			PSM_ERROR("PSM2_NCCL_USE_GPUDIRECT must be 0 or 1.");
			return ncclInternalError;
		}
		use_gpudirect = val;
	}

	// PSM2 API does not allow for getting PSM2-CUDA/GPUDirect status before opening
	// an endpoint.
	// Assume that CUDA, GPUDirect are disabled but get the actual
	// value from the PSM2_CUDA and PSM2_GPUDIRECT envvars.
	int psm_cuda, psm_gpudirect;
	psm_cuda = psm_gpudirect = 0;
	char *ecuda = getenv("PSM2_CUDA");
	if (ecuda) {
		char *end = NULL;
		unsigned long val = strtoul(ecuda, &end, 2);
		if (*end != '\0' || (val > 1)) {
			PSM_ERROR("PSM2_CUDA must be 0 or 1.");
			return ncclInternalError;
		}
		psm_cuda = val;
	}

	char *egpudirect = getenv("PSM2_GPUDIRECT");
	if (egpudirect) {
		char *end = NULL;
		unsigned long val = strtoul(egpudirect, &end, 2);
		if (*end != '\0' || (val > 1)) {
			PSM_ERROR("PSM2_GPUDIRECT must be 0 or 1.");
			return ncclInternalError;
		}

		psm_gpudirect = val;
	}

	if (use_gpudirect && (!psm_cuda || !psm_gpudirect)) {
		PSM_WARN("PSM2_CUDA and/or PSM2_GPUDIRECT not set in environment or disabled. Disabling plugin GPUDirect support.")
		use_gpudirect = 0;
	}


	int ver_major = PSM2_VERNO_MAJOR;
	int ver_minor = PSM2_VERNO_MINOR;
	int rc = psm2_init(&ver_major, &ver_minor);
	PSM_DBG("rc=%d,ver_major=0x%X,ver_minor=0x%X", rc, ver_major, ver_minor);
	return (rc == PSM2_OK? ncclSuccess: ncclInternalError);
}

// Return the number of adapters.
ncclResult_t psm2_nccl_devices(int* ndev)
{
	uint32_t devcount;
	int rc = psm2_info_query(PSM2_INFO_QUERY_NUM_UNITS, (void*)&devcount, 0, NULL);
	PSM_DBG("rc=%d, devcount=%u", rc, devcount);
	if (rc != PSM2_OK || devcount > INT_MAX)
		return ncclInternalError;

	*ndev = (int) devcount;
	return ncclSuccess;
}

static int guid_str_to_u64(const char *guid, uint64_t *guidn)
{
	unsigned short int guid_words[4];

	int match = sscanf(guid, "%hx:%hx:%hx:%hx", &guid_words[0], &guid_words[1], &guid_words[2], &guid_words[3]);
	if (match != 4)
		return 1;

	*guidn = ((uint64_t)guid_words[0] << 48) | ((uint64_t)guid_words[1] << 32) |
		((uint64_t)guid_words[2] << 16) | ((uint64_t)guid_words[3]);

	return 0;
}

// Get various device properties.
ncclResult_t psm2_nccl_getProperties(int dev, ncclNetProperties_v4_t* props)
{
	int rc;
	char *name = NULL;
	// /sys/**/hfi1_<dev>/node_guid should be "xxxx:xxxx:xxxx:xxxx" => 20 chars + '\0'
	const size_t GUID_LEN = 32;
	char guid[GUID_LEN];
	guid[GUID_LEN - 1] = '\0';
	char *pciPath = NULL;

	if (dev < 0 || dev > MAX_DEV)
		return ncclInvalidArgument;

	rc = hfi_sysclass_get_devname(dev, &name);
	if (rc <= 0) {
		PSM_DBG("hfi_sysclass_get_devname(): dev=%d,rc=%d", dev, rc);
		goto bail;
	}
	PSM_DBG("dev=%d,name=%s", dev, name);
	props->name = name;

	rc = hfi_sysclass_get_pciPath(dev, &pciPath);
	if (rc <= 0) {
		PSM_DBG("hfi_sysclass_get_pciPath(): dev=%d,rc=%d", dev, rc);
		goto bail;
	}
	PSM_DBG("dev=%d,pciPath=%s", dev, pciPath);
	props->pciPath = pciPath;

	rc = hfi_sysclass_rd(dev, "node_guid", guid, GUID_LEN);
	if (rc <= 0 || guid_str_to_u64(guid, &props->guid))
		goto bail;
	PSM_DBG("dev=%d,guid=0x%"PRIX64, dev, props->guid);

	props->ptrSupport = NCCL_PTR_HOST;
	if (use_gpudirect) {
		props->ptrSupport |= NCCL_PTR_CUDA;
	}
	PSM_DBG("dev=%d,ptrSupport=0x%X", dev, props->ptrSupport);

	props->speed = 100e3;

	uint32_t ports = 0;
	rc = psm2_info_query(PSM2_INFO_QUERY_NUM_PORTS, (void*)&ports, 0, NULL);
	PSM_DBG("dev=%d,rc=%d,ports=%u", dev, rc, ports);
	if (rc != PSM2_OK) {
		PSM_WARN("Querying for number of ports returned an error. Port count query is informational only.");
	}

	if (ports > 1) {
		PSM_WARN("PSM2-NCCL does not handle devices with more than one port.");
	}
	props->port = 1;

	psm2_info_query_arg_t qa = {0};
	qa.unit = dev;
	uint32_t numctxts = 0;
	rc = psm2_info_query(PSM2_INFO_QUERY_NUM_FREE_CONTEXTS, (void*)&numctxts, 1, &qa);
	PSM_DBG("dev=%d,rc=%d,numctxts=%u", dev, rc, numctxts);
	if (rc != PSM2_OK)
		goto bail;
	props->maxComms = numctxts;

	return ncclSuccess;

bail:
	if (name) {
		free(name);
		props->name = NULL;
	}

	if (pciPath) {
		free(pciPath);
		props->pciPath = NULL;
	}

	return ncclInternalError;
}

// Create a receiving object and provide a handle to connect to it. The
// handle can be up to NCCL_NET_HANDLE_MAXSIZE bytes and will be exchanged
// between ranks to create a connection.
/**
 * PSM2 doesn't have the concept of a listening endpoint or listening socket.
 * So *listenComm returned here becomes the recvComm in psm2_nccl_accept().
 *
 * @param [out] handle @c psm2comm_handle_t to connect with this listener
 * @param [out] listenComm @c psm2comm_t
 */
ncclResult_t psm2_nccl_listen(int dev, void* handle, void** listenComm)
{
	PSM_DBG("dev=%d,handle=%p,listenComm=%p", dev, handle, listenComm);
	*listenComm = NULL;

	assert((next_tag + 1) < ULLONG_MAX);
	if ((next_tag + 1) >= ULLONG_MAX) {
		return ncclInternalError;
	}

	psm2comm_t *comm = NULL;
	psm2comm_handle_t *h = (psm2comm_handle_t*)handle;
	int rc = psm2comm_init(dev, jobkey, &comm);
	dump_comm(comm, __func__);
	if (comm) {
		comm->tag = next_tag++;
		h->epid = comm->epid;
		h->tag = comm->tag;
	}
	*listenComm = comm;

	return rc;
}

/**
 * Connect to @c handle and return a sender comm object.
 * @param [in] handle @psm2comm_handle_t
 * @param [out] sendComm @c psm2comm_t
 */
// Connect to a handle and return a sending comm object for that peer.
ncclResult_t psm2_nccl_connect(int dev, void* handle, void** sendComm)
{
	PSM_DBG("dev=%d,handle=%p,sendComm=%p", dev, handle, sendComm);
	*sendComm = NULL;

	psm2comm_t *comm = NULL;
	int rc = psm2comm_init(dev, jobkey, &comm);
	dump_comm(comm, __func__);
	if (rc != ncclSuccess)
		goto bail;

	*sendComm = comm;

	// Now connect to remote "listener" endpoint
	psm2comm_handle_t *h = (psm2comm_handle_t*)handle;
	comm->rem_epid = h->epid;
	comm->tag = h->tag;
#define PSM2_NCCL_COMM_CONNECT_ARRAY_SIZE 1
	psm2_epid_t epids[PSM2_NCCL_COMM_CONNECT_ARRAY_SIZE] = { comm->rem_epid };
	int mask[PSM2_NCCL_COMM_CONNECT_ARRAY_SIZE] = { 1 };
	psm2_error_t conn_errors[PSM2_NCCL_COMM_CONNECT_ARRAY_SIZE] = { 0 };
	psm2_epaddr_t epaddrs[PSM2_NCCL_COMM_CONNECT_ARRAY_SIZE] = { 0 };

	rc = psm2_ep_connect(comm->ep, PSM2_NCCL_COMM_CONNECT_ARRAY_SIZE,
		epids, mask, conn_errors, epaddrs, COMM_EP_CONNECT_TIMEOUT);
	PSM_DBG("epids[0]=%"PRId64",rc=%d,conn_errors[0]=%d", epids[0], rc, conn_errors[0]);

	if (rc != PSM2_OK || conn_errors[0] != PSM2_OK) {
		rc = ncclInternalError;
		goto bail;
	}
	comm->rem_epaddr = epaddrs[0];

	return ncclSuccess;
bail:
	if (comm) {
		psm2comm_fini(comm);
		*sendComm = NULL;
	}
	return rc;
}

/**
 * PSM2 does not have the idea of a "listening socket", just make
 * the listener become the receiver.
 * @param [in] listenComm @c psm2comm_t
 * @param [out] recvComm @c psm2comm_t
 */
ncclResult_t psm2_nccl_accept(void* listenComm, void** recvComm)
{
	PSM_DBG("listenComm=%p,recvComm=%p", listenComm, recvComm);
	dump_comm(listenComm, __func__);
	*recvComm = listenComm;
	return ncclSuccess;
}

// Register/Deregister memory. Comm can be either a sendComm or a recvComm.
// Type is either NCCL_PTR_HOST or NCCL_PTR_CUDA.
ncclResult_t psm2_nccl_regMr(void* comm, void* data, int size, int type, void** mhandle)
{
	if (mhandle)
		*mhandle = NULL;
	return ncclSuccess;
}

ncclResult_t psm2_nccl_deregMr(void* comm, void* mhandle)
{
	return ncclSuccess;
}

static comm_req_t * get_req(psm2comm_t *comm, int type)
{
	assert(type == ReqSend || type == ReqRecv);
	int i;
	comm_req_t *r = NULL;
	for (i = 0; i < NUM_REQUESTS; i++) {
		 if (!comm->requests[i].used) {
			r = comm->requests + i;
			break;
		}
	}
	if (!r)
		return r;

	memset(r, 0, sizeof(comm_req_t));
	r->type = type;
	r->used = 1;
	r->comm = comm;
	r->ep = comm->ep;
	r->mq = comm->mq;
	PSM_VERDBG("r=%p,r.type=%d", r, r->type);
	return r;
}

static void put_req(comm_req_t *r)
{
	PSM_VERDBG("r=%p,r.type=%d", r, r->type);
	memset(r, 0, sizeof(comm_req_t));
}

// Asynchronous send to a peer.
// May return request == NULL if the call cannot be performed (or would block)
ncclResult_t psm2_nccl_isend(void* sendComm, void* data, int size, void* mhandle, void** request)
{
	*request = NULL;
	psm2comm_t *comm = (psm2comm_t*)sendComm;
	PSM_VERDBG("mq=%p,rem_epid=%"PRId64",data=%p,size=%d,mhandle=%p", comm->mq, comm->rem_epid, data, size, mhandle);
	comm_req_t *r = get_req(sendComm, ReqSend);
	if (!r) {
		// No requests available, signal NCCL
		return ncclSuccess;
	}

	int rc = psm2_mq_isend(comm->mq, comm->rem_epaddr, PSM2_MQ_FLAG_GDRCPY_ONLY, comm->tag, data, size, (void*)r, &r->req);
	PSM_VERDBG("rc=%d,req=%p", rc, r->req);
	if (rc != PSM2_OK) {
		PSM_ERROR("isend() failed; rc=%d", rc);
		put_req(r);
		return ncclInternalError;
	}

	*request = r;
	return ncclSuccess;
}

// Asynchronous recv from a peer.
// May return request == NULL if the call cannot be performed (or would block)
ncclResult_t psm2_nccl_irecv(void* recvComm, void* data, int size, void* mhandle, void** request)
{
	*request = NULL;
	psm2comm_t *comm = (psm2comm_t*)recvComm;
	PSM_VERDBG("mq=%p,epid=%"PRId64",data=%p,size=%d,mhandle=%p", comm->mq, comm->epid, data, size, mhandle);
	comm_req_t *r = get_req(recvComm, ReqRecv);
	if (!r) {
		// No requests available, signal NCCL
		return ncclSuccess;
	}

	int rc = psm2_mq_irecv(comm->mq, comm->tag, ~((uint64_t)0), PSM2_MQ_FLAG_GDRCPY_ONLY, data, size, (void*)r, &r->req);
	PSM_VERDBG("rc=%d,req=%p", rc, r->req);
	if (rc != PSM2_OK) {
		PSM_ERROR("irecv() failed; rc=%d", rc);
		put_req(r);
		return ncclInternalError;
	}

	*request = r;
	return ncclSuccess;
}

// Perform a flush/fence to make sure all data received with NCCL_PTR_CUDA is
// visible to the GPU
ncclResult_t psm2_nccl_iflush(void* recvComm, void* data, int size, void* mhandle, void** request)
{
	return ncclSuccess;
}

static int mq_progress_loop(psm2_mq_t mq)
{
	int rc;
	int completed = 0;

	do {
		psm2_mq_req_t mqr = {0};
		psm2_mq_status_t s = {0};
		rc = psm2_mq_ipeek(mq, &mqr, NULL);
		if (rc == PSM2_MQ_INCOMPLETE) {
			PSM_VERDBG("%s:mq=%p,completed=%d", __func__, mq, completed);
			return 0;
		}

		assert(rc == PSM2_OK);
		if (rc != PSM2_OK) {
			PSM_VERDBG("%s:mq=%p,psm2_mq_ipeek returned with rc=%d is not OK nor INCOMPLETE", __func__, mq, rc);
			return rc;
		}
		// Retire/free the PSM2 request object
		int testrc = psm2_mq_test(&mqr, &s);
		assert(testrc == PSM2_OK);
		if (testrc != PSM2_OK) {
			PSM_VERDBG("%s:mq=%p,testrc=%d", __func__, mq, testrc);
			return testrc;
		}
		completed++;
		comm_req_t *r = (comm_req_t*) s.context;
		assert(r->used);
		if (!r->used) {
			PSM_VERDBG("%s:r=%p, request not used", __func__, r);
			return -1;
		}
		assert(!r->done);
		if (r->done) {
			PSM_VERDBG("%s:r=%p, request is done", __func__, r);
			return -1;
		}
		r->done = 1;
		if (r->type == ReqRecv) {
			assert(s.nbytes <= INT_MAX);
			if (s.nbytes > INT_MAX) {
				PSM_VERDBG("%s:r=%p, nbytes=%d > INT_MAX", __func__, r, (int)s.nbytes);;
				return -1;
			}
			r->recv_size = (int)s.nbytes;
		}
		PSM_VERDBG("%s: r=%p,r.comm=%p,r.done=%d,r.type=%d,r.recv_size=%d", __func__,
			r, r->comm, r->done, r->type, r->recv_size);
	} while (rc == PSM2_OK);
	PSM_VERDBG("%s completed=%d", __func__, completed);

	return -1;
}

/**
 * Test whether a request is complete.
 *
 * @param r @c comm_req_t request being tested
 * @param [out] done if request has completed; will not be modified if r->used is 0
 * @param [out] size only applicable to receive requests
 */
ncclResult_t psm2_nccl_test(void* request, int* done, int* size)
{
	comm_req_t *r = (comm_req_t*)request;

	// NCCL may retest communications that have already completed; this is not an error.
	if (!r->used)
		return ncclSuccess;

	int rc = mq_progress_loop(r->mq);
	if (rc != ncclSuccess)
		return ncclInternalError;

	if (!r->done) {
		*done = 0;
		return ncclSuccess;
	}

	PSM_VERDBG("test r=%p,r.ep=%p,r.mq=%p,r.req=%p,r.type=%d,r.recv_size=%d,rc=%d",
		r, r->ep, r->mq, r->req, r->type, r->recv_size, rc);

	if (r->type == ReqRecv) {
		*size = r->recv_size;
	}

	put_req(r);
	if (rc != PSM2_OK)
		return ncclInternalError;

	*done = 1;
	return ncclSuccess;
}

// Close and free send/recv comm objects
ncclResult_t psm2_nccl_closeSend(void* sendComm)
{
	PSM_DBG("sendComm=%p", sendComm);
	dump_comm((psm2comm_t*)sendComm, __func__);
	return psm2comm_fini((psm2comm_t*)sendComm);
}

ncclResult_t psm2_nccl_closeRecv(void* recvComm)
{
	PSM_DBG("recvComm=%p", recvComm);
	dump_comm((psm2comm_t*)recvComm, __func__);
	return psm2comm_fini((psm2comm_t*)recvComm);
}

ncclResult_t psm2_nccl_closeListen(void* listenComm)
{
	return ncclSuccess;
}

ncclNet_t NCCL_PLUGIN_SYMBOL =
{
	.name = "psm2-nccl",
	// Initialize the network.
	.init = psm2_nccl_init,
	// Return the number of adapters.
	.devices = psm2_nccl_devices,
	// Get various device properties.
	.getProperties = psm2_nccl_getProperties,
	// Create a receiving object and provide a handle to connect to it. The
	// handle can be up to NCCL_NET_HANDLE_MAXSIZE bytes and will be exchanged
	// between ranks to create a connection.
	.listen = psm2_nccl_listen,
	// Connect to a handle and return a sending comm object for that peer.
	.connect = psm2_nccl_connect,
	// Finalize connection establishment after remote peer has called connectHandle
	.accept = psm2_nccl_accept,
	// Register/Deregister memory. Comm can be either a sendComm or a recvComm.
	// Type is either NCCL_PTR_HOST or NCCL_PTR_CUDA.
	.regMr = psm2_nccl_regMr,
	.deregMr = psm2_nccl_deregMr,
	// Asynchronous send to a peer.
	// May return request == NULL if the call cannot be performed (or would block)
	.isend = psm2_nccl_isend,
	// Asynchronous recv from a peer.
	// May return request == NULL if the call cannot be performed (or would block)
	.irecv = psm2_nccl_irecv,
	// Perform a flush/fence to make sure all data received with NCCL_PTR_CUDA is
	// visible to the GPU
	.iflush = psm2_nccl_iflush,
	// Test whether a request is complete. If size is not NULL, it returns the
	// number of bytes sent/received.
	.test = psm2_nccl_test,
	// Close and free send/recv comm objects
	.closeSend = psm2_nccl_closeSend,
	.closeRecv = psm2_nccl_closeRecv,
	.closeListen = psm2_nccl_closeListen
};
