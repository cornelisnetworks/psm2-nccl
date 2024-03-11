/* BEGIN_ICS_COPYRIGHT7 ****************************************

Copyright (c) 2024, Cornelis Networks

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

#ifndef _PSM2_NCCL_API_H_INCLUDED_
#define _PSM2_NCCL_API_H_INCLUDED_

#include <psm2.h>
#include <psm2_mq.h>
#include <limits.h>
#include "net.h"

#define NUM_REQUESTS NCCL_NET_MAX_REQUESTS
#define SHARED_EP_MULTI_ERROR 0
#define SHARED_EP_MULTI_WARN 1
#define PSM2_NCCL_COMM_CONNECT_ARRAY_SIZE 1
// TODO: Change NCCL_NET_PSM2_MAX_RECVS to a value > 1, if we support grouped receive.
// Maximum number for grouped receive operations: Currently not supported
#define NCCL_NET_PSM2_MAX_RECVS 1

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

typedef struct {
	psm2_ep_t ep;
	psm2_epid_t epid;
	int unit;
	unsigned int refcount;
} shared_ep_t;

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

#endif
