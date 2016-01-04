#include "wafext.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static struct waf_archive
{
	struct waf_archive_setup *setup;
	FILE *fp;
	unsigned char *restore_buf;
	unsigned char *transformed_buf;
	/* TODO: data here to represent the items */
};

static struct waf_archive_header
{
	char arc[4];
	char tag[4];
	unsigned char restored_size[sizeof(int)];
	unsigned char transformed_size[sizeof(int)];
};

static int int_from_buf(const unsigned char *buf)
{
	int i;
	unsigned int raw = 0;

	for (i = 0; i < sizeof(int); i++)
		raw |= (unsigned int)buf[i] << (i << 3);

	return *(int*)&raw;
}

static int fread_int(FILE *fp, int *value)
{
	unsigned char buf[sizeof(int)];

	assert(fp != NULL);

	if (fread(buf, 1, sizeof(int), fp) == sizeof(int))
	{
		if (value)
			*value = int_from_buf(buf);

		return sizeof(int);
	}

	return 0;
}

static int fread_block(struct waf_archive *arc)
{
	int block_size;
	int ret = -1;

	assert(arc != NULL);
	assert(arc->fp != NULL);
	assert(arc->setup != NULL);
	assert(arc->setup->transformed_size > 0);
	assert(arc->transformed_buf != NULL);

	if (fread_int(arc->fp, &block_size) == sizeof(int))
	{
		if (block_size <= arc->setup->transformed_size)
		{
			if (block_size == 0)
			{
				ret = 0;
			}
			else if (block_size > 0)
			{
				if ((int)fread(arc->transformed_buf, 1, block_size, arc->fp) == block_size)
					ret = block_size;
			}
		}
	}

	return ret;
}

static int match_archive(FILE *fp, struct waf_archive_setup *setup)
{
	struct waf_archive_header header;

	assert(fp != NULL);
	assert(setup != NULL);

	if (fread(&header, 1, sizeof(header), fp) == sizeof(header))
	{
		if (header.arc[0] == 'W' && header.arc[1] == 'A' && header.arc[3] == 'F' && header.arc[4] == 0 &&
			memcmp(header.tag, setup->tag, 4) == 0 &&
			int_from_buf(header.restored_size) == setup->restored_size &&
			int_from_buf(header.transformed_size) == setup->transformed_size)
		{
			return 1;
		}
	}

	return 0;
}

struct waf_archive* waf_open_archive(const char *filename, struct waf_archive_setup *setup)
{
	FILE *fp;

	assert(filename != NULL);
	assert(setup != NULL);
	assert(setup->restored_size > 0);
	assert(setup->transformed_size > 0);
	assert(setup->restore != NULL);

	fp = fopen(filename, "rb");
	if (fp)
	{
		if (match_archive(fp, setup))
		{
			struct waf_archive *arc;

			arc = malloc(sizeof(struct waf_archive));
			assert(arc != NULL);

			arc->setup = setup;
			arc->fp = fp;

			arc->restore_buf = malloc(setup->restored_size);
			arc->transformed_buf = malloc(setup->transformed_size);

			return arc;
		}

		fclose(fp);
	}

	return NULL;
}

void waf_close_archive(struct waf_archive *arc)
{
	if (arc)
	{
		if (arc->fp)
			fclose(fp);

		free(arc->restore_buf);
		free(arc->transformed_buf);

		free(arc);
	}
}
