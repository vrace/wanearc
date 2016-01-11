#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "archive.h"
#include "../wanearc_module_minilzo/archive_setup_minilzo.h"

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
}

int build_archive(const char *archive_name, const char *listfile_name, struct archive_setup *setup)
{
	int err = 0;
	FILE *listfile = NULL;
	struct archive *arc = NULL;
	char source[_MAX_PATH];

	assert(archive_name != NULL);
	assert(listfile_name != NULL);

	do
	{
		err = 1;
		listfile = fopen(listfile_name, "r");
		if (!listfile)
			break;

		err = 2;
		arc = archive_create(archive_name, setup);
		if (!arc)
			break;

		printf("Creating archive '%s'...\n\n", archive_name);

		err = 0;
		while (err == 0 && fgets(source, _MAX_PATH, listfile))
		{
			if (strlen(source) > 0)
				source[strlen(source) - 1] = '\0';

			if (strlen(source) > 0)
			{
				printf("\t%s\n", source);
				err = archive_append(arc, source);
			}
		}

	} while(0);

	if (listfile)
		fclose(listfile);

	if (arc)
		archive_close(arc);

	switch (err)
	{
	case 1:
		printf("Can't read list file: %s\n", listfile_name);
		break;

	case 2:
		printf("Can't create archive: %s\n", archive_name);
		break;

	case 0:
		printf("\nArchive '%s' created.\n", archive_name);
		break;
	}

	return err;
}

struct archive_setup* init_minilzo_setup(void)
{
	init_archive_setup_minilzo();
	return (struct archive_setup*)&archive_setup_minilzo;
}

struct archive_setup* archive_setup_from_arg(const char *transform)
{
	struct archive_setup *setup = NULL;

	if (!transform)
		transform = "LZO";

	if (strcmp(transform, "RAW") == 0)
	{
		setup = &default_setup;
	}
	else if (strcmp(transform, "LZO") == 0)
	{
		setup = init_minilzo_setup();
	}

	if (!setup)
	{
		printf("Unrecognized transform '%s'.\n", transform);
		setup = init_minilzo_setup();
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

	return 0;
}
