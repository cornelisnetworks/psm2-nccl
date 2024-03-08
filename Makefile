# BEGIN_ICS_COPYRIGHT8 ****************************************
#
# Copyright (c) 2021, Cornelis Networks
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright notice,
#       this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of Cornelis Networks nor the names of its contributors
#       may be used to endorse or promote products derived from this software
#       without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# END_ICS_COPYRIGHT8   ****************************************

#[ICS VERSION STRING: unknown]

PSM2_INCLUDE := /usr/include
PSM2_LIB := /usr/lib64

NCCL_SRC_DIR := ../nccl/src
ifneq ($(NCCL_HOME),)
NCCL_INCLUDE := $(NCCL_HOME)/include/
NCCL_LIB := $(NCCL_HOME)/lib/
endif

CUDA_HOME := /usr/local/cuda
CUDA_INCLUDE := $(CUDA_HOME)/include

BUILDDIR := .
SONAME := $(BUILDDIR)/libnccl-net.so
LIBSRC := src/psm2_nccl_net.c src/psm2_nccl_api.c src/hfi_sysclass.c
LIBOBJS := $(LIBSRC:.c=.o)

INCLUDES := -Isrc/include -Isrc/include/nccl_net $(addprefix -I,$(PSM2_INCLUDE) $(NCCL_INCLUDE) $(CUDA_INCLUDE))

# nccl_net.h is needed to build the plugin.
# NCCL source builds (all compatible versions) and libnccl-devel rpms (version
# >= 2.10.3) install nccl_net.h.
# But libnccl-devel rpms for version < 2.10.3 do not.
# So in that case, use nccl_net.h from nccl source tree if available.
ifneq ($(strip $(NCCL_SRC_DIR)),)
ifneq ($(strip $(wildcard $(NCCL_SRC_DIR)/include/nccl_net.h)),)
INCLUDES += -I"$(NCCL_SRC_DIR)/include"
endif
endif

DEBUG := 0

BASECFLAGS := -Wall -Werror=incompatible-pointer-types
ifeq ($(DEBUG),1)
BASECFLAGS += -g -Og
else
BASECFLAGS += -O3
endif

BASELDFLAGS := $(addprefix -L,$(PSM2_LIB) $(NCCL_LIB)) -lpsm2 -lnccl

.PHONY : all clean
.DEFAULT_GOAL := all

%.o : %.c
	$(CC) $(INCLUDES) $(BASECFLAGS) $(CFLAGS)  -c -fPIC -o $@  $^

$(SONAME) : $(LIBOBJS)
	$(LD) -shared $(BASELDFLAGS) $(LDFLAGS) -o $@ $^

all : $(SONAME)

clean :
	-rm $(SONAME) $(LIBOBJS)
