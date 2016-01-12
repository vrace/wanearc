#include "archive_setup_minilzo.h"
#include "../minilzo/minilzo.h"

#define LZO_ALLOC(var,size)\
	lzo_align_t __LZO_MMODEL var [((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t)]

static struct archive_setup_minilzo
{
	struct archive_setup base;
	LZO_ALLOC(workmem, LZO1X_1_MEM_COMPRESS);
};

static int transform_minilzo(struct archive_setup_minilzo *s, unsigned char *dest, const unsigned char *src, int size)
{
	lzo_uint destsize = 0;
	lzo1x_1_compress(src, size, dest, &destsize, s->workmem);
	return (int)destsize;
}

static struct archive_setup_minilzo minilzo_setup =
{
	32 * 1024,
	32 * 1024 + 32 * 1024 / 16 + 64 + 3,
	(transform_func)transform_minilzo,
	{ 'M', 'L', 'Z', 'O' },
};

struct archive_setup* archive_setup_minilzo(void)
{
	lzo_init();
	return (struct archive_setup*)&minilzo_setup;
}
