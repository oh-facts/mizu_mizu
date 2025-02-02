#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <dlfcn.h>

fn void *os_reserve(u64 size)
{
	void *out = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	total_res += size;
	return out;
}

fn b32 os_commit(void *ptr, u64 size)
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

fn void os_decommit(void *ptr, u64 size)
{
	madvise(ptr, size, MADV_DONTNEED);
	mprotect(ptr, size, PROT_NONE);
}

fn void os_free(void *ptr, u64 size)
{
	munmap(ptr, size);
}

fn u64 os_getPageSize()
{
	return getpagesize();
}

fn void os_sleep(s32 ms)
{
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, 0);
}

// NOTE(mizu): idk if this will work on mac. If not, move this to linux and write mac stuff
fn Str8 os_getAppDir(Arena *arena)
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

fn u64 os_getPerfCounter() 
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

fn u64 os_getPerfFreq() 
{
	return 1000000000ull;
}

fn OS_Handle os_loadLibrary(char *name)
{
	OS_Handle out = {0};
	void *dll = dlopen(name, RTLD_NOW);
	printf("%s %p\r\n", name, dll);
	out.u64[0] = (uint64_t)dll;
	return out;
}

void *os_loadFunction(OS_Handle handle, char *name)
{
	void *dll = (void *)handle.u64[0];
	void *out = dlsym(dll, name);
	return out;
}