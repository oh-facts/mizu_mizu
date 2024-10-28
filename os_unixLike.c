function void *os_reserve(u64 size)
{
	void *out = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	total_res += size;
	return out;
}

function b32 os_commit(void *ptr, u64 size)
{
	if(mprotect(ptr, size, PROT_READ | PROT_WRITE) == -1)
	{
		int err = errno;
		printf("mprotect failed: %s\r\n", strerror(err));
		return 0;
	}
	total_cmt += size;
	return 1;
}

function void os_decommit(void *ptr, u64 size)
{
	madvise(ptr, size, MADV_DONTNEED);
	mprotect(ptr, size, PROT_NONE);
}

function void os_release(void *ptr, u64 size)
{
	munmap(ptr, size);
}

function u64 os_getPageSize()
{
	return getpagesize();
}

// NOTE(mizu): idk if this will work on mac. If not, move this to linux and write mac stuff
function Str8 os_getAppDir(Arena *arena)
{
	char buffer[256];
	ssize_t len = readlink("/proc/self/exe", buffer, 256);
	
	char *c = &buffer[len];
	while(*(--c) != '/')
	{
		*c = 0;
		--len;
	}
	
	u8 *str = push_array(arena, u8, len);
	memcpy(str, buffer, len);
	
	Str8 out = str8(str, len);
	
	return out;
}

function u64 os_getPerfCounter()
{
	NOT_IMPLEMENTED();
	return 0;
}

function u64 os_getPerfFreq()
{
	NOT_IMPLEMENTED();
	return 0;
}
