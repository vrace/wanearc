#ifndef __ARCHIVE_SETUP_MINILZO_H__
#define __ARCHIVE_SETUP_MINILZO_H__

#include "../wanearc/archive_setup.h"
#include "../minilzo/minilzo.h"

#define LZO_ALLOC(var,size)\
	lzo_align_t __LZO_MMODEL var [((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t)]

struct archive_setup_minilzo
{
	struct archive_setup base;
	LZO_ALLOC(workmem, LZO1X_1_MEM_COMPRESS);
};

void init_archive_setup_minilzo(void);
extern struct archive_setup_minilzo archive_setup_minilzo;

#endif
