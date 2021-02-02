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

# psm2-nccl requires nccl_net.h, nccl_net.h is in a nccl development clone.
NCCL_SRC_DIR := ../nccl/src
NCCL_INCLUDE := $(NCCL_SRC_DIR)/include
CUDA_INCLUDE := /usr/local/cuda/include

BUILDDIR := .
SONAME := $(BUILDDIR)/libnccl-net.so
LIBSRC := src/psm2_nccl_net.c src/hfi_sysclass.c
LIBOBJS := $(LIBSRC:.c=.o)

INCLUDES := -Isrc/include -I$(PSM2_INCLUDE) -I$(NCCL_INCLUDE) -I$(CUDA_INCLUDE)
DEBUG := 1

.PHONY : all clean
.DEFAULT_GOAL := all

BASECFLAGS := -Wall
ifeq ($(DEBUG),1)
BASECFLAGS += -g -Og
else
BASECFLAGS += -O2
endif

BASELDFLAGS := -lpsm2 -lnccl

%.o : %.c
	$(CC) $(INCLUDES) $(CFLAGS) $(BASECFLAGS) -c -fPIC -o $@  $^

$(SONAME) : $(LIBOBJS)
	$(LD) -shared $(BASELDFLAGS) $(LDFLAGS) -o $@ $^

all : $(SONAME)

clean :
	-rm $(SONAME) $(LIBOBJS)
