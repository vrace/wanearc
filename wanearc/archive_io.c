#include "archive_io.h"
#include <string.h>
#include <assert.h>

static int align_size(int size)
{
	return ((size - 1 + sizeof(int)) / sizeof(int)) * sizeof(int);
}

int fwrite_int(FILE *fp, int value)
{
	unsigned char buf[sizeof(int)];
	unsigned int raw;
	int i;

	assert(fp != NULL);

	raw = *(unsigned int*)&value;
	for (i = 0; i < sizeof(int); i++)
		buf[i] = (unsigned char)((raw >> (i << 3)) & 0xff);

	return (int)fwrite(buf, 1, sizeof(int), fp);
}

int fwrite_buf(FILE *fp, const unsigned char *buf, int size)
{
	unsigned int pad = 0;
	int written;
	int align_pad = align_size(size) - size;

	assert(fp != NULL);
	assert(size == 0 || (size > 0 && buf != NULL));

	do
	{
		written = -1;
		if (fwrite_int(fp, size) != sizeof(int))
			break;

		written = (int)fwrite(buf, 1, size, fp);
		if (written != size)
			break;

		if (align_pad > 0)
		{
			if (fwrite(&pad, 1, align_pad, fp) != align_pad)
				written = -1;
		}
	} while(0);

	return written;
}

int fwrite_str(FILE *fp, const char *str)
{
	return fwrite_buf(fp, str, (int)strlen(str));
}
