#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../wafext/wafext.h"
#include "../wafext_minilzo/wafext_minilzo.h"

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

void locate(struct waf_archive *arc, const char *file)
{
	if (waf_locate(arc, file))
	{
		printf("The file '%s' (%d bytes) has been located.\n",
			file, waf_size(arc));
	}
	else
	{
		printf("Can't locate file '%s'\n", file);
	}
}

void read_test(struct waf_archive *arc)
{
	char buf[4096];
	int read;

	read = waf_read(arc, buf, 4000);
	if (read >= 0)
	{
		buf[read] = '\0';
		printf("%d bytes read.\n", read);
		printf("%s\n", buf);
	}
	else
	{
		printf("Error reading file.\n");
	}
}

int main(void)
{
	struct waf_archive *arc;

	arc = waf_open_archive("../wanearc/data.waf", wafext_setup_minilzo());
	if (arc)
	{
		printf("Archive opened!\n");

		locate(arc, "blablabla.xxx");

		locate(arc, "archive.c");
		read_test(arc);

		locate(arc, "archive.h");
		read_test(arc);

		locate(arc, "listfile.txt");
		read_test(arc);

		waf_close_archive(arc);
	}
	else
	{
		printf("Unable to open archive.\n");
	}

	return 0;
}
