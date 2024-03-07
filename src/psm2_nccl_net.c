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

const ncclNet_v4_t ncclNetPlugin_v4 =
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
