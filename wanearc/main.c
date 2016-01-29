#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "archive.h"
#include "../wanearc_module_minilzo/archive_setup_minilzo.h"
#include "../wanearc_module_zlib/archive_setup_zlib.h"

void show_title(void)
{
	printf("WANE Archive Builder\n");
	printf("Copyright (c) 2010-2016 WANE. All rights reserved.\n");
	printf("\n");
}

void show_usage(void)
{
	printf("Usage: wanearc <archive> <listfile> [transform]\n");
	printf("\n");
	printf("Available transforms:\n");
	printf("    RAW     Keep the data unchanged.\n");
	printf("    LZO     Use minilzo to compress the data.\n");
	printf("    ZIP     Use zlib to compress the data.\n");
}

FILE* open_listfile(const char *filename)
{
	FILE *fp;

	assert(filename != NULL);

	fp = fopen(filename, "r");
	if (!fp)
		printf("Can't open list file '%s'.\n", filename);

	return fp;
}

struct archive* begin_create_archive(const char *filename, struct archive_setup *setup)
{
	struct archive *arc;

	assert(filename != NULL);

	arc = archive_create(filename, setup);
	if (!arc)
		printf("Can't create archive '%s'.\n", filename);

	return arc;
}

void trim_newline(char *str)
{
	int len = (int)strlen(str);

	assert(str != NULL);

	if (len > 0 && str[len - 1] == '\n')
		str[len - 1] = '\0';
}

int build_from_filelist(struct archive *arc, FILE *filelist)
{
	int err = WANEARC_OK;
	char source[_MAX_PATH];

	while (err == WANEARC_OK && fgets(source, _MAX_PATH, filelist))
	{
		trim_newline(source);

		if (strlen(source) > 0)
		{
			printf("\t%s\n", source);
			err = archive_append(arc, source);
			if (err != WANEARC_OK)
				printf("\nFailed to add '%s'.\n", source);
		}
	}

	if (err == WANEARC_OK)
		printf("\nDone.\n");

	return err;
}

int build_archive(const char *archive_name, const char *listfile_name, struct archive_setup *setup)
{
	int err = WANEARC_OK;
	FILE *listfile = NULL;
	struct archive *arc = NULL;

	assert(archive_name != NULL);
	assert(listfile_name != NULL);

	do
	{
		err = WANEARC_ERR_FILELIST;
		listfile = open_listfile(listfile_name);
		if (!listfile)
			break;

		err = WANEARC_ERR_WRITE;
		arc = begin_create_archive(archive_name, setup);
		if (!arc)
			break;

		printf("Creating archive '%s'...\n\n", archive_name);
		err = build_from_filelist(arc, listfile);
	} while(0);

	if (listfile)
		fclose(listfile);

	if (arc)
		archive_close(arc);

	return err;
}

struct archive_setup* archive_setup_from_arg(const char *transform)
{
	struct archive_setup *setup = NULL;

	if (!transform)
		transform = "LZO";

	if (strcmp(transform, "RAW") == 0)
	{
		setup = archive_setup_default();
	}
	else if (strcmp(transform, "LZO") == 0)
	{
		setup = archive_setup_minilzo();
	}
	else if (strcmp(transform, "ZIP") == 0)
	{
		setup = archive_setup_zlib();
	}

	if (!setup)
	{
		printf("Unrecognized transform '%s'.\n", transform);
		setup = archive_setup_minilzo();
		transform = "LZO";
	}

	printf("Setting transform to '%s'...\n", transform);
	return setup;
}

int main(int argc, char *argv[])
{
	int err = 0;
	show_title();

	if (argc != 3 && argc != 4)
	{
		show_usage();
		err = -1;
	}
	else
	{
		struct archive_setup *setup;
		const char *transform;

		transform = argc == 4 ? argv[3] : NULL;
		setup = archive_setup_from_arg(transform);

		err = build_archive(argv[1], argv[2], setup);
	}

	return err;
}
