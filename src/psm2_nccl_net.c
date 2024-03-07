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

/* [ICS VERSION STRING: unknown] */

#include "psm2_nccl_net.h"
#include "nccl_net/net_v2.h"
#include "nccl_net/net_v3.h"

// TODO: We probably need to set typedef ncclNet_vX_t ncclNet_t; for NCCL Net version X.
// The old NCCL versions define only one structure ncclNet_t that corresponds to 
// ncclNet_v2_t or ncclNet_v3_t depending on the nccl version.
// Also starting from nccl version 2.19, backwards compatibility goes up to nccl net version 5:
// ncclNetPlugin_v4 has been removed from nccl_net.h header.
// We need to include "nccl_net/net_v4.h", so this code compiles.

const ncclNet_v2_t ncclNetPlugin_v2 = {
	.name = "psm2-nccl",
	.init = psm2_nccl_init,
	.devices = psm2_nccl_devices,
	.pciPath = psm2_nccl_pciPath_v2,
	.ptrSupport = psm2_nccl_ptrSupport_v2,
	.listen = psm2_nccl_listen,
	.connect = psm2_nccl_connect,
	.accept = psm2_nccl_accept,
	.regMr = psm2_nccl_regMr,
	.deregMr = psm2_nccl_deregMr,
	.isend = psm2_nccl_isend,
	.irecv = psm2_nccl_irecv,
	.flush = psm2_nccl_flush_v3,
	.test = psm2_nccl_test,
	.closeSend = psm2_nccl_closeSend,
	.closeRecv = psm2_nccl_closeRecv,
	.closeListen = psm2_nccl_closeListen
};

const ncclNet_v3_t ncclNetPlugin_v3 = {
	.name = "psm2-nccl",
	.init = psm2_nccl_init,
	.devices = psm2_nccl_devices,
	.getProperties = psm2_nccl_getProperties,
	.listen = psm2_nccl_listen,
	.connect = psm2_nccl_connect,
	.accept = psm2_nccl_accept,
	.regMr = psm2_nccl_regMr,
	.deregMr = psm2_nccl_deregMr,
	.isend = psm2_nccl_isend,
	.irecv = psm2_nccl_irecv,
	.flush = psm2_nccl_flush_v3,
	.test = psm2_nccl_test,
	.closeSend = psm2_nccl_closeSend,
	.closeRecv = psm2_nccl_closeRecv,
	.closeListen = psm2_nccl_closeListen
};

const ncclNet_v4_t ncclNetPlugin_v4 = {
	.name = "psm2-nccl",
	.init = psm2_nccl_init,
	.devices = psm2_nccl_devices,
	.getProperties = psm2_nccl_getProperties,
	.listen = psm2_nccl_listen,
	.connect = psm2_nccl_connect,
	.accept = psm2_nccl_accept,
	.regMr = psm2_nccl_regMr,
	.deregMr = psm2_nccl_deregMr,
	.isend = psm2_nccl_isend,
	.irecv = psm2_nccl_irecv,
	.iflush = psm2_nccl_iflush_v4,
	.test = psm2_nccl_test,
	.closeSend = psm2_nccl_closeSend,
	.closeRecv = psm2_nccl_closeRecv,
	.closeListen = psm2_nccl_closeListen
};
