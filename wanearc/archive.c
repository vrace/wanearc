#include "archive.h"
#include "archive_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct archive
{
	struct archive_setup *setup;
	FILE *fp;
	unsigned char *src;
	unsigned char *transformed;
};

static int write_header(FILE *fp, struct archive_setup *setup)
{
	int err;
	char waf[4] = { 'W', 'A', 'F', '\0' };

	assert(fp != NULL);
	assert(setup != NULL);

	do
	{
		err = 1;
		if (fwrite(waf, 1, 4, fp) != 4)
			break;

		err = 2;
		if (fwrite(setup->tag, 1, 4, fp) != 4)
			break;

		err = 3;
		if (fwrite_int(fp, setup->src_size) != sizeof(int))
			break;

		err = 4;
		if (fwrite_int(fp, setup->transform_size) != sizeof(int))
			break;

		err = 0;
	} while (0);

	return err;
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
		if (write_header(fp, setup) == 0)
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
