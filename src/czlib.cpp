/*
	Copyright (C) 2003-2005 Daniel Muller, dan at verliba dot cz
	Copyright (C) 2006-2015 Verlihub Team, info at verlihub dot net

	Verlihub is free software; You can redistribute it
	and modify it under the terms of the GNU General
	Public License as published by the Free Software
	Foundation, either version 3 of the license, or at
	your option any later version.

	Verlihub is distributed in the hope that it will be
	useful, but without any warranty, without even the
	implied warranty of merchantability or fitness for
	a particular purpose. See the GNU General Public
	License for more details.

	Please see http://www.gnu.org/licenses/ for a copy
	of the GNU General Public License.
*/

#include <zlib.h>
#include "czlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

namespace nVerliHub {
	namespace nUtils {

cZLib::cZLib():
	//inBufPos(0),
	inBufLen(ZLIB_BUFFER_SIZE),
	inLastLen(0),
	outBufLen(ZLIB_BUFFER_SIZE),
	outLastLen(0)
{
	outBuf = (char*)calloc(ZLIB_BUFFER_SIZE, 1);
	inBuf = (char*)calloc(ZLIB_BUFFER_SIZE, 1);
	memcpy(outBuf, "$ZOn|", ZON_LEN);
}

cZLib::~cZLib()
{
	if (outBuf)
		free(outBuf);

	if (inBuf)
		free(inBuf);
}

char *cZLib::Compress(const char *buffer, size_t len, size_t &outLen)
{
	/*
		check if we are compressing same data as last time
		then return last compressed buffer to save resources
	*/

	if (inLastLen && outLastLen && (len == inLastLen) && buffer && inBuf && (memcmp(buffer, inBuf, len) == 0)) {
		outLen = outLastLen;

		if (len <= outLastLen) // fall back already here
			return NULL;
		else
			return outBuf;
	}

	z_stream strm;
	memset((void*) &strm, '\0', sizeof(strm));

	if (inBufLen < len) // increase in buffer if not enough space
		for (; inBufLen < len; inBufLen += ZLIB_BUFFER_SIZE);

	char *new_buffer = (char*)realloc(inBuf, inBufLen);

	if (!new_buffer) { // todo: throw exception and log error
		free(inBuf);
		outLen = inLastLen = outLastLen = 0;
		return NULL;
	}

	inBuf = new_buffer;
	strm.zalloc = Z_NULL; // allocate deflate state
	strm.zfree = Z_NULL;
	strm.data_type = Z_TEXT;

	if (deflateInit(&strm, Z_BEST_COMPRESSION) != Z_OK) { // initialize
		outLen = inLastLen = outLastLen = 0;
		return NULL;
	}

	memcpy(inBuf, buffer, len); // copy data to in buffer

	if (outBufLen < len) // increase out buffer if not enough space
		for (; outBufLen < len; outBufLen += ZLIB_BUFFER_SIZE);

	strm.avail_in = (uInt)len;
	strm.next_in = (Bytef*)inBuf;
	strm.avail_out = (uInt)(outBufLen - ZON_LEN);
	strm.next_out = (Bytef*)(outBuf + ZON_LEN);

	if (deflate(&strm, Z_FINISH) != Z_STREAM_END) { // compress
		deflateEnd(&strm);
		outLen = inLastLen = outLastLen = 0;
		return NULL;
	}

	outLen = outLastLen = (strm.total_out + ZON_LEN);
	inLastLen = len;

	if (deflateEnd(&strm) != Z_OK) { // finalize
		outLen = inLastLen = outLastLen = 0;
		return NULL;
	}

	if (len <= outLen) // out data size is bigger or equal in data size, fall back to raw data
		return NULL;

	return outBuf;
}

/*
void cZLib::AppendData(const char *buffer, size_t len)
{
	if ((inBufLen - inBufPos) < len) // increase zlib buffer if not enough
		for (; (inBufLen - inBufPos) < len; inBufLen += ZLIB_BUFFER_SIZE);

	char *new_buffer = (char*)realloc(inBuf, inBufLen);

	if (new_buffer == NULL) { // todo: throw exception and log error
		free(inBuf);
		return;
	}

	inBuf = new_buffer;
	memcpy(inBuf + inBufPos, buffer, len);
	inBufPos += len;
}
*/

	}; // namespace nUtils
}; // namespace nVerliHub
