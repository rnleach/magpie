#ifndef _COYOTE_H_
#define _COYOTE_H_

#include <stdint.h>
#include <stddef.h>

#pragma warning(push)

/*---------------------------------------------------------------------------------------------------------------------------
 * TODO: Things I'd like to add.
 *-------------------------------------------------------------------------------------------------------------------------*/
// TODO: Formatted output.

/*---------------------------------------------------------------------------------------------------------------------------
 * Define simpler types.
 *-------------------------------------------------------------------------------------------------------------------------*/

/* Other libraries I may have already included may use these exact definitions too. */
#ifndef _TYPE_ALIASES_
#define _TYPE_ALIASES_

typedef int32_t     b32;
#ifndef false
   #define false 0
   #define true  1
#endif

#if !defined(_WINDOWS_) && !defined(_INC_WINDOWS)
typedef char       byte;
#endif

typedef ptrdiff_t  size;
typedef size_t    usize;

typedef uintptr_t  uptr;
typedef intptr_t   iptr;

typedef float       f32;
typedef double      f64;

typedef uint8_t      u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef int8_t       i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;
#endif

#define COY_ARRAY_SIZE(A) (sizeof(A) / sizeof(A[0]))

/*---------------------------------------------------------------------------------------------------------------------------
 * Declare parts of the standard C library I use. These should almost always be implemented as compiler intrinsics anyway.
 *-------------------------------------------------------------------------------------------------------------------------*/

void *memset(void *buffer, int val, size_t num_bytes);
void *memcpy(void *dest, void const *src, size_t num_bytes);
void *memmove(void *dest, void const *src, size_t num_bytes);

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                       Error Handling
 *-------------------------------------------------------------------------------------------------------------------------*/

// Crash immediately, useful with a debugger!
#ifndef HARD_EXIT
  #define HARD_EXIT (*(int volatile*)0) 
#endif

#ifndef PanicIf
  #define PanicIf(assertion) StopIf((assertion), HARD_EXIT)
#endif

#ifndef Panic
  #define Panic() HARD_EXIT
#endif

#ifndef StopIf
  #define StopIf(assertion, error_action) if (assertion) { error_action; }
#endif

#ifndef Assert
  #ifndef NDEBUG
    #define Assert(assertion) if(!(assertion)) { HARD_EXIT; }
  #else
    #define Assert(assertion) (void)(assertion)
  #endif
#endif

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                      Date and Time
 *-------------------------------------------------------------------------------------------------------------------------*/
static inline u64 coy_time_now(void); // Get the current system time in seconds since midnight, Jan. 1 1970.

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                         Memory
 *---------------------------------------------------------------------------------------------------------------------------
 * Request big chunks of memory from the OS, bypassing the CRT. The system may round up your requested memory size, but it
 * will return an error instead of rounding down if there isn't enough memory.
 */
typedef struct
{
    void *mem;
    size size;
    b32 valid;
} CoyMemoryBlock;

#define COY_KB(a) ((a) * INT64_C(1000))
#define COY_MB(a) (COY_KB(a) * INT64_C(1000))
#define COY_GB(a) (COY_MB(a) * INT64_C(1000))
#define COY_TB(a) (COY_GB(a) * INT64_C(1000))

#define COY_KiB(a) ((a) * (1024))
#define COY_MiB(a) (COY_KiB(a) * INT64_C(1024))
#define COY_GiB(a) (COY_MiB(a) * INT64_C(1024))
#define COY_TiB(a) (COY_GiB(a) * INT64_C(1024))

static inline CoyMemoryBlock coy_memory_allocate(size minimum_num_bytes);
static inline void coy_memory_free(CoyMemoryBlock *mem);

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                     Files & Paths
 *---------------------------------------------------------------------------------------------------------------------------
 * Check the 'valid' member of the structs to check for errors!
 */

typedef struct
{
    char *start; /* NULL indicates not a valid path element. */
    size len;    /* 0 indicates not a valid path element.    */
} CoyPathStr;

typedef struct
{
    char *full_path;       /* Pointer to null terminated string. (Non-owning!!!)        */
    CoyPathStr dir;        /* Directory, not including terminating directory separator. */
    CoyPathStr base;       /* Last element in the path, if it is a file.                */
    CoyPathStr extension;  /* File extension, if this path is a file.                   */
    b32 exists;            /* Does this path already exist on the system?               */
    b32 is_file;           /* Is this a file? If not, it's a directory.                 */
} CoyPathInfo;

/* If path exists and is a file, assumes file, if path exists and is a directory, assumes not file. If path does not exist,
 * assumes file IFF it has an extension. You can override any assumptions via the assume_file argument. In the event that
 * assume_file conflicts with the system information about an existing file, the system information is used.               */
static inline CoyPathInfo coy_path_info(char *path, b32 assume_file);

// Append new path to the path in path_buffer, return true on success or false on error. path_buffer must be zero terminated.
static inline b32 coy_path_append(size buf_len, char path_buffer[], char const *new_path);

static inline size coy_file_size(char const *filename); /* size of a file in bytes, -1 on error. */

#define COY_FILE_READER_BUF_SIZE COY_KiB(32)
typedef struct
{
    iptr handle; // posix returns an int and windows a HANDLE (e.g. void*), this should work for all of them.
    byte buffer[COY_FILE_READER_BUF_SIZE];
    size buf_cursor;
    size bytes_remaining;
    b32 valid;   // error indicator
} CoyFileReader;

static inline CoyFileReader coy_file_open_read(char const *filename);
static inline size coy_file_read(CoyFileReader *file, size buf_size, byte *buffer); /* return nbytes read or -1 on error                           */
static inline b32 coy_file_read_f64(CoyFileReader *file, f64 *val);
static inline b32 coy_file_read_i8(CoyFileReader *file, i8 *val);
static inline b32 coy_file_read_i16(CoyFileReader *file, i16 *val);
static inline b32 coy_file_read_i32(CoyFileReader *file, i32 *val);
static inline b32 coy_file_read_i64(CoyFileReader *file, i64 *val);
static inline b32 coy_file_read_u8(CoyFileReader *file, u8 *val);
static inline b32 coy_file_read_u16(CoyFileReader *file, u16 *val);
static inline b32 coy_file_read_u32(CoyFileReader *file, u32 *val);
static inline b32 coy_file_read_u64(CoyFileReader *file, u64 *val);
static inline b32 coy_file_read_str(CoyFileReader *file, size *len, char *str);     /* set len to buffer length, updated to actual size on return. */
static inline void coy_file_reader_close(CoyFileReader *file);                      /* Must set valid member to false on success or failure!       */

/* return size in bytes of the loaded data or -1 on error. If buffer is too small, load nothing and return -1 */
static inline size coy_file_slurp(char const *filename, size buf_size, byte *buffer);


#define COY_FILE_WRITER_BUF_SIZE COY_KiB(32)
typedef struct
{
    iptr handle; // posix returns an int and windows a HANDLE (e.g. void*), this should work for all of them.
    byte buffer[COY_FILE_WRITER_BUF_SIZE];
    size buf_cursor;
    b32 valid;   // error indicator
} CoyFileWriter;

static inline CoyFileWriter coy_file_create(char const *filename); // Truncate if it already exists, otherwise create it.
static inline CoyFileWriter coy_file_append(char const *filename); // Create file if it doesn't exist yet, otherwise append.
static inline size coy_file_writer_flush(CoyFileWriter *file); /* Close will also do this, only use if ALL you need is flush */
static inline void coy_file_writer_close(CoyFileWriter *file); /* Must set valid member to false on success or failure! */

static inline size coy_file_write(CoyFileWriter *file, size nbytes_write, byte const *buffer); // return nbytes written or -1 on error
static inline b32 coy_file_write_f64(CoyFileWriter *file, f64 val);
static inline b32 coy_file_write_i8(CoyFileWriter *file, i8 val);
static inline b32 coy_file_write_i16(CoyFileWriter *file, i16 val);
static inline b32 coy_file_write_i32(CoyFileWriter *file, i32 val);
static inline b32 coy_file_write_i64(CoyFileWriter *file, i64 val);
static inline b32 coy_file_write_u8(CoyFileWriter *file, u8 val);
static inline b32 coy_file_write_u16(CoyFileWriter *file, u16 val);
static inline b32 coy_file_write_u32(CoyFileWriter *file, u32 val);
static inline b32 coy_file_write_u64(CoyFileWriter *file, u64 val);
static inline b32 coy_file_write_str(CoyFileWriter *file, size len, char *str);

typedef struct
{
    size size_in_bytes;     // size of the file
    byte const *data; 
    iptr _internal[2];      // implementation specific data
    b32 valid;              // error indicator
} CoyMemMappedFile;

static inline CoyMemMappedFile coy_memmap_read_only(char const *filename);
static inline void coy_memmap_close(CoyMemMappedFile *file);

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                File System Interactions
 *---------------------------------------------------------------------------------------------------------------------------
 * Check the 'valid' member of the structs to check for errors!
 *
 * WARNING: NONE OF THESE ARE THREADSAFE.
 */

typedef struct
{
    iptr os_handle;         // for internal use only
    char const *file_extension;
    b32 valid;
} CoyFileNameIter;

// Create an iterator. file_extension can be NULL if you want all files. Does not list directories. NOT THREADSAFE.
static inline CoyFileNameIter coy_file_name_iterator_open(char const *directory_path, char const *file_extension);

// Returns NULL when done. Copy the string if you need it, it will be overwritten on the next call. NOT THREADSAFE.
static inline char const *coy_file_name_iterator_next(CoyFileNameIter *cfni);
static inline void coy_file_name_iterator_close(CoyFileNameIter *cfin); // should leave the argument zeroed. NOT THREADSAFE.

/*---------------------------------------------------------------------------------------------------------------------------
 *                                         Dynamically Loading Shared Libraries
 *---------------------------------------------------------------------------------------------------------------------------
 *
 * WARNING: NONE OF THESE ARE THREADSAFE.
 */
typedef struct
{
    void *handle;
} CoySharedLibHandle;

static inline CoySharedLibHandle coy_shared_lib_load(char const *lib_name);
static inline void coy_shared_lib_unload(CoySharedLibHandle handle);
static inline void *coy_share_lib_load_symbol(CoySharedLibHandle handle, char const *symbol_name);

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                     Terminal Info
 *-------------------------------------------------------------------------------------------------------------------------*/
