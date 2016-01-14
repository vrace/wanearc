#include "archive_setup_zlib.h"
#include "../zlib/zlib.h"

static int transform_zlib(struct archive_setup *setup, unsigned char *dest, const unsigned char *src, int size)
{
	uLongf destsize = setup->transform_size;
	if (compress(dest, &destsize, src, size) == Z_OK)
		return (int)destsize;

	return -1;
}

static struct archive_setup zlib_setup =
{
	32 * 1024,
	0,  /* to be set later */
	transform_zlib,
	{ 'Z', 'L', 'I', 'B' },
};

struct archive_setup* archive_setup_zlib(void)
{
	zlib_setup.transform_size = (int)compressBound(zlib_setup.src_size);
	return &zlib_setup;
}
