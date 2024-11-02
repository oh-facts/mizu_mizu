pub u64 total_cmt;
pub u64 total_res;
typedef struct OS_Handle OS_Handle;
struct OS_Handle
{
	u64 u64[1];
};

// memory
fn void *os_reserve(u64 size);
fn b32 os_commit(void *ptr, u64 size);
fn void os_decommit(void *ptr, u64 size);
fn void os_free(void *ptr, u64 size);
fn u64 os_getPageSize();
fn void os_sleep(s32 ms);

// proc
struct Arena;
fn struct Str8 os_getAppDir(struct Arena *arena);
fn u64 os_getPerfCounter();
fn u64 os_getPerfFreq();

// dll
fn OS_Handle os_loadLibrary(char *name);
fn void *os_loadFunction(OS_Handle handle, char *name);