typedef struct
{
    i32 columns;
    i32 rows;
} CoyTerminalSize;

static inline CoyTerminalSize coy_get_terminal_size(void);

/*---------------------------------------------------------------------------------------------------------------------------
 *                                           Multi-threading & Syncronization
 *---------------------------------------------------------------------------------------------------------------------------
 * Basic threading and syncronization.
 */

/* Windows requires a u32 return type while Linux (pthreads) requires a void*. Just return 0 or 1 to indicate success
 * and Coyote will cast it to the correct type for the API.
 */

typedef void (*CoyThreadFunc)(void *thread_data);

typedef struct
{
    _Alignas(16) byte handle[32];
    CoyThreadFunc func;
    void *thread_data;
} CoyThread;

typedef struct 
{
    _Alignas(16) byte mutex[64];
    b32 valid;
} CoyMutex;

typedef struct
{
    _Alignas(16) byte cond_var[64];
    b32 valid;
} CoyCondVar;

static inline b32 coy_thread_create(CoyThread *thrd, CoyThreadFunc func, void *thread_data); /* Returns NULL on failure. */
static inline b32 coy_thread_join(CoyThread *thread); /* Returns false if there was an error. */
static inline void coy_thread_destroy(CoyThread *thread);

static inline CoyMutex coy_mutex_create(void);
static inline b32 coy_mutex_lock(CoyMutex *mutex);    /* Block, return false on failure. */
static inline b32 coy_mutex_unlock(CoyMutex *mutex);  /* Return false on failure.        */
static inline void coy_mutex_destroy(CoyMutex *mutex); /* Must set valid member to false. */

static inline CoyCondVar coy_condvar_create(void);
static inline b32 coy_condvar_sleep(CoyCondVar *cv, CoyMutex *mtx);
static inline b32 coy_condvar_wake(CoyCondVar *cv);
static inline b32 coy_condvar_wake_all(CoyCondVar *cv);
static inline void coy_condvar_destroy(CoyCondVar *cv); /* Must set valid member to false. */

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                  Thread Safe Channel
 *---------------------------------------------------------------------------------------------------------------------------
 * Threadsafe channel for sending / receiving pointers. Multiple Producer / Multiple Consumer (mpmc)
 */
#define COYOTE_CHANNEL_SIZE 16
typedef struct
{
    size head;
    size tail;
    size count;
    CoyMutex mtx;
    CoyCondVar space_available;
    CoyCondVar data_available;
    i32 num_producers_started;
    i32 num_producers_finished;
    i32 num_consumers_started;
    i32 num_consumers_finished;
    void *buf[COYOTE_CHANNEL_SIZE];
} CoyChannel;

static inline CoyChannel coy_channel_create(void);
static inline void coy_channel_destroy(CoyChannel *chan, void(*free_func)(void *ptr, void *ctx), void *free_ctx);
static inline void coy_channel_wait_until_ready_to_receive(CoyChannel *chan);
static inline void coy_channel_wait_until_ready_to_send(CoyChannel *chan);
static inline void coy_channel_register_sender(CoyChannel *chan);
static inline void coy_channel_register_receiver(CoyChannel *chan);
static inline void coy_channel_done_sending(CoyChannel *chan);
static inline void coy_channel_done_receiving(CoyChannel *chan);
static inline b32 coy_channel_send(CoyChannel *chan, void *data);
static inline b32 coy_channel_receive(CoyChannel *chan, void **out);

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                    Profiling Tools
 *---------------------------------------------------------------------------------------------------------------------------
 * Macro enabled tools for profiling code.
 */

#ifndef COY_PROFILE
#define COY_PROFILE 0
#endif

#if COY_PROFILE
#define COY_PROFILE_NUM_BLOCKS 64
#else
#define COY_PROFILE_NUM_BLOCKS 1
#endif

typedef struct
{
    u64 tsc_elapsed_inclusive;
    u64 tsc_elapsed_exclusive;

    u64 hit_count;
    i32 ref_count;

    u64 bytes;

    char const *label;

    double exclusive_pct;
    double inclusive_pct;
    double gibibytes_per_second;
} CoyBlockProfiler;

#if COY_PROFILE
typedef struct
{
    u64 start;
    u64 old_tsc_elapsed_inclusive;
    i32 index;
    i32 parent_index;
} CoyProfileAnchor;
#else
typedef u32 CoyProfileAnchor;
#endif

typedef struct
{
    CoyBlockProfiler blocks[COY_PROFILE_NUM_BLOCKS];
    i32 current_block;

    u64 start;

    double total_elapsed;
    u64 freq;
} GlobalProfiler;

static GlobalProfiler coy_global_profiler;

/* CPU & timing */
static inline void coy_profile_begin(void);
static inline void coy_profile_end(void);
static inline u64 coy_profile_read_cpu_timer(void);
static inline u64 coy_profile_estimate_cpu_timer_freq(void);

/* OS Counters (page faults, etc.) */
static inline void coy_profile_initialize_os_metrics(void);
static inline void coy_profile_finalize_os_metrics(void);
static inline u64 coy_profile_read_os_page_fault_count(void);

#define COY_START_PROFILE_FUNCTION_BYTES(bytes) coy_profile_start_block(__func__, __COUNTER__, bytes)
#define COY_START_PROFILE_FUNCTION coy_profile_start_block(__func__, __COUNTER__, 0)
#define COY_START_PROFILE_BLOCK_BYTES(name, bytes) coy_profile_start_block(name, __COUNTER__, bytes)
#define COY_START_PROFILE_BLOCK(name) coy_profile_start_block(name, __COUNTER__, 0)
#define COY_END_PROFILE(ap) coy_profile_end_block(&ap)

#if COY_PROFILE
#define COY_PROFILE_STATIC_CHECK _Static_assert(__COUNTER__ <= COY_ARRAY_SIZE(coy_global_profiler.blocks), "not enough profiler blocks")
#else
#define COY_PROFILE_STATIC_CHECK
#endif
/*---------------------------------------------------------------------------------------------------------------------------
 *
 *
 *
 *                                          Implementation of `inline` functions.
 *                                                      Internal Only
 *
 *
 *
 *-------------------------------------------------------------------------------------------------------------------------*/

// assumes zero terminated string returned from OS - not for general use.
static inline char const *
coy_file_extension(char const *path)
{
    char const *extension = path;
    char const *next_char = path;
    while(*next_char)
    {
        if(*next_char == '.')
        {
            extension = next_char + 1;
        }
        ++next_char;
    }
    return extension;
}

// assumes zero terminated string returned from OS - not for general use.
static inline b32
coy_null_term_strings_equal(char const *left, char const *right)
{
    char const *l = left;
    char const *r = right;

    while(*l || *r)
    {
        if(*l != *r)
        {
            return false;
        }

        ++l;
        ++r;
    }

    return true;
}

