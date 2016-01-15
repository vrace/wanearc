#include "wafext_zlib.h"
#include "../zlib/zlib.h"

static int uncompress_zlib(struct waf_archive_setup *s, unsigned char *restored, const unsigned char *transformed, int size)
{
	uLongf restored_size = s->restored_size;
	if (uncompress(restored, &restored_size, transformed, size) == Z_OK)
		return (int)restored_size;
	return -1;
}

static struct waf_archive_setup zlib_setup =
{
	32 * 1024,
	0,  /* to be set later */
	uncompress_zlib,
	{ 'Z', 'L', 'I', 'B' },
};

struct waf_archive_setup* wafext_setup_zlib(void)
{
	zlib_setup.transformed_size = compressBound(zlib_setup.restored_size);
	return &zlib_setup;
}
