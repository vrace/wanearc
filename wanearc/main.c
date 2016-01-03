#include <stdio.h>
#include "archive.h"

void show_title(void)
{
	printf("WANE Archive Builder\n");
	printf("Copyright (c) 2010-2016 WANE. All rights reserved.\n");
	printf("\n");
}

int main(void)
{
	struct archive *arc;

	show_title();

	/* test code */
	arc = archive_create("test.dat", &default_setup);
	if (arc)
	{
		if (archive_append(arc, "archive.c") == 0)
		{
			printf("add success\n");
		}
		else
		{
			printf("add failed\n");
		}

		archive_close(arc);
	}
	else
	{
		printf("Unable to create archive.\n");
	}

	return 0;
}