static inline b32 
coy_file_write_f64(CoyFileWriter *file, f64 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline b32 
coy_file_write_i8(CoyFileWriter *file, i8 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline b32 
coy_file_write_i16(CoyFileWriter *file, i16 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline b32 
coy_file_write_i32(CoyFileWriter *file, i32 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline b32 
coy_file_write_i64(CoyFileWriter *file, i64 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline b32 
coy_file_write_u8(CoyFileWriter *file, u8 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline b32 
coy_file_write_u16(CoyFileWriter *file, u16 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline b32 
coy_file_write_u32(CoyFileWriter *file, u32 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline b32 
coy_file_write_u64(CoyFileWriter *file, u64 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline b32 
coy_file_write_str(CoyFileWriter *file, size len, char *str)
{
    _Static_assert(sizeof(size) == sizeof(i64), "must not be on 64 bit!");
    b32 success = coy_file_write_i64(file, len);
    StopIf(!success, return false);
    if(len > 0)
    {
        size nbytes = coy_file_write(file, len, (byte *)str);
        if(nbytes != len) { return false; }
    }
    return true;
}

static inline b32 
coy_file_read_f64(CoyFileReader *file, f64 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline b32 
coy_file_read_i8(CoyFileReader *file, i8 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline b32 
coy_file_read_i16(CoyFileReader *file, i16 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline b32 
coy_file_read_i32(CoyFileReader *file, i32 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline b32 
coy_file_read_i64(CoyFileReader *file, i64 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline b32 
coy_file_read_u8(CoyFileReader *file, u8 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline b32 
coy_file_read_u16(CoyFileReader *file, u16 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline b32 
coy_file_read_u32(CoyFileReader *file, u32 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline b32 
coy_file_read_u64(CoyFileReader *file, u64 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline b32 
coy_file_read_str(CoyFileReader *file, size *len, char *str)
{
    i64 str_len = 0;
    b32 success = coy_file_read_i64(file, &str_len);
    StopIf(!success || str_len > *len, return false);

    if(str_len > 0)
    {
        success = coy_file_read(file, str_len, (byte *)str) > 0;
        StopIf(!success, return false);
    }
    else
    {
        /* Clear the provided buffer. */
        memset(str, 0, *len);
    }

    *len = str_len;
    return true;
}

static inline CoyChannel 
coy_channel_create(void)
{
    CoyChannel chan = 
        {
            .head = 0,
            .tail = 0,
            .count = 0,
            .mtx = coy_mutex_create(),
            .space_available = coy_condvar_create(),
            .data_available = coy_condvar_create(),
            .num_producers_started = 0,
            .num_producers_finished = 0,
            .num_consumers_started = 0,
            .num_consumers_finished = 0,
            .buf = {0},
        };

    Assert(chan.mtx.valid);
    Assert(chan.space_available.valid);
    Assert(chan.data_available.valid);

    return chan;
}

static inline void 
coy_channel_destroy(CoyChannel *chan, void(*free_func)(void *ptr, void *ctx), void *free_ctx)
{
    Assert(chan->num_producers_started == chan->num_producers_finished);
    Assert(chan->num_consumers_started == chan->num_consumers_finished);
    if(free_func)
    {
        while(chan->count > 0)
        {
            free_func(chan->buf[chan->head % COYOTE_CHANNEL_SIZE], free_ctx);
            chan->head += 1;
            chan->count -= 1;
        }

        Assert(chan->head == chan->tail && chan->count == 0);
    }

    coy_mutex_destroy(&chan->mtx);
    coy_condvar_destroy(&chan->space_available);
    coy_condvar_destroy(&chan->data_available);
    *chan = (CoyChannel){0};
}

static inline void 
coy_channel_wait_until_ready_to_receive(CoyChannel *chan)
{
    b32 success = coy_mutex_lock(&chan->mtx);
    Assert(success);

    while(chan->num_producers_started == 0) 
    {
        coy_condvar_sleep(&chan->data_available, &chan->mtx); 
    }

    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);
}

static inline void 
coy_channel_wait_until_ready_to_send(CoyChannel *chan)
{
    b32 success = coy_mutex_lock(&chan->mtx);
    Assert(success);
    while(chan->num_consumers_started == 0) { coy_condvar_sleep(&chan->space_available, &chan->mtx); }
    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);
}

static inline void 
coy_channel_register_sender(CoyChannel *chan)
{
    b32 success = coy_mutex_lock(&chan->mtx);
    Assert(success);
    chan->num_producers_started += 1;

    if(chan->num_producers_started == 1)
    {
        /* Broadcast here so any threads blocked on coy_channel_wait_until_ready_to_receive can progress. If the number of
         * producers is greater than 1, then this was already sent.
         */
        success = coy_condvar_wake_all(&chan->data_available);
        Assert(success);
    }

    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);
}

static inline void 
coy_channel_register_receiver(CoyChannel *chan)
{
    b32 success = coy_mutex_lock(&chan->mtx);
    Assert(success);

    chan->num_consumers_started += 1;

    if(chan->num_consumers_started == 1)
    {
        /* Broadcast here so any threads blocked on coy_channel_wait_until_ready_to_send can progress. If the number of
         * consumers is greater than 1, then this was already sent.
         */
        success = coy_condvar_wake_all(&chan->space_available);
        Assert(success);
    }

    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);
}

static inline void 
coy_channel_done_sending(CoyChannel *chan)
{
    b32 success = coy_mutex_lock(&chan->mtx);
    Assert(success);
    Assert(chan->num_producers_started > 0);

    chan->num_producers_finished += 1;

    if(chan->num_producers_started == chan->num_producers_finished)
    {
        /* Broadcast in case any thread is waiting for data to become available that won't because there are no more
         * producers running. This will let them check the number of producers started/finished and realize nothing is
         * coming so they too can quit.
         */
        success = coy_condvar_wake_all(&chan->data_available);
        Assert(success);
    }
    else
    {
        /* Deadlock may occur if I don't do this. This thread got signaled when others should have. */
        success = coy_condvar_wake_all(&chan->space_available);
        Assert(success);
    }

    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);
}

static inline void 
coy_channel_done_receiving(CoyChannel *chan)
{
    b32 success = coy_mutex_lock(&chan->mtx);
    Assert(success);
    Assert(chan->num_consumers_started > 0);

    chan->num_consumers_finished += 1;

    if(chan->num_consumers_started == chan->num_consumers_finished)
    {
        /* Broadcast in case any thread is waiting to send data that they will never be able to send because there are no
         * consumers to receive it! This will let them check the number of consumers started/finished and realize space 
         * will never become available.
         */
        success = coy_condvar_wake_all(&chan->space_available);
        Assert(success);
    }
    else
    {
        /* Deadlock may occur if I don't do this. This thread got signaled when others should have. */
        success = coy_condvar_wake_all(&chan->data_available);
        Assert(success);
    }

    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);
}

static inline b32 
coy_channel_send(CoyChannel *chan, void *data)
{
    b32 success = coy_mutex_lock(&chan->mtx);
    Assert(success);
    Assert(chan->num_producers_started > 0);

    while(chan->count == COYOTE_CHANNEL_SIZE && chan->num_consumers_started != chan->num_consumers_finished)
    {
        success = coy_condvar_sleep(&chan->space_available, &chan->mtx);
        Assert(success);
    }

    Assert(chan->count < COYOTE_CHANNEL_SIZE || chan->num_consumers_started == chan->num_consumers_finished);

    if(chan->num_consumers_started > chan->num_consumers_finished)
    {
        chan->buf[(chan->tail) % COYOTE_CHANNEL_SIZE] = data;
        chan->tail += 1;
        chan->count += 1;

        /* If the count was increased to 1, then someone may have been waiting to be notified! */
        if(chan->count == 1)
        {
            success = coy_condvar_wake_all(&chan->data_available);
            Assert(success);
        }

        success = coy_mutex_unlock(&chan->mtx);
        Assert(success);
        return true;
    }
    else
    {
        /* Space will never become available. */
        success = coy_mutex_unlock(&chan->mtx);
        Assert(success);
        return false;
    }
}

static inline b32 
coy_channel_receive(CoyChannel *chan, void **out)
{
    b32 success = coy_mutex_lock(&chan->mtx);
    Assert(success);

    while(chan->count == 0 && chan->num_producers_started > chan->num_producers_finished)
    {
        success = coy_condvar_sleep(&chan->data_available, &chan->mtx);
        Assert(success);
    }

    Assert(chan->count > 0 || chan->num_producers_started == chan->num_producers_finished);

    *out = NULL;
    if(chan->count > 0)
    {
        *out = chan->buf[(chan->head) % COYOTE_CHANNEL_SIZE];
        chan->head += 1;
        chan->count -= 1;
    }
    else
    {
        /* Nothing more is coming, to get here num_producers_started must num_producers_finished. */
        success = coy_mutex_unlock(&chan->mtx);
        Assert(success);
        return false;
    }

    /* If the queue was full before, send a signal to let other's know there is room now. */
    if( chan->count + 1 == COYOTE_CHANNEL_SIZE)
    {
        success = coy_condvar_wake_all(&chan->space_available);
        Assert(success);
    }
    
    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);

    return true;
}

typedef struct
{
    b32 initialized;
    uptr handle;
} CoyOsMetrics;

static CoyOsMetrics coy_global_os_metrics;

/* NOTE: force the __COUNTER__ to start at 1, so I can offset and NOT use position 0 of global_profiler.blocks array */
i64 dummy =  __COUNTER__; 

static inline u64 coy_profile_get_os_timer_freq(void);
static inline u64 coy_profile_read_os_timer(void);

static inline u64
coy_profile_estimate_cpu_timer_freq(void)
{
	u64 milliseconds_to_wait = 100;
	u64 os_freq = coy_profile_get_os_timer_freq();

	u64 cpu_start = coy_profile_read_cpu_timer();
	u64 os_start = coy_profile_read_os_timer();
	u64 os_end = 0;
	u64 os_elapsed = 0;
	u64 os_wait_time = os_freq * milliseconds_to_wait / 1000;
	while(os_elapsed < os_wait_time)
	{
		os_end = coy_profile_read_os_timer();
		os_elapsed = os_end - os_start;
	}
	
	u64 cpu_end = coy_profile_read_cpu_timer();
	u64 cpu_elapsed = cpu_end - cpu_start;
	
	u64 cpu_freq = 0;
	if(os_elapsed)
	{
		cpu_freq = os_freq * cpu_elapsed / os_elapsed;
	}
	
	return cpu_freq;
}

static inline void
coy_profile_begin(void)
{
    coy_global_profiler.start = coy_profile_read_cpu_timer();
    coy_global_profiler.blocks[0].label = "Global";
    coy_global_profiler.blocks[0].hit_count++;
}

#pragma warning(disable : 4723)
static inline void
coy_profile_end(void)
{
    u64 end = coy_profile_read_cpu_timer();
    u64 total_elapsed = end - coy_global_profiler.start;

    CoyBlockProfiler *gp = coy_global_profiler.blocks;
    gp->tsc_elapsed_inclusive = total_elapsed;
    gp->tsc_elapsed_exclusive += total_elapsed;

    u64 freq = coy_profile_estimate_cpu_timer_freq();
    f64 const ZERO = 0.0;
    f64 const A_NAN = 0.0 / ZERO;
    if(freq)
    {
        coy_global_profiler.total_elapsed = (double)total_elapsed / (double)freq;
        coy_global_profiler.freq = freq;
    }
    else
    {
        coy_global_profiler.total_elapsed = A_NAN;
        coy_global_profiler.freq = 0;
    }

    for(i32 i = 0; i < COY_ARRAY_SIZE(coy_global_profiler.blocks); ++i)
    {
        CoyBlockProfiler *block = coy_global_profiler.blocks + i;
        if(block->tsc_elapsed_inclusive)
        {
            block->exclusive_pct = (double)block->tsc_elapsed_exclusive / (double) total_elapsed * 100.0;
            block->inclusive_pct = (double)block->tsc_elapsed_inclusive / (double) total_elapsed * 100.0;
            if(block->bytes && freq)
            {
                double gib = (double)block->bytes / (1024 * 1024 * 1024);
                block->gibibytes_per_second = gib * (double) freq / (double)block->tsc_elapsed_inclusive;
            }
            else
            {
                block->gibibytes_per_second = A_NAN;
            }
        }
        else
        {
            block->exclusive_pct = A_NAN;
            block->inclusive_pct = A_NAN;
            block->gibibytes_per_second = A_NAN;
        }
    }
}
#pragma warning(default : 4723)

static inline CoyProfileAnchor 
coy_profile_start_block(char const *label, i32 index, u64 bytes_processed)
{
#if COY_PROFILE
    /* Update global state */
    i32 parent_index = coy_global_profiler.current_block;
    coy_global_profiler.current_block = index;

    /* update block profiler for this block */
    CoyBlockProfiler *block = coy_global_profiler.blocks + index;
    block->hit_count++;
    block->ref_count++;
    block->bytes += bytes_processed;
    block->label = label; /* gotta do it every time */

    /* build anchor to track this block */
    u64 start = coy_profile_read_cpu_timer();
    return (CoyProfileAnchor)
        { 
            .index = index, 
            .parent_index = parent_index, 
            .start=start, 
            .old_tsc_elapsed_inclusive = block->tsc_elapsed_inclusive 
        };
#else
    return -1;
#endif
}

static inline void 
coy_profile_end_block(CoyProfileAnchor *anchor)
{
#if COY_PROFILE
    /* read the end time, hopefully before the rest of this stuff */
    u64 end = coy_profile_read_cpu_timer();
    u64 elapsed = end - anchor->start;

    /* update global state */
    coy_global_profiler.current_block = anchor->parent_index;

    /* update the parent block profilers state */
    CoyBlockProfiler *parent = coy_global_profiler.blocks + anchor->parent_index;
    parent->tsc_elapsed_exclusive -= elapsed;

    /* update block profiler state */
    CoyBlockProfiler *block = coy_global_profiler.blocks + anchor->index;
    block->tsc_elapsed_exclusive += elapsed;
    block->tsc_elapsed_inclusive = anchor->old_tsc_elapsed_inclusive + elapsed;
    block->ref_count--;
#endif
}


#if defined(_WIN32) || defined(_WIN64)

#pragma warning(disable: 4142)
#ifndef _COYOTE_WIN32_H_
#define _COYOTE_WIN32_H_

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                 Windows Implementation
 *-------------------------------------------------------------------------------------------------------------------------*/
#include <windows.h>
#include <bcrypt.h>
#include <intrin.h>
#include <psapi.h>

_Static_assert(UINT32_MAX < INTPTR_MAX, "DWORD cannot be cast to intptr_t safely.");

union WinTimePun
{
    FILETIME ft;
    ULARGE_INTEGER as_uint64;
};

static inline u64
coy_time_now(void)
{
    SYSTEMTIME now_st = {0};
    GetSystemTime(&now_st);

    union WinTimePun now = {0};
    b32 success = SystemTimeToFileTime(&now_st, &now.ft);
    StopIf(!success, goto ERR_RETURN);

    SYSTEMTIME epoch_st = { 
        .wYear=1970, 
        .wMonth=1,
        .wDayOfWeek=4,
        .wDay=1,
        .wHour=0,
        .wMinute=0,
        .wSecond=0,
        .wMilliseconds=0
    };

    union WinTimePun epoch = {0};
    success = SystemTimeToFileTime(&epoch_st, &epoch.ft);
    PanicIf(!success);

    return (now.as_uint64.QuadPart - epoch.as_uint64.QuadPart) / 10000000;

ERR_RETURN:
    return UINT64_MAX;
}

static char const coy_path_sep = '\\';

static inline CoyPathInfo
coy_path_info(char *path, b32 assume_file)
{
    CoyPathInfo info = {0};
    info.full_path = path;

    /* Get the length. */
    size len = 0;
    char *c = path;
    while(*c && len < MAX_PATH)
    {
        ++len;
        ++c;
    }

    /* Check for error condition. */
    if(len == 0 || len >= MAX_PATH)
    {
        return info; /* Return empty info, there's nothing here. */
    }

    /* Find extension, and base.*/
    size idx = len - 1;
    c = &path[idx];
    if(*c == coy_path_sep) /* Skip a trailing slash. */
    {
        --c;
        --idx;
    }
    size next_len = 0;
    while(idx >= 0)
    {
        /* Found the extension. */
        if(*c == '.' && !info.extension.start)
        {
            info.extension = (CoyPathStr){ .start = c + 1, .len = next_len };
        }

        /* Found file name, but check to make sure it's not just a trailing slash. */
        if(*c == coy_path_sep && !info.base.start && (assume_file || info.extension.start) && (idx < len - 1))
        {
            info.base = (CoyPathStr){ .start = c + 1, .len = next_len };
            info.is_file = true; /* Assume this is a file. */
            next_len = -1;
        }

        /* Increment counters. */
        ++next_len;
        --idx;
        --c;
    }

    info.dir.start = path;
    info.dir.len = next_len;

    /* Does it exist? */
    DWORD attr = GetFileAttributes(path);
    if(attr == INVALID_FILE_ATTRIBUTES)
    {
        info.exists = false;
    }
    else
    {
        info.exists = true;
        info.is_file = FILE_ATTRIBUTE_DIRECTORY & attr ? false : true;
    }

    return info;
}

static inline b32 
coy_path_append(size buf_len, char path_buffer[], char const *new_path)
{
    // Find first '\0'
    size position = 0;
    char *c = path_buffer;
    while(position < buf_len && *c)
    {
        ++c;
        position += 1;
    }

    StopIf(position >= buf_len, goto ERR_RETURN);

    // Add a path separator - unless the buffer is empty or the last path character was a path separator.
    if(position > 0 && path_buffer[position - 1] != coy_path_sep)
    {
        path_buffer[position] = coy_path_sep;
        position += 1;
        StopIf(position >= buf_len, goto ERR_RETURN);
    }

    // Copy in the new path part.
    char const *new_c = new_path;
    while(position < buf_len && *new_c)
    {
        path_buffer[position] = *new_c;
        ++new_c;
        position += 1;
    }

    StopIf(position >= buf_len, goto ERR_RETURN);

    // Null terminate the path.
    path_buffer[position] = '\0';
    
    return true;

ERR_RETURN:
    path_buffer[buf_len - 1] = '\0';
    return false;
}

static inline CoyFileWriter
coy_file_create(char const *filename)
{
    HANDLE fh = CreateFileA(filename,              // [in]           LPCSTR                lpFileName,
                            GENERIC_WRITE,         // [in]           DWORD                 dwDesiredAccess,
                            0,                     // [in]           DWORD                 dwShareMode,
                            NULL,                  // [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                            CREATE_ALWAYS,         // [in]           DWORD                 dwCreationDisposition,
                            FILE_ATTRIBUTE_NORMAL, // [in]           DWORD                 dwFlagsAndAttributes,
                            NULL);                 // [in, optional] HANDLE                hTemplateFile

    if(fh != INVALID_HANDLE_VALUE)
    {
        return (CoyFileWriter){.handle = (iptr)fh, .valid = true};
    }
    else
    {
        return (CoyFileWriter){.handle = (iptr)INVALID_HANDLE_VALUE, .valid = false};
    }
}

static inline CoyFileWriter
coy_file_append(char const *filename)
{
    HANDLE fh = CreateFileA(filename,              // [in]           LPCSTR                lpFileName,
                            FILE_APPEND_DATA,      // [in]           DWORD                 dwDesiredAccess,
                            0,                     // [in]           DWORD                 dwShareMode,
                            NULL,                  // [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                            OPEN_ALWAYS,           // [in]           DWORD                 dwCreationDisposition,
                            FILE_ATTRIBUTE_NORMAL, // [in]           DWORD                 dwFlagsAndAttributes,
                            NULL);                 // [in, optional] HANDLE                hTemplateFile

    if(fh != INVALID_HANDLE_VALUE)
    {
        return (CoyFileWriter){.handle = (iptr)fh, .valid = true};
    }
    else
    {
        return (CoyFileWriter){.handle = (iptr)INVALID_HANDLE_VALUE, .valid = false};
    }
}

static inline size 
coy_file_writer_flush(CoyFileWriter *file)
{
    StopIf(!file->valid, goto ERR_RETURN);

    if(file->buf_cursor)
    {
        DWORD nbytes_written = 0;
        BOOL success = WriteFile(
            (HANDLE)file->handle,     // [in]                HANDLE       hFile,
            file->buffer,             // [in]                LPCVOID      lpBuffer,
            (DWORD)file->buf_cursor,  // [in]                DWORD        nNumberOfBytesToWrite,
            &nbytes_written,          // [out, optional]     LPDWORD      lpNumberOfBytesWritten,
            NULL                      // [in, out, optional] LPOVERLAPPED lpOverlapped
        );

        StopIf(!success || file->buf_cursor != (size)nbytes_written, goto ERR_RETURN);

        file->buf_cursor = 0;

        return (size)nbytes_written;
    }

    return 0;

ERR_RETURN:
    return -1;
}

static inline void 
coy_file_writer_close(CoyFileWriter *file)
{
    /* TODO change API to return size so I can return an error if the flush fails. */
    coy_file_writer_flush(file);
    CloseHandle((HANDLE)file->handle);
    file->valid = false;

    return;
}

static inline size 
coy_file_write(CoyFileWriter *file, size nbytes_write, byte const *buffer)
{
    /* check to see if the buffer needs flushed. */
    if(file->buf_cursor + nbytes_write > COY_FILE_WRITER_BUF_SIZE)
    {
        size bytes_flushed = coy_file_writer_flush(file);
        StopIf(bytes_flushed < 0, goto ERR_RETURN);
    }

    if(nbytes_write < COY_FILE_WRITER_BUF_SIZE)
    {
        /* Dump small writes into the buffer. */
        memcpy(file->buffer + file->buf_cursor, buffer, nbytes_write);
        file->buf_cursor += nbytes_write;
        return nbytes_write;
    }
    else
    {
        /* Large writes bypass the buffer and go straight to the file. */
        Assert(INT32_MAX >= nbytes_write); /* Not prepared for REALLY large writes. */
        DWORD nbytes_written = 0;
        BOOL success = WriteFile(
            (HANDLE)file->handle,     // [in]                HANDLE       hFile,
            buffer,                   // [in]                LPCVOID      lpBuffer,
            (DWORD)nbytes_write,      // [in]                DWORD        nNumberOfBytesToWrite,
            &nbytes_written,          // [out, optional]     LPDWORD      lpNumberOfBytesWritten,
            NULL                      // [in, out, optional] LPOVERLAPPED lpOverlapped
        );

        StopIf(!success, goto ERR_RETURN);
        return (size)nbytes_written;
    }

ERR_RETURN:
    return -1;
}

static inline CoyFileReader
coy_file_open_read(char const *filename)
{
    HANDLE fh = CreateFileA(filename,              // [in]           LPCSTR                lpFileName,
                            GENERIC_READ,          // [in]           DWORD                 dwDesiredAccess,
                            FILE_SHARE_READ,       // [in]           DWORD                 dwShareMode,
                            NULL,                  // [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                            OPEN_EXISTING,         // [in]           DWORD                 dwCreationDisposition,
                            FILE_ATTRIBUTE_NORMAL, // [in]           DWORD                 dwFlagsAndAttributes,
                            NULL);                 // [in, optional] HANDLE                hTemplateFile

    if(fh != INVALID_HANDLE_VALUE)
    {
        return (CoyFileReader){.handle = (iptr)fh, .valid = true};
    }
    else
    {
        return (CoyFileReader){.handle = (iptr)INVALID_HANDLE_VALUE, .valid = false};
    }
}

static inline size 
coy_file_fill_buffer(CoyFileReader *file)
{
    if(file->bytes_remaining > 0)
    {
        /* Move remaining data to the front of the buffer */
        memmove(file->buffer, file->buffer + file->buf_cursor, file->bytes_remaining);
    }
    file->buf_cursor = 0;

    size space_available = COY_FILE_READER_BUF_SIZE - file->bytes_remaining;

    Assert(INT32_MAX >= space_available); /* Not prepared for REALLY large reads. */
    DWORD nbytes_read = 0;
    BOOL success =  ReadFile((HANDLE) file->handle,                //  [in]                HANDLE       hFile,
                             file->buffer + file->bytes_remaining, //  [out]               LPVOID       lpBuffer,
                             (DWORD)space_available,               //  [in]                DWORD        nNumberOfBytesToRead,
                             &nbytes_read,                         //  [out, optional]     LPDWORD      lpNumberOfBytesRead,
                             NULL);                                //  [in, out, optional] LPOVERLAPPED lpOverlapped

    StopIf(!success, goto ERR_RETURN);

    file->bytes_remaining += nbytes_read;

    return (size)nbytes_read;

ERR_RETURN:
    return -1;
}

static inline size 
coy_file_read(CoyFileReader *file, size buf_size, byte *buffer)
{
    Assert(buf_size > 0);
    StopIf(!file->valid, goto ERR_RETURN);

    if(buf_size > file->bytes_remaining)
    {
        size bytes_read = coy_file_fill_buffer(file);
        StopIf(bytes_read < 0, goto ERR_RETURN);
    }

    size size_to_copy = buf_size > file->bytes_remaining ? file->bytes_remaining : buf_size;
    memcpy(buffer, file->buffer + file->buf_cursor, size_to_copy);
    file->buf_cursor += size_to_copy;
    file->bytes_remaining -= size_to_copy;

    return size_to_copy;

ERR_RETURN:
    return -1;
}

static inline void 
coy_file_reader_close(CoyFileReader *file)
{
    CloseHandle((HANDLE)file->handle);
    file->valid = false;

    return;
}

static inline size 
coy_file_slurp(char const *filename, size buf_size, byte *buffer)
{
    size file_size = coy_file_size(filename);
    StopIf(file_size < 1 || file_size > buf_size, goto ERR_RETURN);

    HANDLE fh = CreateFileA(filename,              // [in]           LPCSTR                lpFileName,
                            GENERIC_READ,          // [in]           DWORD                 dwDesiredAccess,
                            FILE_SHARE_READ,       // [in]           DWORD                 dwShareMode,
                            NULL,                  // [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                            OPEN_EXISTING,         // [in]           DWORD                 dwCreationDisposition,
                            FILE_ATTRIBUTE_NORMAL, // [in]           DWORD                 dwFlagsAndAttributes,
                            NULL);                 // [in, optional] HANDLE                hTemplateFile

    StopIf(fh == INVALID_HANDLE_VALUE, goto ERR_RETURN);
    
    size space_available = buf_size;

    Assert(INT32_MAX >= space_available); /* Not prepared for REALLY large reads. */
    DWORD nbytes_read = 0;
    BOOL success =  ReadFile(fh,                     //  [in]                HANDLE       hFile,
                             buffer,                 //  [out]               LPVOID       lpBuffer,
                             (DWORD)space_available, //  [in]                DWORD        nNumberOfBytesToRead,
                             &nbytes_read,           //  [out, optional]     LPDWORD      lpNumberOfBytesRead,
                             NULL);                  //  [in, out, optional] LPOVERLAPPED lpOverlapped

    success &= CloseHandle(fh);

    StopIf(!success, goto ERR_RETURN);

    return (size)nbytes_read;

ERR_RETURN:
    return -1;
}

static inline size 
coy_file_size(char const *filename)
{
    WIN32_FILE_ATTRIBUTE_DATA attr = {0};
    BOOL success = GetFileAttributesExA(filename, GetFileExInfoStandard, &attr);
    StopIf(!success, goto ERR_RETURN);

    _Static_assert(sizeof(uptr) == 2 * sizeof(DWORD) && sizeof(DWORD) == 4);
    uptr file_size = ((uptr)attr.nFileSizeHigh << 32) | attr.nFileSizeLow;
    StopIf(file_size > INTPTR_MAX, goto ERR_RETURN);

    return (size) file_size;

ERR_RETURN:
    return -1;
}

static inline CoyMemMappedFile 
coy_memmap_read_only(char const *filename)
{
    CoyFileReader cf = coy_file_open_read(filename);
    StopIf(!cf.valid, goto ERR_RETURN);

    HANDLE fmh =  CreateFileMappingA((HANDLE)cf.handle, // [in]           HANDLE                hFile,
                                     NULL,              // [in, optional] LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
                                     PAGE_READONLY,     // [in]           DWORD                 flProtect,
                                     0,                 // [in]           DWORD                 dwMaximumSizeHigh,
                                     0,                 // [in]           DWORD                 dwMaximumSizeLow,
                                     NULL);             // [in, optional] LPCSTR                lpName
    StopIf(fmh == INVALID_HANDLE_VALUE, goto CLOSE_CF_AND_ERR);

    LPVOID ptr =  MapViewOfFile(fmh,           // [in] HANDLE hFileMappingObject,
                                FILE_MAP_READ, // [in] DWORD  dwDesiredAccess,
                                0,             // [in] DWORD  dwFileOffsetHigh,
                                0,             // [in] DWORD  dwFileOffsetLow,
                                0);            // [in] SIZE_T dwNumberOfBytesToMap
    StopIf(!ptr, goto CLOSE_FMH_AND_ERR);

    // Get the size of the file mapped.
    DWORD file_size_high = 0;
    DWORD file_size_low = GetFileSize((HANDLE)cf.handle, &file_size_high);
    StopIf(file_size_low == INVALID_FILE_SIZE, goto CLOSE_FMH_AND_ERR);

    uptr file_size = ((uptr)file_size_high << 32) | file_size_low;
    StopIf(file_size > INTPTR_MAX, goto CLOSE_FMH_AND_ERR);


    return (CoyMemMappedFile){
      .size_in_bytes = (size)file_size, 
        .data = ptr, 
        ._internal = { cf.handle, (size)fmh }, 
        .valid = true 
    };

CLOSE_FMH_AND_ERR:
    CloseHandle(fmh);
CLOSE_CF_AND_ERR:
    coy_file_reader_close(&cf);
ERR_RETURN:
    return (CoyMemMappedFile) { .valid = false };
}

static inline void 
coy_memmap_close(CoyMemMappedFile *file)
{
    void const*data = file->data;
    iptr fh = file->_internal[0];
    HANDLE fmh = (HANDLE)file->_internal[1];

    /*BOOL success = */UnmapViewOfFile(data);
    CloseHandle(fmh);
    CoyFileReader cf = { .handle = fh, .valid = true };
    coy_file_reader_close(&cf);

    file->valid = false;

    return;
}

// I don't normally use static storage, but the linux interface uses it internally, so I'm stuck with those semantics.
// I'll use my own here.
static WIN32_FIND_DATA coy_file_name_iterator_data;
static char coy_file_name[1024];

static inline CoyFileNameIter
coy_file_name_iterator_open(char const *directory_path, char const *file_extension)
{
    char path_buf[1024] = {0};
    int i = 0;
    for(i = 0; i < sizeof(path_buf) && directory_path[i]; ++i)
    {
        path_buf[i] = directory_path[i];
    }
    StopIf(i + 2 >= sizeof(path_buf), goto ERR_RETURN);
    path_buf[i] = '\\';
    path_buf[i + 1] = '*';
    path_buf[i + 2] = '\0';

    HANDLE finder = FindFirstFile(path_buf, &coy_file_name_iterator_data);
    StopIf(finder == INVALID_HANDLE_VALUE, goto ERR_RETURN);
    return (CoyFileNameIter) { .os_handle=(iptr)finder, .file_extension=file_extension, .valid=true };

ERR_RETURN:
    return (CoyFileNameIter) { .valid=false };
}

static inline char const *
coy_file_name_iterator_next(CoyFileNameIter *cfni)
{
    if(cfni->valid)
    {
        // The first call to coy_file_name_iterator_open() should have populated
        char const *fname = coy_file_name_iterator_data.cFileName;
        b32 found = false;
        while(!found)
        {
            if(!(coy_file_name_iterator_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                if(cfni->file_extension == NULL)
                {
                    found = true;
                }
                else
                {
                    char const *ext = coy_file_extension(fname);
                    if(coy_null_term_strings_equal(ext, cfni->file_extension))
                    {
                        found = true;
                    }
                }
            }
            if(found)
            {
                int i = 0;
                for(i = 0; i < sizeof(coy_file_name) && fname[i]; ++i)
                {
                    coy_file_name[i] = fname[i];
                }
                coy_file_name[i] = '\0';
            }

            BOOL foundnext = FindNextFileA((HANDLE)cfni->os_handle, &coy_file_name_iterator_data);

            if(!foundnext)
            {
                cfni->valid = false;
                break;
            }
        }

        if(found) 
        {
            return coy_file_name;
        }
    }

    return NULL;
}

static inline void 
coy_file_name_iterator_close(CoyFileNameIter *cfin)
{
    HANDLE finder = (HANDLE)cfin->os_handle;
    FindClose(finder);
    *cfin = (CoyFileNameIter) {0};
    return;
}

static inline CoySharedLibHandle 
coy_shared_lib_load(char const *lib_name)
{
    void *h = LoadLibraryA(lib_name);
    PanicIf(!h);
    return (CoySharedLibHandle) { .handle = h };
}

static inline void 
coy_shared_lib_unload(CoySharedLibHandle handle)
{
    b32 success = FreeLibrary(handle.handle);
    PanicIf(!success);
}

static inline void *
coy_share_lib_load_symbol(CoySharedLibHandle handle, char const *symbol_name)
{
    void *s = GetProcAddress(handle.handle, symbol_name);
    PanicIf(!s);
    return s;
}

static inline CoyTerminalSize 
coy_get_terminal_size(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi = {0};
    BOOL success = GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

    if(success)
    {
        return (CoyTerminalSize) 
            {
                .columns = csbi.srWindow.Right - csbi.srWindow.Left + 1, 
                .rows =csbi.srWindow.Bottom - csbi.srWindow.Top + 1
            };
    }

    return (CoyTerminalSize) { .columns = -1, .rows = -1 };
}

static inline CoyMemoryBlock 
coy_memory_allocate(size minimum_num_bytes)
{
    if(minimum_num_bytes <= 0)
    {
         return (CoyMemoryBlock){.mem = 0, .size = minimum_num_bytes, .valid = false };
    }

    SYSTEM_INFO info = {0};
    GetSystemInfo(&info);
    uptr page_size = info.dwPageSize;
    uptr alloc_gran = info.dwAllocationGranularity;

    uptr target_granularity = (uptr)minimum_num_bytes > alloc_gran ? alloc_gran : page_size;

    uptr allocation_size = minimum_num_bytes;
    if(minimum_num_bytes % target_granularity)
    {
        allocation_size += target_granularity - (minimum_num_bytes % target_granularity);
    }

    if(allocation_size > INTPTR_MAX)
    {
         return (CoyMemoryBlock){.mem = 0, .size = INTPTR_MAX, .valid = false };
    }
    size a_size = (size)allocation_size;

    void *mem = VirtualAlloc(NULL, allocation_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if(!mem)
    {
         return (CoyMemoryBlock){.mem = 0, .size = 0, .valid = false };
    }

    return (CoyMemoryBlock){.mem = mem, .size = a_size, .valid = true };
}

static inline void
coy_memory_free(CoyMemoryBlock *mem)
{
    if(mem->valid)
    {
         /*BOOL success =*/ VirtualFree(mem->mem, 0, MEM_RELEASE);
         mem->valid = false;
    }

    return;
}

static inline u32 
coy_thread_func_internal(void *thread_params)
{
    CoyThread *thrd = thread_params;
    thrd->func(thrd->thread_data);

    return 0;
}


static inline b32
coy_thread_create(CoyThread *thrd, CoyThreadFunc func, void *thread_data)
{
    thrd->func = func;
    thrd->thread_data = thread_data;

    DWORD id = 0;
    HANDLE h =  CreateThread(
        NULL,                       // [in, optional]  LPSECURITY_ATTRIBUTES   lpThreadAttributes,
        0,                          // [in]            SIZE_T                  dwStackSize,
        coy_thread_func_internal,   // [in]            LPTHREAD_START_ROUTINE  lpStartAddress,
        thrd,                       // [in, optional]  __drv_aliasesMem LPVOID lpParameter,
        0,                          // [in]            DWORD                   dwCreationFlags,
        &id                         // [out, optional] LPDWORD                 lpThreadId
    );

    if(h == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    _Static_assert(sizeof(h) <= sizeof(thrd->handle), "handle doesn't fit in CoyThread");
    _Static_assert(_Alignof(HANDLE) <= 16, "handle doesn't fit alignment in CoyThread");
    memcpy(&thrd->handle[0], &h, sizeof(h));

    return true;
}

static inline b32
coy_thread_join(CoyThread *thread)
{
    HANDLE *h = (HANDLE *)&thread->handle[0];
    DWORD status = WaitForSingleObject(*h, INFINITE);
    return status == WAIT_OBJECT_0;
}

static inline void 
coy_thread_destroy(CoyThread *thread)
{
    HANDLE *h = (HANDLE *)&thread->handle[0];
    /* BOOL success = */ CloseHandle(*h);
    *thread = (CoyThread){0};
}

static inline CoyMutex 
coy_mutex_create()
{
    CoyMutex mutex = {0};

    _Static_assert(sizeof(CRITICAL_SECTION) <= sizeof(mutex.mutex), "CRITICAL_SECTION doesn't fit in CoyMutex");
    _Static_assert(_Alignof(CRITICAL_SECTION) <= 16, "CRITICAL_SECTION doesn't fit alignment in CoyMutex");

    CRITICAL_SECTION *m = (CRITICAL_SECTION *)&mutex.mutex[0];
    mutex.valid = InitializeCriticalSectionAndSpinCount(m, 0x400) != 0;
    return mutex;
}

static inline b32 
coy_mutex_lock(CoyMutex *mutex)
{
    EnterCriticalSection((CRITICAL_SECTION *)&mutex->mutex[0]);
    return true;
}

static inline b32 
coy_mutex_unlock(CoyMutex *mutex)
{
    LeaveCriticalSection((CRITICAL_SECTION *)&mutex->mutex[0]);
    return true;
}

static inline void 
coy_mutex_destroy(CoyMutex *mutex)
{
    DeleteCriticalSection((CRITICAL_SECTION *)&mutex->mutex[0]);
    mutex->valid = false;
}

static inline CoyCondVar 
coy_condvar_create(void)
{
    CoyCondVar cv = {0};

    _Static_assert(sizeof(CONDITION_VARIABLE) <= sizeof(cv.cond_var), "CONDITION_VARIABLE doesn't fit in CoyCondVar");
    _Static_assert(_Alignof(CONDITION_VARIABLE) <= 16, "CONDITION_VARIABLE doesn't fit alignment in CoyCondVar");

    InitializeConditionVariable((CONDITION_VARIABLE *)&cv.cond_var);
    cv.valid = true;
    return cv;
}

static inline b32 
coy_condvar_sleep(CoyCondVar *cv, CoyMutex *mtx)
{
    return 0 != SleepConditionVariableCS((CONDITION_VARIABLE *)&cv->cond_var, (CRITICAL_SECTION *)&mtx->mutex[0], INFINITE);
}

static inline b32 
coy_condvar_wake(CoyCondVar *cv)
{
    WakeConditionVariable((CONDITION_VARIABLE *)&cv->cond_var);
    return true;
}

static inline b32 
coy_condvar_wake_all(CoyCondVar *cv)
{
    WakeAllConditionVariable((CONDITION_VARIABLE *)&cv->cond_var);
    return true;
}

static inline void 
coy_condvar_destroy(CoyCondVar *cv)
{
    cv->valid = false;
}

static inline u64
coy_profile_read_cpu_timer(void)
{
	return __rdtsc();
}

static inline void 
coy_profile_initialize_os_metrics(void)
{
    if(!coy_global_os_metrics.initialized)
    {
        coy_global_os_metrics.handle = (uptr)OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, GetCurrentProcessId());
        coy_global_os_metrics.initialized = true;
    }
}

static inline void 
coy_profile_finalize_os_metrics(void)
{
    /* no op on win 32 */
}

static inline u64
coy_profile_read_os_page_fault_count(void)
{
    PROCESS_MEMORY_COUNTERS_EX memory_counters = { .cb = sizeof(memory_counters) };
    GetProcessMemoryInfo((HANDLE)coy_global_os_metrics.handle, (PROCESS_MEMORY_COUNTERS *)&memory_counters, sizeof(memory_counters));

    return memory_counters.PageFaultCount;
}

static inline u64 
coy_profile_get_os_timer_freq(void)
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return freq.QuadPart;
}

static inline u64 
coy_profile_read_os_timer(void)
{
    LARGE_INTEGER value;
    QueryPerformanceCounter(&value);
    return value.QuadPart;
}

#endif

#pragma warning(default: 4142)

#elif defined(__linux__)

#ifndef _COYOTE_LINUX_H_
#define _COYOTE_LINUX_H_
/*---------------------------------------------------------------------------------------------------------------------------
 *                                                  Linux Implementation
 *-------------------------------------------------------------------------------------------------------------------------*/
// Linux specific implementation goes here - things NOT in common with Apple / BSD
#include <fcntl.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

/* The reason for a seperate Linux and Apple impelementation is on Linux I can use the MAP_POPULATE flag, but I cannot on 
 * Apple. This flag pre-faults all the pages, so you don't have to worry about page faults slowing down the program.
 */

static inline CoyMemoryBlock 
coy_memory_allocate(size minimum_num_bytes)
{
    Assert(minimum_num_bytes > 0);

    long page_size = sysconf(_SC_PAGESIZE);
    StopIf(page_size == -1, goto ERR_RETURN);
    usize nbytes = minimum_num_bytes + page_size - (minimum_num_bytes % page_size);

    void *ptr = mmap(NULL,                                    // the starting address, NULL = don't care
                     nbytes,                                  // the amount of memory to allocate
                     PROT_READ | PROT_WRITE,                  // we should have read and write access to the memory
                     MAP_PRIVATE | MAP_ANON | MAP_POPULATE,   // not backed by a file, this is pure memory
                     -1,                                      // recommended file descriptor for portability
                     0);                                      // offset into what? this isn't a file, so should be zero

    StopIf(ptr == MAP_FAILED, goto ERR_RETURN);

    return (CoyMemoryBlock){ .mem = ptr, .size = nbytes, .valid = true };

ERR_RETURN:
    return (CoyMemoryBlock){ .mem = NULL, .size = 0, .valid = false};
}

static inline void
coy_memory_free(CoyMemoryBlock *mem)
{
    /* int success = */ munmap(mem->mem, (size_t)mem->size);
    mem->valid = false;
    return;
}

static inline void 
coy_profile_initialize_os_metrics(void)
{
    if(!coy_global_os_metrics.initialized)
    {
        int fd = -1;
        struct perf_event_attr pe = {0};
        pe.type = PERF_TYPE_SOFTWARE;
        pe.size = sizeof(pe);
        pe.config = PERF_COUNT_SW_PAGE_FAULTS;
        pe.disabled = 1;
        pe.exclude_kernel = 0; /* Turns out most page faults happen here */

        fd = syscall(SYS_perf_event_open, &pe, 0, -1, -1, 0);
        if (fd == -1) {
            /* Might need to run with sudo, crash! */
            (*(int volatile*)0);
        }

        ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

        coy_global_os_metrics.handle = (uptr)fd;
        coy_global_os_metrics.initialized = true;
    }
}

static inline void 
coy_profile_finalize_os_metrics(void)
{
    close((int)coy_global_os_metrics.handle);
}

static inline u64
coy_profile_read_os_page_fault_count(void)
{
    u64 count = (u64)-1;
    ioctl((int)coy_global_os_metrics.handle, PERF_EVENT_IOC_DISABLE, 0);
    int size_read = read((int)coy_global_os_metrics.handle, &count, sizeof(count));
    if(size_read != sizeof(count))
    {
        count = (u64)-1; 
    }
    ioctl((int)coy_global_os_metrics.handle, PERF_EVENT_IOC_ENABLE, 0);
    return count;
}

#endif


#elif defined(__APPLE__)

#ifndef _COYOTE_APPLE_OSX_H_
#define _COYOTE_APPLE_OSX_H_
/*---------------------------------------------------------------------------------------------------------------------------
 *                                               Apple/MacOSX Implementation
 *-------------------------------------------------------------------------------------------------------------------------*/
// Apple / BSD specific implementation goes here - things NOT in common with Linux
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/syslimits.h>
#include <unistd.h>

/* The reason for a seperate Linux and Apple impelementation is on Linux I can use the MAP_POPULATE flag, but I cannot on 
 * Apple. This flag pre-faults all the pages, so you don't have to worry about page faults slowing down the program. Apple
 * gives far LESS control over allocation.
 */

static inline CoyMemoryBlock 
coy_memory_allocate(size minimum_num_bytes)
{
    Assert(minimum_num_bytes > 0);

    long page_size = sysconf(_SC_PAGESIZE);
    StopIf(page_size == -1, goto ERR_RETURN);
    usize nbytes = minimum_num_bytes + page_size - (minimum_num_bytes % page_size);

    void *ptr = mmap(NULL,                     // the starting address, NULL = don't care
                     nbytes,                   // the amount of memory to allocate
                     PROT_READ | PROT_WRITE,   // we should have read and write access to the memory
                     MAP_PRIVATE | MAP_ANON,   // not backed by a file, this is pure memory
                     -1,                       // recommended file descriptor for portability
                     0);                       // offset into what? this isn't a file, so should be zero

    StopIf(ptr == MAP_FAILED, goto ERR_RETURN);

    return (CoyMemoryBlock){ .mem = ptr, .size = nbytes, .valid = true };

ERR_RETURN:
    return (CoyMemoryBlock){ .mem = NULL, .size = 0, .valid = false};
}

static inline void
coy_memory_free(CoyMemoryBlock *mem)
{
    /* int success = */ munmap(mem->mem, (size_t)mem->size);
    mem->mem = NULL;
    mem->size = 0;
    mem->valid = false;
    return;
}

static inline void 
coy_profile_initialize_os_metrics(void)
{
    if(!coy_global_os_metrics.initialized)
    {
        coy_global_os_metrics.handle = 0;
        coy_global_os_metrics.initialized = true;
    }
}

static inline void 
coy_profile_finalize_os_metrics(void)
{
    /* no op on mac */
}

static inline u64
coy_profile_read_os_page_fault_count(void)
{
    struct rusage usage = {0};
    getrusage(RUSAGE_SELF, &usage);
    return (u64)(usage.ru_minflt + usage.ru_majflt);
}

#endif


#else
#error "Platform not supported by Coyote Library"
#endif

#if defined(__linux__) || defined(__APPLE__)

#ifndef _COYOTE_LINUX_APPLE_OSX_COMMON_H_
#define _COYOTE_LINUX_APPLE_OSX_COMMON_H_
/*---------------------------------------------------------------------------------------------------------------------------
 *                                         Apple/MacOSX Linux Common Implementation
 *-------------------------------------------------------------------------------------------------------------------------*/

#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <x86intrin.h>
#include <unistd.h>
#include <dlfcn.h>

static inline u64
coy_time_now(void)
{
    struct timeval tv = {0};
    int errcode = gettimeofday(&tv, NULL);

    StopIf(errcode != 0, goto ERR_RETURN);
    StopIf(tv.tv_sec < 0, goto ERR_RETURN);

    return tv.tv_sec;

ERR_RETURN:
    return UINT64_MAX;
}

static char const coy_path_sep = '/';

static inline CoyPathInfo
coy_path_info(char *path, b32 assume_file)
{
    CoyPathInfo info = {0};
    info.full_path = path;

    /* Get the length. */
    size len = 0;
    char *c = path;
    while(*c && len < PATH_MAX)
    {
        ++len;
        ++c;
    }

    /* Check for error condition. */
    if(len == 0 || len >= PATH_MAX)
    {
        return info; /* Return empty info, there's nothing here. */
    }

    /* Find extension, and base.*/
    size idx = len - 1;
    c = &path[idx];
    if(*c == coy_path_sep) /* Skip a trailing slash. */
    {
        --c;
        --idx;
    }
    size next_len = 0;
    while(idx >= 0)
    {
        /* Found the extension. */
        if(*c == '.' && !info.extension.start)
        {
            info.extension = (CoyPathStr){ .start = c + 1, .len = next_len };
        }

        /* Found file name, but check to make sure it's not just a trailing slash. */
        if(*c == coy_path_sep && !info.base.start && (assume_file || info.extension.start) && (idx < len - 1))
        {
            info.base = (CoyPathStr){ .start = c + 1, .len = next_len };
            info.is_file = true; /* Assume this is a file. */
            next_len = -1;
        }

        /* Increment counters. */
        ++next_len;
        --idx;
        --c;
    }

    info.dir.start = path;
    info.dir.len = next_len;

    /* Does it exist? */
    struct stat stat_buf = {0};
    int ret_code = stat(path, &stat_buf);
    if(ret_code != 0)
    {
        info.exists = false;
    }
    else
    {
        info.exists = true;
        info.is_file = stat_buf.st_mode & S_IFDIR ? false : true;
    }

    return info;
}

static inline b32 
coy_path_append(size buf_len, char path_buffer[], char const *new_path)
{
    // Find first '\0'
    size position = 0;
    char *c = path_buffer;
    while(position < buf_len && *c)
    {
      ++c;
      position += 1;
    }

    StopIf(position >= buf_len, goto ERR_RETURN);

    // Add a path separator - unless the buffer is empty or the last path character was a path separator.
    if(position > 0 && path_buffer[position - 1] != coy_path_sep)
    {
      path_buffer[position] = coy_path_sep;
      position += 1;
      StopIf(position >= buf_len, goto ERR_RETURN);
    }

    // Copy in the new path part.
    char const *new_c = new_path;
    while(position < buf_len && *new_c)
    {
        path_buffer[position] = *new_c;
        ++new_c;
        position += 1;
    }

    StopIf(position >= buf_len, goto ERR_RETURN);

    // Null terminate the path.
    path_buffer[position] = '\0';
    
    return true;

ERR_RETURN:
    path_buffer[buf_len - 1] = '\0';
    return false;
}

static inline CoyFileWriter
coy_file_create(char const *filename)
{
    int fd = open( filename,                                        // char const *pathname
                   O_WRONLY | O_CREAT | O_TRUNC,                    // Write only, create if needed, or truncate if needed.
                   S_IRWXU | S_IRGRP | S_IXGRP |S_IROTH | S_IXOTH); // Default permissions 0755

    if (fd >= 0)
    {
        return (CoyFileWriter){ .handle = (iptr) fd, .valid = true  };
    }
    else
    {
        return (CoyFileWriter){ .handle = (iptr) fd, .valid = false };
    }
}

static inline CoyFileWriter
coy_file_append(char const *filename)
{
    int fd = open( filename,                                        // char const *pathname
                   O_WRONLY | O_CREAT | O_APPEND,                   // Write only, create if needed, and append.
                   S_IRWXU | S_IRGRP | S_IXGRP |S_IROTH | S_IXOTH); // Default permissions 0755

    if (fd >= 0) {
        return (CoyFileWriter){ .handle = (iptr) fd, .valid = true  };
    }
    else
    {
        return (CoyFileWriter){ .handle = (iptr) fd, .valid = false };
    }
}

static inline size 
coy_file_writer_flush(CoyFileWriter *file)
{
    StopIf(!file->valid, goto ERR_RETURN);

    _Static_assert(sizeof(ssize_t) == sizeof(size), "oh come on people. ssize_t != intptr_t!? Really!");

    if(file->buf_cursor > 0)
    {
        ssize_t num_bytes_written = write((int)file->handle, file->buffer, file->buf_cursor);
        StopIf(num_bytes_written != file->buf_cursor, goto ERR_RETURN);
        file->buf_cursor = 0;
        return (size) num_bytes_written;
    }

    return 0;

ERR_RETURN:
    return -1;
}

static inline size 
coy_file_write(CoyFileWriter *file, size nbytes_to_write, byte const *buffer)
{
    Assert(nbytes_to_write >= 0);
    size num_bytes_written = 0;

    /* Check if we need to flush the buffer */
    if(nbytes_to_write > (COY_FILE_WRITER_BUF_SIZE - file->buf_cursor))
    {
        num_bytes_written = coy_file_writer_flush(file);
        StopIf(num_bytes_written < 0, goto ERR_RETURN);
    }

    /* For "small" writes, buffer the data. */
    if(nbytes_to_write < COY_FILE_WRITER_BUF_SIZE)
    {
        memcpy(file->buffer + file->buf_cursor, buffer, nbytes_to_write);
        file->buf_cursor += nbytes_to_write;
        return nbytes_to_write;
    }
    else
    {
        /* For large writes, just skip several trips through the buffer. */
        num_bytes_written = write((int)file->handle, buffer, nbytes_to_write);
        StopIf(num_bytes_written < 0, goto ERR_RETURN);
        return (size) num_bytes_written;
    }

ERR_RETURN:
    return -1;
}

static inline void 
coy_file_writer_close(CoyFileWriter *file)
{
    /* TODO: Rework API to return error if the flush fails. */
    coy_file_writer_flush(file);
    /* int err_code = */ close((int)file->handle);
    file->valid = false;
}

static inline size 
coy_file_slurp(char const *filename, size buf_size, byte *buffer)
{
    size file_size = coy_file_size(filename);
    StopIf(file_size < 1 || file_size > buf_size, goto ERR_RETURN);

    int fd = open( filename, // char const *pathname
                   O_RDONLY, // Read only
                   0);       // No mode information needed.

    StopIf(fd < 0, goto ERR_RETURN);
    
    size space_available = buf_size;
    size num_bytes_read = 0;
    size total_num_bytes_read = 0;
    do
    {
        num_bytes_read = read(fd, buffer + total_num_bytes_read, space_available);
        space_available -= num_bytes_read;
        total_num_bytes_read += num_bytes_read;

        StopIf(num_bytes_read < 0, goto ERR_RETURN);

    } while(space_available && num_bytes_read);

    close(fd);
    return (size) total_num_bytes_read;

ERR_RETURN:
    return -1;
}

static inline CoyFileReader 
coy_file_open_read(char const *filename)
{
    int fd = open( filename, // char const *pathname
                   O_RDONLY, // Read only
                   0);       // No mode information needed.

    if (fd >= 0)
    {
        return (CoyFileReader){ .handle = (iptr) fd, .valid = true  };
    }
    else
    {
        return (CoyFileReader){ .handle = (iptr) fd, .valid = false };
    }
}

static inline size 
coy_file_fill_buffer(CoyFileReader *file)
{
    _Static_assert(sizeof(ssize_t) <= sizeof(size), "oh come on people. ssize_t != intptr_t!? Really!");

    if(file->bytes_remaining > 0)
    {
        /* Move remaining data to the front of the buffer */
        memmove(file->buffer, file->buffer + file->buf_cursor, file->bytes_remaining);
    }

    file->buf_cursor = 0;

    size space_available = COY_FILE_READER_BUF_SIZE - file->bytes_remaining;
    size num_bytes_read = 0;
    size total_num_bytes_read = 0;
    while(space_available)
    {
        num_bytes_read = read((int)file->handle, file->buffer + file->bytes_remaining + total_num_bytes_read, space_available);
        space_available -= num_bytes_read;
        total_num_bytes_read += num_bytes_read;

        StopIf(num_bytes_read < 0, goto ERR_RETURN);

        if(num_bytes_read == 0) { break; }
    }

    file->bytes_remaining += total_num_bytes_read;

    return (size) total_num_bytes_read;

ERR_RETURN:
    return -1;
}

static inline size 
coy_file_read(CoyFileReader *file, size buf_size, byte *buffer)
{
    Assert(buf_size > 0);
    StopIf(!file->valid, goto ERR_RETURN);

    if(buf_size > file->bytes_remaining)
    {
        size bytes_read = coy_file_fill_buffer(file);
        StopIf(bytes_read < 0, goto ERR_RETURN);
    }

    size size_to_copy = buf_size > file->bytes_remaining ? file->bytes_remaining : buf_size;
    memcpy(buffer, file->buffer + file->buf_cursor, size_to_copy);
    file->buf_cursor += size_to_copy;
    file->bytes_remaining -= size_to_copy;

    return size_to_copy;

ERR_RETURN:
    return -1;
}

static inline void 
coy_file_reader_close(CoyFileReader *file)
{
    /* int err_code = */ close((int)file->handle);
    file->valid = false;
}

static inline size 
coy_file_size(char const *filename)
{
    _Static_assert(sizeof(off_t) == 8 && sizeof(size) == sizeof(off_t), "Need 64-bit off_t");
    struct stat statbuf = {0};
    int success = stat(filename, &statbuf);
    StopIf(success != 0, return -1);

    return (size)statbuf.st_size;
}

static inline CoyMemMappedFile 
coy_memmap_read_only(char const *filename)
{
    _Static_assert(sizeof(usize) == sizeof(size), "sizeof(size_t) != sizeof(intptr_t)");

    size size_in_bytes = coy_file_size(filename);
    StopIf(size_in_bytes == -1, goto ERR_RETURN);

    int fd = open( filename, // char const *pathname
                   O_RDONLY, // Read only
                   0);       // No mode information needed.
    StopIf(fd < 0, goto ERR_RETURN);

    byte const *data = mmap(NULL,           // Starting address - OS chooses
                            size_in_bytes,  // The size of the mapping,should be the file size for this
                            PROT_READ,      // Read only access
                            MAP_PRIVATE,    // flags - not sure if anything is necessary for read only
                            fd,             // file descriptor for the file to map
                            0);             // offset into the file to read

    close(fd);
    StopIf(data == MAP_FAILED, goto ERR_RETURN);

    return (CoyMemMappedFile){ .size_in_bytes = (size)size_in_bytes, 
                               .data = data, 
                               ._internal = {0}, 
                               .valid = true 
    };

ERR_RETURN:
    return (CoyMemMappedFile) { .valid = false };
}

static inline void 
coy_memmap_close(CoyMemMappedFile *file)
{
    /*BOOL success = */ munmap((void *)file->data, file->size_in_bytes);
    file->valid = false;

    return;
}

static inline CoyFileNameIter 
coy_file_name_iterator_open(char const *directory_path, char const *file_extension)
{
    DIR *d = opendir(directory_path);
    StopIf(!d, goto ERR_RETURN);

    return (CoyFileNameIter){ .os_handle = (iptr)d, .file_extension=file_extension, .valid=true};

ERR_RETURN:
    return (CoyFileNameIter) {.valid=false};
}

static inline char const *
coy_file_name_iterator_next(CoyFileNameIter *cfni)
{
    if(cfni->valid)
    {
        DIR *d = (DIR *)cfni->os_handle;
        struct dirent *entry = readdir(d);
        while(entry)
        {
            if(entry->d_type == DT_REG) {
                if(cfni->file_extension == NULL) 
                {
                    return entry->d_name;
                }
                else 
                {
                    char const *ext = coy_file_extension(entry->d_name);
                    if(coy_null_term_strings_equal(ext, cfni->file_extension))
                    {
                        return entry->d_name;
                    }
                }
            }
            entry = readdir(d);
        }
    }

    cfni->valid = false;
    return NULL;
}

static inline void 
coy_file_name_iterator_close(CoyFileNameIter *cfin)
{
    DIR *d = (DIR *)cfin->os_handle;
    /*int rc = */ closedir(d);
    *cfin = (CoyFileNameIter){0};
    return;
}

static inline CoySharedLibHandle 
coy_shared_lib_load(char const *lib_name)
{
    void *h = dlopen(lib_name, RTLD_LAZY);
    PanicIf(!h);
    return (CoySharedLibHandle) { .handle = h };
}

static inline void 
coy_shared_lib_unload(CoySharedLibHandle handle)
{
    dlclose(handle.handle);
}

static inline void *
coy_share_lib_load_symbol(CoySharedLibHandle handle, char const *symbol_name)
{
    void *s = dlsym(handle.handle, symbol_name);
    PanicIf(!s);
    return s;
}

static inline CoyTerminalSize 
coy_get_terminal_size(void)
{
    struct winsize w = {0};
    int ret_val = ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if(ret_val == 0) 
    { 
        return (CoyTerminalSize){ .columns = w.ws_col, .rows = w.ws_row }; 
    }

    return (CoyTerminalSize){ .columns = -1, .rows = -1 };
}

static inline void *
coy_thread_func_internal(void *thread_params)
{
    CoyThread *thrd = thread_params;
    thrd->func(thrd->thread_data);

    return NULL;
}

static inline b32
coy_thread_create(CoyThread *thrd, CoyThreadFunc func, void *thread_data)
{
    *thrd = (CoyThread){ .func=func, .thread_data=thread_data };

    _Static_assert(sizeof(pthread_t) <= sizeof(thrd->handle), "pthread_t doesn't fit in CoyThread");
    _Static_assert(_Alignof(pthread_t) <= 16, "pthread_t doesn't fit alignment in CoyThread");

    return 0 == pthread_create((pthread_t *)thrd->handle, NULL, coy_thread_func_internal, thrd);
}

static inline b32
coy_thread_join(CoyThread *thread)
{

    pthread_t *t = (pthread_t  *)thread->handle;
    int status = pthread_join(*t, NULL);
    if(status == 0) { return true; }
    return false;
}

static inline void 
coy_thread_destroy(CoyThread *thread)
{
    *thread = (CoyThread){0};
}

static inline CoyMutex 
coy_mutex_create()
{
    CoyMutex mtx = {0};
    pthread_mutex_t mut = {0};

    _Static_assert(sizeof(pthread_mutex_t) <= sizeof(mtx.mutex), "pthread_mutex_t doesn't fit in CoyMutex");
    _Static_assert(_Alignof(pthread_mutex_t) <= 16, "pthread_mutex_t doesn't fit alignment in CoyMutex");

    int status = pthread_mutex_init(&mut, NULL);
    if(status == 0)
    {
        memcpy(&mtx.mutex[0], &mut, sizeof(mut));
        mtx.valid = true;

        return mtx;
    }

    return mtx;
}

static inline b32 
coy_mutex_lock(CoyMutex *mutex)
{
    return pthread_mutex_lock((pthread_mutex_t *)&mutex->mutex[0]) == 0;
}

static inline b32 
coy_mutex_unlock(CoyMutex *mutex)
{
    return pthread_mutex_unlock((pthread_mutex_t *)&mutex->mutex[0]) == 0;
}

static inline void 
coy_mutex_destroy(CoyMutex *mutex)
{
    pthread_mutex_destroy((pthread_mutex_t *)&mutex->mutex[0]);
    mutex->valid = false;
}

static inline CoyCondVar 
coy_condvar_create(void)
{
    CoyCondVar cv = {0};

    _Static_assert(sizeof(pthread_cond_t) <= sizeof(cv.cond_var), "pthread_cond_t doesn't fit in CoyCondVar");
    _Static_assert(_Alignof(pthread_cond_t) <= 16, "pthread_cond_t doesn't fit alignment in CoyCondVar");

    cv.valid = pthread_cond_init((pthread_cond_t *)&cv.cond_var[0], NULL) == 0;
    return cv;
}

static inline b32 
coy_condvar_sleep(CoyCondVar *cv, CoyMutex *mtx)
{
    return 0 == pthread_cond_wait((pthread_cond_t *)&cv->cond_var[0], (pthread_mutex_t *)&mtx->mutex[0]);
}

static inline b32 
coy_condvar_wake(CoyCondVar *cv)
{
    return 0 == pthread_cond_signal((pthread_cond_t *)&cv->cond_var[0]);
}

static inline b32 
coy_condvar_wake_all(CoyCondVar *cv)
{
    return 0 == pthread_cond_broadcast((pthread_cond_t *)&cv->cond_var[0]);
}

static inline void 
coy_condvar_destroy(CoyCondVar *cv)
{
    int status = pthread_cond_destroy((pthread_cond_t *)&cv->cond_var[0]);
    Assert(status == 0);
    cv->valid = false;
}

static inline u64
coy_profile_read_cpu_timer(void)
{
	return __rdtsc();
}

static inline u64 
coy_profile_get_os_timer_freq(void)
{
	return 1000000;
}

static inline u64 
coy_profile_read_os_timer(void)
{
	struct timeval value;
	gettimeofday(&value, 0);
	
	u64 res = coy_profile_get_os_timer_freq() * (u64)value.tv_sec + (u64)value.tv_usec;
	return res;
}

#endif


#endif

#pragma warning(pop)

#endif
