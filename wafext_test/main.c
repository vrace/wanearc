#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../wafext/wafext.h"

static int unit_restore(struct waf_archive_setup *setup, unsigned char *restored, const unsigned char *transformed, int size)
{
	memcpy(restored, transformed, size);
	return size;
}

static struct waf_archive_setup unit_setup =
{
	32 * 1024,
	32 * 1024,
	unit_restore,
	{ 'R', 'A', 'W', 0 },
};

int main(void)
{
	struct waf_archive *arc;

	arc = waf_open_archive("../wanearc/data.waf", &unit_setup);
	if (arc)
	{
		printf("Archive opened!\n");
		waf_close_archive(arc);
	}
	else
	{
		printf("Unable to open archive.\n");
	}

	return 0;
}
