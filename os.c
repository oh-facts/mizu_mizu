function void *os_reserve(u64 size);
function b32 os_commit(void *ptr, u64 size);
function void os_decommit(void *ptr, u64 size);
function void os_release(void *ptr, u64 size);
function u64 os_getPageSize();

struct Arena;
function struct Str8 os_getAppDir(struct Arena *arena);
function u64 os_getPerfCounter();
function u64 os_getPerfFreq();