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

/* temporary testing code */
/* TODO: to be removed */
extern void waf_enum_files(struct waf_archive *arc, void (*enum_func)(const char *name, int size));

void show_file_detail(const char *name, int size)
{
	printf("--> [%s] %d bytes\n", name, size);
}

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

	arc = waf_open_archive("../wanearc/data.waf", &unit_setup);
	if (arc)
	{
		printf("Archive opened!\n");

		waf_enum_files(arc, show_file_detail);

		locate(arc, "blablabla.xxx");

		locate(arc, "archive.c");
		read_test(arc);

		locate(arc, "archive.h");
		read_test(arc);

		waf_close_archive(arc);
	}
	else
	{
		printf("Unable to open archive.\n");
	}

	return 0;
}
