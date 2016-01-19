#include "archive.h"
#include "archive_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static struct archive
{
	struct archive_setup *setup;
	FILE *fp;
	unsigned char *src;
	unsigned char *transformed;
};

static int write_header(FILE *fp, struct archive_setup *setup)
{
	int ret;
	char waf[4] = { 'W', 'A', 'F', '\0' };

	assert(fp != NULL);
	assert(setup != NULL);

	do
	{
		ret = WANEARC_ERR_WRITE;

		if (fwrite(waf, 1, 4, fp) != 4)
			break;

		if (fwrite(setup->tag, 1, sizeof(setup->tag), fp) != sizeof(setup->tag))
			break;

		if (fwrite_int(fp, setup->src_size) != sizeof(int))
			break;

		if (fwrite_int(fp, setup->transform_size) != sizeof(int))
			break;

		ret = WANEARC_OK;
	} while (0);

	return ret;
}

struct archive* archive_create(const char *name, struct archive_setup *setup)
{
	FILE *fp;
	struct archive *arc = NULL;

	assert(name != NULL);
	assert(setup != NULL);
	assert(setup->src_size > 0);
	assert(setup->transform_size > 0);
	assert(setup->transform != NULL);

	fp = fopen(name, "wb");
	if (fp)
	{
		if (write_header(fp, setup) == WANEARC_OK)
		{
			arc = malloc(sizeof(struct archive));
			assert(arc != NULL);

			arc->setup = setup;
			arc->fp = fp;

			arc->src = malloc(setup->src_size);
			assert(arc->src != NULL);

			arc->transformed = malloc(setup->transform_size);
			assert(arc->transformed != NULL);
		}
	}

	return arc;
}

void archive_close(struct archive *archive)
{
	if (archive)
	{
		if (archive->fp)
			fclose(archive->fp);

		free(archive->src);
		free(archive->transformed);

		free(archive);
	}
}

static int write_content(struct archive *archive, FILE *src)
{
	int err = WANEARC_OK;
	int size;
	int transformed_size;

	assert(archive != NULL);
	assert(src != NULL);

	while ((size = (int)fread(archive->src, 1, archive->setup->src_size, src)) > 0)
	{
		err = WANEARC_ERR_WRITE;
		if (fwrite_int(archive->fp, size) != sizeof(int))
			break;

		err = WANEARC_ERR_TRANSFORM;
		transformed_size = archive->setup->transform(archive->setup, archive->transformed, archive->src, size);
		if (transformed_size < 0)
			break;

		err = WANEARC_ERR_WRITE;
		if (fwrite_buf(archive->fp, archive->transformed, transformed_size) != transformed_size)
			break;

		err = WANEARC_OK;
	}

	if (err == WANEARC_OK)
	{
		if (fwrite_int(archive->fp, 0) != sizeof(int))
			err = WANEARC_ERR_WRITE;
	}

	return err;
}

int archive_append(struct archive *archive, const char *file)
{
	int err;
	FILE *src;

	assert(archive != NULL);
	assert(file != NULL);

	do
	{
		err = WANEARC_ERR_READ;
		src = fopen(file, "rb");
		if (!src)
			break;

		err = WANEARC_ERR_WRITE;
		if(fwrite_str(archive->fp, file) != (int)strlen(file))
			break;

		err = write_content(archive, src);
	} while(0);

	if (src)
		fclose(src);

	return err;
}
