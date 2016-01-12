#include "archive_setup.h"
#include <string.h>
#include <assert.h>

static int unit_transform(struct archive_setup *setup, unsigned char *dest, const unsigned char *src, int size)
{
	assert(dest != NULL);
	assert(src != NULL);
	assert(size >= 0);

	if (size > 0)
		memcpy(dest, src, size);

	return size;
}

static struct archive_setup default_setup =
{
	32 * 1024,
	32 * 1024,
	unit_transform,
	{ 'R', 'A', 'W', 0 },
};

struct archive_setup* archive_setup_default(void)
{
	return &default_setup;
}
