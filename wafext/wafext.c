#include "wafext.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static struct waf_file
{
	char name[_MAX_PATH];
	int size;
	int *blocklist;
	int blocklist_size;
	int blocklist_cap;
};

static struct waf_working_file
{
	struct waf_file *file;
	int block;
	int offset;
};

static struct waf_archive
{
	struct waf_archive_setup *setup;
	FILE *fp;
	unsigned char *restore_buf;
	unsigned char *transformed_buf;

	struct waf_file *filelist;
	int filelist_cap;
	int filelist_size;
	
	struct waf_working_file working;
};

static struct waf_archive_header
{
	char arc[4];
	char tag[4];
	unsigned char restored_size[sizeof(int)];
	unsigned char transformed_size[sizeof(int)];
};

static int int_from_buf(const unsigned char *buf)
{
	int i;
	unsigned int raw = 0;

	for (i = 0; i < sizeof(int); i++)
		raw |= (unsigned int)buf[i] << (i << 3);

	return *(int*)&raw;
}

static int align_size(int size)
{
	return ((size - 1 + sizeof(int)) / sizeof(int)) * sizeof(int);
}

static int fread_int(FILE *fp, int *value)
{
	unsigned char buf[sizeof(int)];

	assert(fp != NULL);

	if (fread(buf, 1, sizeof(int), fp) == sizeof(int))
	{
		if (value)
			*value = int_from_buf(buf);

		return sizeof(int);
	}

	return 0;
}

static int fread_block(struct waf_archive *arc)
{
	int block_size;
	int ret = -1;

	assert(arc != NULL);
	assert(arc->fp != NULL);
	assert(arc->setup != NULL);
	assert(arc->setup->transformed_size > 0);
	assert(arc->transformed_buf != NULL);

	if (fread_int(arc->fp, &block_size) == sizeof(int))
	{
		if (block_size <= arc->setup->transformed_size)
		{
			if (block_size == 0)
			{
				ret = 0;
			}
			else if (block_size > 0)
			{
				if ((int)fread(arc->transformed_buf, 1, block_size, arc->fp) == block_size)
					ret = block_size;

				fseek(arc->fp, align_size(block_size) - block_size, SEEK_CUR);
			}
		}
	}

	return ret;
}

static int fread_str(struct waf_archive *arc, char *str, int size)
{
	int readsize;

	assert(arc != NULL);
	assert(str != NULL);
	assert(size > 0);

	readsize = fread_block(arc);
	if (readsize >= 0)
	{
		if (readsize > size - 1)
			readsize = size - 1;

		memcpy(str, arc->transformed_buf, readsize);
		str[readsize] = '\0';
	}

	return readsize;
}

static void* expand_array(void *current, int unit_size, int *cap)
{
	assert(unit_size >= 0);
	assert(cap != NULL);
	assert(*cap >= 0);

	*cap += ((*cap == 0) ? 8 : (*cap / 2));
	return realloc(current, *cap * unit_size);
}

static int match_archive(FILE *fp, struct waf_archive_setup *setup)
{
	struct waf_archive_header header;

	assert(fp != NULL);
	assert(setup != NULL);

	if (fread(&header, 1, sizeof(header), fp) == sizeof(header))
	{
		if (header.arc[0] == 'W' && header.arc[1] == 'A' && header.arc[2] == 'F' && header.arc[3] == 0 &&
			memcmp(header.tag, setup->tag, 4) == 0 &&
			int_from_buf(header.restored_size) == setup->restored_size &&
			int_from_buf(header.transformed_size) == setup->transformed_size)
		{
			return 1;
		}
	}

	return 0;
}

static void record_block_offset(struct waf_file *file, int offset)
{
	assert(file != NULL);
	assert(offset >= 0);

	if (file->blocklist_size >= file->blocklist_cap)
		file->blocklist = expand_array(file->blocklist, sizeof(int), &file->blocklist_cap);

	file->blocklist[file->blocklist_size++] = offset;
}

static void build_file_detail(struct waf_archive *arc, struct waf_file *file)
{
	FILE *fp;

	assert(arc != NULL);
	assert(arc->fp != NULL);
	assert(file != NULL);

	fp = arc->fp;
	file->size = 0;
	file->blocklist = NULL;
	file->blocklist_cap = 0;
	file->blocklist_size = 0;

	while (1)
	{
		int size;

		if (fread_int(fp, &size) != sizeof(int))
			break;
		if (size == 0)
			break;

		file->size += size;
		record_block_offset(file, (int)ftell(fp));
		
		if (fread_int(fp, &size) != sizeof(int))
			break;
		fseek(fp, align_size(size), SEEK_CUR);
	}
}

static void build_filelist(struct waf_archive *arc)
{
	assert(arc != NULL);
	assert(arc->filelist == NULL);
	assert(arc->filelist_cap == 0);
	assert(arc->filelist_size == 0);

	fseek(arc->fp, sizeof(struct waf_archive_header), SEEK_SET);

	while (1)
	{
		if (arc->filelist_size >= arc->filelist_cap)
			arc->filelist = expand_array(arc->filelist, sizeof(struct waf_file), &arc->filelist_cap);

		if (fread_str(arc, arc->filelist[arc->filelist_size].name, _MAX_PATH) <= 0)
			break;

		build_file_detail(arc, &arc->filelist[arc->filelist_size]);
		arc->filelist_size++;
	}
}

