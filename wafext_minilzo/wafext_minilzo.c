#include "wafext_minilzo.h"
#include "../minilzo/minilzo.h"

static int uncompress_minilzo(struct waf_archive_setup *s, unsigned char *restored, const unsigned char *transformed, int size)
{
	lzo_uint restored_size;
	lzo1x_decompress(transformed, size, restored, &restored_size, NULL);
	return restored_size;
}

static struct waf_archive_setup minilzo_setup =
{
	32 * 1024,
	32 * 1024 + 32 * 1024 / 16 + 64 + 3,
	uncompress_minilzo,
	{ 'M', 'L', 'Z', 'O' },
};

struct waf_archive_setup* wafext_setup_minilzo(void)
{
	lzo_init();
	return &minilzo_setup;
}
