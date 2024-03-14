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

// NCCL Version from v2.4.2-1 to v2.5.7-1
const ncclNet_v2_t ncclNetPlugin_v2 = {
	.name = PSM2_NCCL_PLUGIN_NAME,
	.init = psm2_nccl_init,
	.devices = psm2_nccl_devices,
	.pciPath = psm2_nccl_pciPath_v2,
	.ptrSupport = psm2_nccl_ptrSupport_v2,
	.listen = psm2_nccl_listen,
	.connect = psm2_nccl_connect,
	.accept = psm2_nccl_accept,
	.regMr = psm2_nccl_regMr,
	.deregMr = psm2_nccl_deregMr,
	.isend = psm2_nccl_isend_v4,
	.irecv = psm2_nccl_irecv_v4,
	.flush = psm2_nccl_flush_v3,
	.test = psm2_nccl_test,
	.closeSend = psm2_nccl_closeSend,
	.closeRecv = psm2_nccl_closeRecv,
	.closeListen = psm2_nccl_closeListen
};

// NCCL Version from v2.6.4-1 to v2.7.8-1
const ncclNet_v3_t ncclNetPlugin_v3 = {
	.name = PSM2_NCCL_PLUGIN_NAME,
	.init = psm2_nccl_init,
	.devices = psm2_nccl_devices,
	.getProperties = psm2_nccl_getProperties_v4,
	.listen = psm2_nccl_listen,
	.connect = psm2_nccl_connect,
	.accept = psm2_nccl_accept,
	.regMr = psm2_nccl_regMr,
	.deregMr = psm2_nccl_deregMr,
	.isend = psm2_nccl_isend_v4,
	.irecv = psm2_nccl_irecv_v4,
	.flush = psm2_nccl_flush_v3,
	.test = psm2_nccl_test,
	.closeSend = psm2_nccl_closeSend,
	.closeRecv = psm2_nccl_closeRecv,
	.closeListen = psm2_nccl_closeListen
};

// NCCL Version from v2.8.3-1 to v2.11.4-1
const ncclNet_v4_t ncclNetPlugin_v4 = {
	.name = PSM2_NCCL_PLUGIN_NAME,
	.init = psm2_nccl_init,
	.devices = psm2_nccl_devices,
	.getProperties = psm2_nccl_getProperties_v4,
	.listen = psm2_nccl_listen,
	.connect = psm2_nccl_connect,
	.accept = psm2_nccl_accept,
	.regMr = psm2_nccl_regMr,
	.deregMr = psm2_nccl_deregMr,
	.isend = psm2_nccl_isend_v4,
	.irecv = psm2_nccl_irecv_v4,
	.iflush = psm2_nccl_iflush_v4,
	.test = psm2_nccl_test,
	.closeSend = psm2_nccl_closeSend,
	.closeRecv = psm2_nccl_closeRecv,
	.closeListen = psm2_nccl_closeListen
};

// NCCL Version from v2.12.10-1 to v2.12.7-1
const ncclNet_v5_t ncclNetPlugin_v5 = {
	.name = PSM2_NCCL_PLUGIN_NAME,
	.init = psm2_nccl_init,
	.devices = psm2_nccl_devices,
	.getProperties = psm2_nccl_getProperties_v6,
	.listen = psm2_nccl_listen,
	.connect = psm2_nccl_connect,
	.accept = psm2_nccl_accept,
	.regMr = psm2_nccl_regMr,
	.deregMr = psm2_nccl_deregMr,
	.isend = psm2_nccl_isend,
	.irecv = psm2_nccl_irecv,
	.iflush = psm2_nccl_iflush,
	.test = psm2_nccl_test,
	.closeSend = psm2_nccl_closeSend,
	.closeRecv = psm2_nccl_closeRecv,
	.closeListen = psm2_nccl_closeListen
};
// NCCL Version from v2.13.4-1 to v2.18.6-1
const ncclNet_v6_t ncclNetPlugin_v6 = {
	.name = PSM2_NCCL_PLUGIN_NAME,
	.init = psm2_nccl_init,
	.devices = psm2_nccl_devices,
	.getProperties = psm2_nccl_getProperties_v6,
	.listen = psm2_nccl_listen,
	.connect = psm2_nccl_connect,
	.accept = psm2_nccl_accept,
	.regMr = psm2_nccl_regMr,
	.regMrDmaBuf = psm2_nccl_regMrDmaBuf,
	.deregMr = psm2_nccl_deregMr,
	.isend = psm2_nccl_isend,
	.irecv = psm2_nccl_irecv,
	.iflush = psm2_nccl_iflush,
	.test = psm2_nccl_test,
	.closeSend = psm2_nccl_closeSend,
	.closeRecv = psm2_nccl_closeRecv,
	.closeListen = psm2_nccl_closeListen
};
// NCCL Version from v2.19.1-1 to v2.19.4-1
const ncclNet_v7_t ncclNetPlugin_v7 = {
	.name = PSM2_NCCL_PLUGIN_NAME,
	.init = psm2_nccl_init,
	.devices = psm2_nccl_devices,
	.getProperties = psm2_nccl_getProperties_v7,
	.listen = psm2_nccl_listen,
	.connect = psm2_nccl_connect_v8,
	.accept = psm2_nccl_accept_v8,
	.regMr = psm2_nccl_regMr,
	.regMrDmaBuf = psm2_nccl_regMrDmaBuf,
	.deregMr = psm2_nccl_deregMr,
	.isend = psm2_nccl_isend,
	.irecv = psm2_nccl_irecv,
	.iflush = psm2_nccl_iflush,
	.test = psm2_nccl_test,
	.closeSend = psm2_nccl_closeSend,
	.closeRecv = psm2_nccl_closeRecv,
	.closeListen = psm2_nccl_closeListen,
	.getDeviceMr = NULL,
	.irecvConsumed = NULL
};
// NCCL Version from v2.20.3-1
const ncclNet_v8_t ncclNetPlugin_v8 = {
	.name = PSM2_NCCL_PLUGIN_NAME,
	.init = psm2_nccl_init,
	.devices = psm2_nccl_devices,
	.getProperties = psm2_nccl_getProperties,
	.listen = psm2_nccl_listen,
	.connect = psm2_nccl_connect_v8,
	.accept = psm2_nccl_accept_v8,
	.regMr = psm2_nccl_regMr_v8,
	.regMrDmaBuf = psm2_nccl_regMrDmaBuf,
	.deregMr = psm2_nccl_deregMr,
	.isend = psm2_nccl_isend,
	.irecv = psm2_nccl_irecv,
	.iflush = psm2_nccl_iflush,
	.test = psm2_nccl_test,
	.closeSend = psm2_nccl_closeSend,
	.closeRecv = psm2_nccl_closeRecv,
	.closeListen = psm2_nccl_closeListen,
	.getDeviceMr = NULL,
	.irecvConsumed = NULL
};
