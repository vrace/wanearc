#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../wafext/wafext.h"
#include "../wafext_minilzo/wafext_minilzo.h"
#include "../wafext_zlib/wafext_zlib.h"

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
	char buf[100];
	int read;
	int total = 0;

	while (!waf_eof(arc) && (read = waf_read(arc, buf, 99)) > 0)
	{
		total += read;
		buf[read] = '\0';
		printf("%s", buf);
	}

	printf("\n%d bytes read.\n", total);
}

void read_offset_test(struct waf_archive *arc)
{
	char buf[100];
	int read;

	printf("Offset = %d\n", waf_tell(arc));
	read = waf_read(arc, buf, 99);
	if (read >= 0)
		buf[read] = '\0';
	printf("Read (%d): %s\n", read, buf);
}

void offset_test(struct waf_archive *arc)
{
	printf("Before offset: %d\n", waf_tell(arc));
	printf("File size: %d\n", waf_size(arc));

	printf("\nOffset to begin.\n");
	if (waf_seek(arc, 0, SEEK_SET) != 0)
		printf("Seek failed.\n");
	read_offset_test(arc);

	printf("\nOffset to 7\n");
	if (waf_seek(arc, 0, SEEK_SET) != 0)
		printf("Seek failed.\n");
	if (waf_seek(arc, 7, SEEK_CUR) != 0)
		printf("Seek failed.\n");
	read_offset_test(arc);

	printf("\nOffset back 7\n");
	if (waf_seek(arc, -7, SEEK_CUR) != 0)
		printf("Seek failed.\n");
	read_offset_test(arc);

	printf("\nOffset to 9 backward\n");
	if (waf_seek(arc, -9, SEEK_END) != 0)
		printf("Seek failed.\n");
	read_offset_test(arc);

	printf("\nOffset to after end\n");
	if (waf_seek(arc, 100, SEEK_END) != 0)
		printf("Seek failed.\n");
	read_offset_test(arc);

	printf("\nOffset to before begin\n");
	if (waf_seek(arc, -100, SEEK_SET) != 0)
		printf("Seek failed.\n");
	read_offset_test(arc);
}

int main(void)
{
	struct waf_archive *arc = NULL;
	const char *file = "../wanearc/data.waf";

	/* demo only! */
	/* should explicitly use one setup in real world! */
	arc = waf_open_archive(file, &unit_setup);
	arc = arc ? arc : waf_open_archive(file, wafext_setup_minilzo());
	arc = arc ? arc : waf_open_archive(file, wafext_setup_zlib());

	if (arc)
	{
		printf("Archive opened!\n");

		locate(arc, "blablabla.xxx");
		waf_locate(arc, NULL);

		locate(arc, "archive.c");
		read_test(arc);
		waf_locate(arc, NULL);

		locate(arc, "archive.h");
		read_test(arc);
		waf_locate(arc, NULL);

		locate(arc, "listfile.txt");
		read_test(arc);
		waf_locate(arc, NULL);

		locate(arc, "text.txt");
		read_test(arc);
		offset_test(arc);

		waf_locate(arc, NULL);
		waf_close_archive(arc);
	}
	else
	{
		printf("Unable to open archive.\n");
	}

	return 0;
}
