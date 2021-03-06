/****************************************************************************
 * GC Zip Extension
 *
 * GC DVD Zip File Loader.
 *
 * The idea here is not to support every zip file on the planet!
 * The unzip routine will simply unzip the first file in the zip archive.
 *
 * For maximum compression, I'd recommend using 7Zip,
 *	7za a -tzip -mx=9 rom.zip rom.smc
 ****************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "gczip.h"

	int zipoffset = 0;
	z_stream zs;

    


int inflate_init(PKZIPHEADER* pkzip){
	/*** Prepare the zip stream ***/
	zipoffset = sizeof(PKZIPHEADER) + bswap16(pkzip->filenameLength) + bswap16(pkzip->extraDataLength);

	memset(&zs, 0, sizeof(z_stream));
	zs.zalloc = Z_NULL;
	zs.zfree = Z_NULL;
	zs.opaque = Z_NULL;
	zs.avail_in = 0;
	zs.next_in = Z_NULL;
	return inflateInit2(&zs, -MAX_WBITS);
}

int inflate_chunk(void* dst, void* src, int length){
	zs.avail_in = (length  - zipoffset);
	zs.next_in = (src + zipoffset);
	unsigned char buf[ZIPCHUNK];
	int res;
	int have, bufferoffset = 0;

	/*** Now inflate until input buffer is exhausted ***/
	do {
		zs.avail_out = ZIPCHUNK;
		zs.next_out = buf;

		res = inflate(&zs, Z_NO_FLUSH);

		if(res == Z_MEM_ERROR) {
			inflateEnd(&zs);
			return -1;
		}
		have = ZIPCHUNK - zs.avail_out;
		if (have) {
			/*** Copy to normal block buffer ***/
			memcpy(dst + bufferoffset, buf, have);
			bufferoffset += have;
		}
		if(res == Z_STREAM_END){
			inflateEnd(&zs);
			return bufferoffset;
		}
	}while(zs.avail_out == 0);

	zipoffset = 0;
	return bufferoffset;
}