struct waf_archive* waf_open_archive(const char *filename, struct waf_archive_setup *setup)
{
	FILE *fp;

	assert(filename != NULL);
	assert(setup != NULL);
	assert(setup->restored_size > 0);
	assert(setup->transformed_size > 0);
	assert(setup->restore != NULL);

	fp = fopen(filename, "rb");
	if (fp)
	{
		if (match_archive(fp, setup))
		{
			struct waf_archive *arc;

			arc = malloc(sizeof(struct waf_archive));
			assert(arc != NULL);

			arc->setup = setup;
			arc->fp = fp;

			arc->restore_buf = malloc(setup->restored_size);
			arc->transformed_buf = malloc(setup->transformed_size);

			arc->filelist = NULL;
			arc->filelist_cap = 0;
			arc->filelist_size = 0;

			build_filelist(arc);

			arc->working.file = NULL;
			arc->working.block = -1;
			arc->working.offset = 0;

			return arc;
		}

		fclose(fp);
	}

	return NULL;
}

void waf_close_archive(struct waf_archive *arc)
{
	int i;

	if (arc)
	{
		if (arc->fp)
			fclose(arc->fp);

		free(arc->restore_buf);
		free(arc->transformed_buf);
	
		for (i = 0; i < arc->filelist_size; i++)
			free(arc->filelist[i].blocklist);

		free(arc->filelist);

		free(arc);
	}
}

int waf_locate(struct waf_archive *arc, const char *file)
{
	struct waf_file *cur = NULL;
	int i;

	assert(arc != NULL);

	for (i = 0; i < arc->filelist_size; i++)
	{
		if (strcmp(arc->filelist[i].name, file) == 0)
		{
			cur = &arc->filelist[i];
			break;
		}
	}

	if (cur)
	{
		arc->working.file = cur;
		arc->working.block = -1;
		arc->working.offset = 0;
	}

	return cur != NULL;
}

int waf_size(struct waf_archive *arc)
{
	assert(arc != NULL);

	if (arc->working.file)
		return arc->working.file->size;

	return -1;
}

int waf_tell(struct waf_archive *arc)
{
	int pos = -1;
	assert(arc != NULL);

	if (arc->working.file)
		pos = arc->working.offset;

	return pos;
}

int waf_eof(struct waf_archive *arc)
{
	assert(arc != NULL);

	if (arc->working.file)
		return arc->working.offset >= arc->working.file->size;

	return 1;
}

static int readable_size(struct waf_archive *arc)
{
	struct waf_working_file *working;
	int begin;
	int end;

	assert(arc != NULL);
	assert(arc->working.file != NULL);

	working = &arc->working;

	begin = working->block * arc->setup->restored_size;
	end = (working->block + 1) * arc->setup->restored_size;

	if (end > working->file->size)
		end = working->file->size;

	if (working->offset >= begin && working->offset < end)
		return end - working->offset;

	return 0;
}

static int copy_block_buf(struct waf_archive *arc, unsigned char *buf, int size)
{
	int size_to_copy;

	assert(arc != NULL);
	assert(buf != NULL);
	assert(size >= 0);

	size_to_copy = readable_size(arc);
	if (size_to_copy > size)
		size_to_copy = size;

	memcpy(buf, arc->restore_buf, size_to_copy);
	arc->working.offset += size_to_copy;

	return size_to_copy;
}

static int restore_block(struct waf_archive *arc)
{
	struct waf_working_file *working;
	int block_to_restore;

	assert(arc != NULL);
	assert(arc->working.file != NULL);

	working = &arc->working;
	block_to_restore = working->offset / arc->setup->restored_size;

	if (block_to_restore >= 0 && block_to_restore < working->file->blocklist_size)
	{
		int blocksize;
		fseek(arc->fp, working->file->blocklist[block_to_restore], SEEK_SET);
		if ((blocksize = fread_block(arc)) > 0)
		{
			int restored_size = arc->setup->restore(arc->setup, arc->restore_buf, arc->transformed_buf, blocksize);
			if (restored_size >= 0)
			{
				working->block = block_to_restore;
				return restored_size;
			}
		}
	}

	return 0;
}

int waf_read(struct waf_archive *arc, unsigned char *buf, int size)
{
	int sizeread = -1;

	assert(arc != NULL);
	assert(buf != NULL);

	if (arc->working.file)
	{
		int read = 0;

		sizeread = 0;
		while (size > 0 && read >= 0 && !waf_eof(arc))
		{
			int read = copy_block_buf(arc, &buf[sizeread], size);
			
			if (read == 0)
			{
				if (!restore_block(arc))
					read = -1;
			}
			else if (read > 0)
			{
				sizeread += read;
				size -= read;
			}
		}

		if (read < 0)
			sizeread = -1;
	}

	return sizeread;
}
