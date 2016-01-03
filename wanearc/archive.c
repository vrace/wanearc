#include "archive.h"
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
		arc = malloc(sizeof(struct archive));
		assert(arc != NULL);

		arc->setup = setup;
		arc->fp = fp;

		arc->src = malloc(setup->src_size);
		assert(arc->src != NULL);

		arc->transformed = malloc(setup->transform_size);
		assert(arc->transformed != NULL);
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
