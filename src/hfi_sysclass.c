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

#include "hfi_sysclass.h"
#include "psm2_nccl_debug.h"

#include <stdio.h>
#include <stdlib.h>

#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

ssize_t hfi_sysclass_rd(int dev, const char *attr, char *out, size_t len)
{
	int fd;
	char path[PATH_MAX];

	int prct = snprintf(path, PATH_MAX, HFI_SYSCLASS_FMT"/%s", dev, attr);
	if (prct >= PATH_MAX)
		return -1;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return -2;

	ssize_t rdct = read(fd, out, len);
	PSM_DBG("dev=%d,attr=%s,len=%zu,rdct=%zd", dev, attr, len, rdct);
	if (rdct < 0 || rdct >= len) {
		// Read error or too much data, no room for null-terminator
		close(fd);
		return -3;
	}

	out[rdct] = '\0';
	PSM_DBG("dev=%d,attr=%s,out=%s", dev, attr, out);
	close(fd);
	return rdct;
}

// hfi1_<dev>/uevent should be of the form NAME=<devname>
static const size_t UEVENT_MAX = 256;

int hfi_sysclass_get_devname(int dev, char **name)
{
	char tmpname[UEVENT_MAX];
	*name = NULL;

	if (hfi_sysclass_rd(dev, "uevent", tmpname, UEVENT_MAX) < 0) {
		return -1;
	}

	size_t newlen = strnlen(tmpname, UEVENT_MAX);
	if (newlen < 5 || strncmp(tmpname, "NAME=", 5) != 0)
		return -2;

	*name = malloc(newlen + 1);
	if (!*name)
		return -3;
	strncpy(*name, tmpname + 5, newlen);
	return 1;
}

int hfi_sysclass_get_pciPath(int dev, char **path)
{
	char devpath[PATH_MAX];
	struct stat sb = {0};
	*path = NULL;

	int prct = snprintf(devpath, sizeof(devpath), HFI_SYSCLASS_FMT"/device", dev);
	if (prct >= PATH_MAX)
		return -1;

	// Test that devpath exists and is a symlink
	if (lstat(devpath, &sb) == -1) {
		return -2;
	}

	// No way to tell how long the resolved path will be, only safe option is to allocate PATH_MAX.
	*path = malloc(PATH_MAX);
	if (!*path)
		return -3;

	char *resolved = realpath(devpath, *path);
	if (!resolved) {
		free(*path);
		*path = NULL;
		return -4;
	}

	return 1;
}
