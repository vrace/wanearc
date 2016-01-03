#ifndef __ARCHIVE_IO_H__
#define __ARCHIVE_IO_H__

#include <stdio.h>

int fwrite_int(FILE *fp, int value);
int fwrite_buf(FILE *fp, const unsigned char *buf, int size);
int fwrite_str(FILE *fp, const char *str);

#endif
