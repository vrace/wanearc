#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

struct crazy_chunk
{
	int offset;
	int size;
};

struct crazy_chunk_array
{
	struct crazy_chunk *p;
	int cap;
	int size;
};

void randomize_array(struct crazy_chunk_array *arr)
{
	int i;

	for (i = 0; i < arr->size; i++)
	{
		struct crazy_chunk t = arr->p[i];
		int r = rand() % arr->size;

		arr->p[i] = arr->p[r];
		arr->p[r] = t;
	}
}

struct crazy_chunk_array build_crazy_chunks(int total)
{
	int offset = 0;
	struct crazy_chunk_array arr;

	arr.p = NULL;
	arr.cap = 0;
	arr.size = 0;

	while (offset < total)
	{
		struct crazy_chunk c;

		c.offset = offset;
		c.size = rand() % 1048576;

		if (offset + c.size > total)
			c.size = total - offset;

		if (arr.size >= arr.cap)
		{
			int newcap = arr.cap + (arr.cap > 4 ? arr.cap / 2 : 4);
			arr.p = realloc(arr.p, sizeof(struct crazy_chunk) * newcap);
			arr.cap = newcap;
		}

		arr.p[arr.size++] = c;
		offset += c.size;
	}

	randomize_array(&arr);

	return arr;
}

/* this extracts a big file in random offset and size */
/* the output file should be identical to packed source file */
void crazy_extract(struct waf_archive *arc, const char *name)
{
	if (waf_locate(arc, name))
	{
		FILE *fp;
		struct crazy_chunk_array arr = build_crazy_chunks(waf_size(arc));

		fp = fopen(name, "wb");
		if (fp)
		{
			int i;
			char *buf = malloc(1048576);

			for (i = 0; i < waf_size(arc); i++)
				fputc(0, fp);

			for (i = 0; i < arr.size; i++)
			{
				waf_seek(arc, arr.p[i].offset, SEEK_SET);
				waf_read(arc, buf, arr.p[i].size);

				fseek(fp, arr.p[i].offset, SEEK_SET);
				fwrite(buf, 1, arr.p[i].size, fp);
			}

			free(buf);
			fclose(fp);

			printf("New file has been created.\n");
			printf("%d blocks have been written.\n", arr.size);
		}
		else
		{
			printf("Could not create '%s'.\n", name);
		}

		free(arr.p);
	}
	else
	{
		printf("Could not locate '%s'.\n", name);
	}
}

int main(void)
{
	struct waf_archive *arc = NULL;
	const char *file = "../wanearc/data.waf";

	srand((unsigned int)time(NULL));

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

		crazy_extract(arc, "VS80sp1-KB926748-X86-INTL.exe");

		waf_close_archive(arc);
	}
	else
	{
		printf("Unable to open archive.\n");
	}

	return 0;
}
