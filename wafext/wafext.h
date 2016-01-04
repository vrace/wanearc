#ifndef __WAF_EXT_H__
#define __WAF_EXT_H__

typedef int (*restore_func)(struct waf_archive_setup *setup, unsigned char *restored, const unsigned char *transformed, int size);

struct waf_archive_setup
{
	int restored_size;
	int transformed_size;
	restore_func restore;
	char tag[4];
};

struct waf_archive* waf_open_archive(const char *filename, struct waf_archive_setup *setup);

#endif