#include "archive_setup_minilzo.h"
#include "../minilzo/minilzo.h"

void init_archive_setup_minilzo(void)
{
	lzo_init();
}

static int transform_minilzo(struct archive_setup_minilzo *s, unsigned char *dest, const unsigned char *src, int size)
{
	lzo_uint destsize = 0;
	lzo1x_1_compress(src, size, dest, &destsize, s->workmem);
	return (int)destsize;
}

struct archive_setup_minilzo archive_setup_minilzo =
{
	32 * 1024,
	32 * 1024 + 32 * 1024 / 16 + 64 + 3,
	(transform_func)transform_minilzo,
	{ 'M', 'L', 'Z', 'O' },
};
