#ifndef __ARCHIVE_SETUP_H__
#define __ARCHIVE_SETUP_H__

typedef int (*transform_func)(struct archive_setup *setup, unsigned char *dest, const unsigned char *src, int size);

struct archive_setup
{
	int src_size;
	int transform_size;
	transform_func transform;
	char tag[4];
};

extern struct archive_setup default_setup;

#endif
