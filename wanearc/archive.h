#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

#include "archive_setup.h"

#define WANEARC_OK	0

#define WANEARC_ERR_READ		100
#define WANEARC_ERR_WRITE		200
#define WANEARC_ERR_TRANSFORM	300
#define WANEARC_ERR_FILELIST	400

struct archive* archive_create(const char *name, struct archive_setup *setup);
void archive_close(struct archive *archive);
int archive_append(struct archive *archive, const char *file);

#endif
