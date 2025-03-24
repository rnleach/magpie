/* This program combines all the header files together into the final product, a single header library. */
#include <stdio.h>

#include "coyote.h"

void *memcpy(void *dest, const void *src, size_t n);
int memcmp (const void * ptr1, const void * ptr2, size_t num);
void *memchr(const void *, int, size_t);

static inline b32
str_eq(char const *left, char const *right, size len)
{
    return 0 == memcmp(left, right, len);
}

static b32
load_file(char const *fname, size buf_size, byte *buffer, size *read)
{
    *read = coy_file_slurp(fname, buf_size, buffer);
    if(*read < 0) { return false; }
    return true;
}

static void
insert_buffer(size out_size, size *out_idx, char *out, size in_size, char *in)
{
    Assert(*out_idx + in_size < out_size);

    char *start = &out[*out_idx];
    memcpy(start, in, in_size);
    *out_idx += in_size;
}

int
main(int argc, char *argv[])
{
    char main_buffer[COY_KiB(100)] = {0};
    size mb_size = 0;
    char win32_buffer[COY_KiB(100)] = {0};
    size w32_size = 0;
    char apple_buffer[COY_KiB(20)] = {0};
    size ap_size = 0;
    char linux_buffer[COY_KiB(20)] = {0};
    size li_size = 0;

    char finished_lib[COY_KiB(350)] = {0};

    // Load all the files
    char const *fname = "../src/magpie.h";
    b32 success = load_file(fname, sizeof(main_buffer), main_buffer, &mb_size);
    StopIf(!success, return 1);

    char const *fname_win32 = "../src/magpie_win32.h";
    success = load_file(fname_win32, sizeof(win32_buffer), win32_buffer, &w32_size);
    StopIf(!success, return 1);

    char const *fname_apple = "../src/magpie_apple_osx.h";
    success = load_file(fname_apple, sizeof(apple_buffer), apple_buffer, &ap_size);
    StopIf(!success, return 1);

    char const *fname_linux = "../src/magpie_linux.h";
    success = load_file(fname_linux, sizeof(linux_buffer), linux_buffer, &li_size);
    StopIf(!success, return 1);

    // Merge them in a buffer
    char *c = main_buffer;
    char *end = memchr(c, '\0', sizeof(main_buffer));

    char *insert_marker = memchr(c, '#', end - c);
    Assert(insert_marker);
    size oi = 0; //output index
    while(insert_marker - c < end - c)
    {
	  if(str_eq("#include \"magpie_win32.h\"", insert_marker, 25))
	  {
		insert_buffer(sizeof(finished_lib), &oi, finished_lib, insert_marker - c, c);
		insert_buffer(sizeof(finished_lib), &oi, finished_lib, w32_size, win32_buffer);
		insert_marker += 25;
		c = insert_marker;
	  }
	  else if(str_eq("#include \"magpie_linux.h\"", insert_marker, 25))
	  {
		insert_buffer(sizeof(finished_lib), &oi, finished_lib, insert_marker - c, c);
		insert_buffer(sizeof(finished_lib), &oi, finished_lib, li_size, linux_buffer);
		insert_marker += 25;
		c = insert_marker;
	  }
	  else if(str_eq("#include \"magpie_apple_osx.h\"", insert_marker, 29))
	  {
		insert_buffer(sizeof(finished_lib), &oi, finished_lib, insert_marker - c, c);
		insert_buffer(sizeof(finished_lib), &oi, finished_lib, ap_size, apple_buffer);
		insert_marker += 29;
		c = insert_marker;
	  }
	  else
	  {
		insert_marker = memchr(insert_marker + 1, '#', end - insert_marker);
		if(!insert_marker)
		{
		    insert_buffer(sizeof(finished_lib), &oi, finished_lib, end - c, c);
		    break;
		}
	  }
    }

    // Write out the buffer
    CoyFileWriter magpie_ = coy_file_create("magpie.h");
    CoyFileWriter *magpie = &magpie_;
    StopIf(!magpie->valid, fprintf(stderr, "Error opening magpie.h for output\n"); return 1);

    size magpie_wrote = coy_file_write(magpie, oi, finished_lib);
    Assert(magpie_wrote == oi);

    coy_file_writer_close(magpie);
}
