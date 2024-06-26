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

#ifndef _PSM2_NCCL_NET_H_INCLUDED_
#define _PSM2_NCCL_NET_H_INCLUDED_

#include "net.h"

// psm2_ncclNetProperties must be the latest ncclNetProperties_t structure
// that holds all the fields defined by all ncclNetProperties_vX_t structures.
typedef ncclNetProperties_v8_t psm2_ncclNetProperties;

#define PSM2_NCCL_PLUGIN_NAME "psm2-nccl"

ncclResult_t psm2_nccl_init(ncclDebugLogger_t logFunction);
ncclResult_t psm2_nccl_devices(int* ndev);
ncclResult_t psm2_nccl_pciPath_v2(int dev, char** path);
ncclResult_t psm2_nccl_ptrSupport_v2(int dev, int *supportedTypes);
ncclResult_t psm2_nccl_getProperties(int dev, psm2_ncclNetProperties* props);
ncclResult_t psm2_nccl_getProperties_v4(int dev, ncclNetProperties_v4_t* props);
ncclResult_t psm2_nccl_getProperties_v6(int dev, ncclNetProperties_v6_t* props);
ncclResult_t psm2_nccl_getProperties_v7(int dev, ncclNetProperties_v7_t* props);
ncclResult_t psm2_nccl_getProperties_v8(int dev, ncclNetProperties_v8_t* props);
ncclResult_t psm2_nccl_listen(int dev, void* handle, void** listenComm);
ncclResult_t psm2_nccl_connect(int dev, void* handle, void** sendComm);
ncclResult_t psm2_nccl_connect_v8(int dev, void* handle, void** sendComm, ncclNetDeviceHandle_v8_t** sendDevComm);
ncclResult_t psm2_nccl_accept(void* listenComm, void** recvComm);
ncclResult_t psm2_nccl_accept_v8(void* listenComm, void** recvComm, ncclNetDeviceHandle_v8_t** recvDevComm);
ncclResult_t psm2_nccl_regMr(void* comm, void* data, int size, int type, void** mhandle);
ncclResult_t psm2_nccl_regMr_v8(void* comm, void* data, size_t size, int type, void** mhandle);
ncclResult_t psm2_nccl_regMrDmaBuf(void* comm, void* data, size_t size, int type, uint64_t offset, int fd, void** mhandle);
ncclResult_t psm2_nccl_deregMr(void* comm, void* mhandle);
ncclResult_t psm2_nccl_isend(void* sendComm, void* data, int size, int tag, void* mhandle, void** request);
ncclResult_t psm2_nccl_isend_v4(void* sendComm, void* data, int size, void* mhandle, void** request);
ncclResult_t psm2_nccl_irecv(void* recvComm, int n, void** data, int* sizes, int* tags, void** mhandles, void** request);
ncclResult_t psm2_nccl_irecv_v4(void* recvComm, void* data, int size, void* mhandle, void** request);
ncclResult_t psm2_nccl_iflush(void* recvComm, int n, void** data, int* sizes, void** mhandles, void** request);
ncclResult_t psm2_nccl_iflush_v4(void* recvComm, void* data, int size, void* mhandle, void** request);
ncclResult_t psm2_nccl_flush_v3(void* recvComm, void* data, int size, void* mhandle);
ncclResult_t psm2_nccl_test(void* request, int* done, int* size);
ncclResult_t psm2_nccl_closeSend(void* sendComm);
ncclResult_t psm2_nccl_closeRecv(void* recvComm);
ncclResult_t psm2_nccl_closeListen(void* listenComm);

#endif
