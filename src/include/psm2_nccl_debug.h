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

#ifndef _PSM2_NCCL_DEBUG_H_INCLUDED_
#define _PSM2_NCCL_DEBUG_H_INCLUDED_ 1

#include <stdio.h>
#include <pthread.h>
#include <nccl_net.h>

// Defined by nccl/include/debug.h.
extern ncclDebugLogger_t pluginLogFunction;

extern unsigned int plugin_logLevel;

// _WARN and _ERROR are always displayed
#define PSM2_NCCL_LOG_LEVEL_WARN 0
#define PSM2_NCCL_LOG_LEVEL_ERROR 0
#define PSM2_NCCL_LOG_LEVEL_INFO 1
#define PSM2_NCCL_LOG_LEVEL_DBG 2
#define PSM2_NCCL_LOG_LEVEL_VERDBG 3

// Redirect log messages to nccl's own logging facility.
#define PSM_LOG_MSG(level, levelstr, fmt, ...) \
	if (plugin_logLevel >= (level)) { \
		if (level<=PSM2_NCCL_LOG_LEVEL_ERROR) \
			pluginLogFunction(NCCL_LOG_WARN, NCCL_NET, __FILE__, __LINE__, "PSM2 " fmt, ##__VA_ARGS__); \
		else \
			pluginLogFunction(NCCL_LOG_INFO, NCCL_NET, __FILE__, __LINE__, "PSM2 " fmt, ##__VA_ARGS__); \
	}

#define PSM_WARN(fmt, ...) PSM_LOG_MSG(PSM2_NCCL_LOG_LEVEL_WARN, "Warn", fmt, ##__VA_ARGS__)
#define PSM_ERROR(fmt, ...) PSM_LOG_MSG(PSM2_NCCL_LOG_LEVEL_ERROR, "Error", fmt, ##__VA_ARGS__)
#define PSM_INFO(fmt, ...) PSM_LOG_MSG(PSM2_NCCL_LOG_LEVEL_INFO, "Info", fmt, ##__VA_ARGS__)
#define PSM_DBG(fmt, ...) PSM_LOG_MSG(PSM2_NCCL_LOG_LEVEL_DBG, "Debug", fmt, ##__VA_ARGS__)
#define PSM_VERDBG(fmt, ...) PSM_LOG_MSG(PSM2_NCCL_LOG_LEVEL_VERDBG, "VerDebug", fmt, ##__VA_ARGS__)

#endif /* _PSM2_NCCL_DEBUG_H_INCLUDED_ */
