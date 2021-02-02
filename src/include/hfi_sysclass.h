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

#ifndef _HFI_SYSCLASS_H_INCLUDED_
#define _HFI_SYSCLASS_H_INCLUDED_ 1

#include <unistd.h>

#define HFI_SYSCLASS_FMT "/sys/class/infiniband/hfi1_%d"

/**
 * Read up to @c len bytes from HFI_SYSCLASS_FMT_<dev>/<attr>
 * into @c out. Adds terminating '\0' to *out. It is an error if @c len
 * is not big enough to fit data from <attr> plus null-terminator.
 *
 * @return number of bytes read.
 * @retval >= 0 Success, 0 indicates EOF. @c out will be NULL-terminated either way.
 * @retval < 0, failure. Contents of @c are undefined.
 */
extern ssize_t hfi_sysclass_rd(int dev, const char *attr, char *out, size_t len);

/**
 * @return 1 on success, < 0 on failure. On success, @c *name will be NULL-terminated.
 * Contents of @c *name are undefined on failure.
 */
extern int hfi_sysclass_get_devname(int dev, char **name);

/**
 * @return 1 on success, <0 on failure. On success, @c *path will be NULL-terminated.
 * Contents of @c *path are undefined on failure.
 */
extern int hfi_sysclass_get_pciPath(int dev, char **path);

#endif /* _HFI_SYSCLASS_H_INCLUDED_ */
