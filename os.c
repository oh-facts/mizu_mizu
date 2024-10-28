fn void *os_reserve(u64 size);
fn b32 os_commit(void *ptr, u64 size);
fn void os_decommit(void *ptr, u64 size);
fn void os_free(void *ptr, u64 size);
fn u64 os_getPageSize();
fn void os_sleep(s32 ms);

struct Arena;
fn struct Str8 os_getAppDir(struct Arena *arena);
fn u64 os_getPerfCounter();
fn u64 os_getPerfFreq();