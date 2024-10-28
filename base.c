#define ADD_FLAG(flags, flagToAdd)   ((flags) |= (flagToAdd))
#define REMOVE_FLAG(flags, flagToRemove) ((flags) &= ~(flagToRemove))
#define IS_FLAG_SET(flags, flagToCheck) ((flags) & (flagToCheck))
#define TOGGLE_FLAG(flags, flagToToggle) ((flags) ^= (flagToToggle))

fn b32 is_pow_of_2(size_t addr)
{
	return (addr & (addr-1)) == 0;
}

typedef struct Arena Arena;
struct Arena
{
	Arena *next;
	u64 used;
	u64 align;
	u64 cmt;
	u64 res;
};

#define ARENA_COMMIT_SIZE KB(64)
#define ARENA_RESERVE_SIZE MB(64)
#define ARENA_HEADER_SIZE 128
#define ARENA_ARR_LEN(arena, type) (arena->used / sizeof(type))

#define AlignPow2(x,b) (((x) + (b) - 1)&(~((b) - 1)))
#define Min(A,B) (((A)<(B))?(A):(B))
#define Max(A,B) (((A)>(B))?(A):(B))
#define ClampTop(A,X) Min(A,X)
#define ClampBot(X,B) Max(X,B)

typedef struct ArenaTemp ArenaTemp;
struct ArenaTemp
{
	Arena *arena;
	u64 pos;
};

#define push_struct(arena, type) (type*)_arenaPushImpl(arena, sizeof(type))
#define push_array(arena,type,count) (type*)_arenaPushImpl(arena, sizeof(type) * count)

fn void* _arenaPushImpl(Arena* arena, size_t size)
{
	u64 pos_mem = AlignPow2(arena->used, arena->align);
	u64 pos_new = pos_mem + size;
	
	if(arena->res < pos_new)
	{
		// TODO(mizu): deal with reserving more (chain arenas)
		INVALID_CODE_PATH();
	}
	
	if(arena->cmt < pos_new)
	{
		u64 cmt_new_aligned, cmt_new_clamped, cmt_new_size;
		
		cmt_new_aligned = AlignPow2(pos_new, ARENA_COMMIT_SIZE);
		cmt_new_clamped = ClampTop(cmt_new_aligned, arena->res);
		cmt_new_size    = cmt_new_clamped - arena->cmt;
		os_commit((u8*)arena + arena->cmt, cmt_new_size);
		arena->cmt = cmt_new_clamped;
	}
	
	void *memory = 0;
	
	if (arena->cmt >= pos_new) 
	{
		memory = (u8*)arena + pos_mem;
		arena->used = pos_new;
	}
	
	return memory;
}

fn ArenaTemp arenaTempBegin(Arena *arena)
{
	ArenaTemp out = {
		.arena = arena,
		.pos = arena->used,
	};
	return out;
}

// TODO(mizu): Can be very expensive
fn void arenaTempEnd(ArenaTemp *temp)
{
	memset((u8*)temp->arena + temp->pos, 0, temp->arena->used - temp->pos);
	
	temp->arena->used = temp->pos;
}

fn Arena *arenaAllocSized(u64 cmt, u64 res)
{
	Arena *arena = 0;
	
	void *memory = os_reserve(res);
	os_commit(memory, cmt);
	
	arena = (Arena*)memory;
	arena->used = ARENA_HEADER_SIZE;
	arena->align = DEFAULT_ALIGN;
	
	arena->cmt = AlignPow2(cmt, os_getPageSize());
	arena->res = res;
	
	return arena;
}

fn Arena *arenaAlloc()
{
	return arenaAllocSized(ARENA_COMMIT_SIZE, ARENA_RESERVE_SIZE);
}

#define str8_varg(S) (int)((S).len), ((S).c)

typedef struct Str8 Str8;
struct Str8
{
	u8 *c;
	u64 len;
};

#define str8_lit(c) Str8{(u8*)c, sizeof(c) - 1}

fn u64 cstr8Len(char *c)
{
	u64 out = 0;
	while(*c++)
	{
		out++;
	}
	return out;
}

fn Str8 str8(u8 *c, u64 len)
{
	Str8 out = 
	{
		c,len
	};
	return out;
}

fn void str8_cpy(Str8 *dst, Str8 *src)
{
	for(u32 i = 0; i < src->len; i ++)
	{
		dst->c[i] = src->c[i];
	}
}

fn Str8 push_str8fv(Arena *arena, char *fmt, va_list args)
{
	Str8 out = {0};
	va_list args_copy;
	va_copy(args_copy, args);
	
	int bytes_req = stbsp_vsnprintf(0, 0, fmt, args) + 1;
	
	out.c = push_array(arena, u8, bytes_req);
	
	out.len = stbsp_vsnprintf((char *)out.c, bytes_req, fmt, args_copy);
	va_end(args_copy);
	
	return out;
}

fn Str8 push_str8f(Arena *arena, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	Str8 result = push_str8fv(arena, fmt, args);
	va_end(args);
	return(result);
}

fn b32 str8_equals(Str8 a, Str8 b)
{
	b32 res = 1;
	
	if(a.len == b.len)
	{
		for(u32 i = 0; i < a.len; i++)
		{
			if(a.c[i] != b.c[i])
			{
				res = 0;
				break;
			}
		}
	}
	else
	{
		res = 0;
	}
	
	return res;
}

fn Str8 str8_join(Arena *arena, Str8 a, Str8 b)
{
	Str8 out = {0};
	out.c = push_array(arena, u8, a.len + b.len);
	
	memcpy(out.c, a.c, a.len);
	
	memcpy((u8*)out.c + a.len, b.c, b.len);
	//printf("%s %lu\r\n", b.c, b.len);
	
	out.len = a.len + b.len;
	return out;
}

typedef enum DEBUG_CYCLE_COUNTER DEBUG_CYCLE_COUNTER;
enum DEBUG_CYCLE_COUNTER
{
	DEBUG_CYCLE_COUNTER_UPDATE_AND_RENDER,
	DEBUG_CYCLE_COUNTER_PATHFINDING,
	DEBUG_CYCLE_COUNTER_PF_GET_NEIGHBORS,
	DEBUG_CYCLE_COUNTER_PF_CONTAINS_NODE,
	DEBUG_CYCLE_COUNTER_PF_OPEN_CONTAINS_NODE,
	DEBUG_CYCLE_COUNTER_PF_CLOSED_CONTAINS_NODE,
	DEBUG_CYCLE_COUNTER_PF_LOWEST_FCOST,
	DEBUG_CYCLE_COUNTER_PF_PREPARE_PATH,
	DEBUG_CYCLE_COUNTER_PF_REVERSE_PATH,
	DEBUG_CYCLE_COUNTER_EDITOR,
	DEBUG_CYCLE_COUNTER_ED_LISTER,
	DEBUG_CYCLE_COUNTER_ED_PROFILER,
	DEBUG_CYCLE_COUNTER_ED_CONSOLE,
	DEBUG_CYCLE_COUNTER_ED_UPDATE,
	DEBUG_CYCLE_COUNTER_ED_SUBMIT,
	DEBUG_CYCLE_COUNTER_COUNT
};

read_only char *debug_cycle_to_str[DEBUG_CYCLE_COUNTER_COUNT] = 
{
	"update & render",
	"pathfinding",
	"get neighbours",
	"contains node",
	"open set",
	"closed set",
	"lowest cost",
	"prepare path",
	"reverse path",
	"editor",
	"lister",
	"profiler",
	"console",
	"update",
	"submit"
};

typedef struct debug_cycle_counter debug_cycle_counter;
struct debug_cycle_counter
{
	u64 cycle_count;
	u32 hit_count;
};

typedef struct TCXT TCXT;
struct TCXT
{
	Arena *arena;
	Arena *arenas[2];
	debug_cycle_counter counters[DEBUG_CYCLE_COUNTER_COUNT];
	debug_cycle_counter counters_last[DEBUG_CYCLE_COUNTER_COUNT];
};

pub TCXT *tcxt;

#if defined (OS_WIN32) || defined(OS_LINUX)
#define BEGIN_TIMED_BLOCK(ID) u64 start_cycle_count_##ID = __rdtsc(); ++tcxt->counters[DEBUG_CYCLE_COUNTER_##ID].hit_count
#define END_TIMED_BLOCK(ID)  tcxt->counters[DEBUG_CYCLE_COUNTER_##ID].cycle_count += __rdtsc() - start_cycle_count_##ID

#else
#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)
#endif

fn void tcxt_init()
{
	Arena *arena = arenaAlloc();
	tcxt = push_struct(arena, TCXT);
	tcxt->arena = arena;
	for(u32 i = 0; i < ARRAY_LEN(tcxt->arenas); i ++)
	{
		tcxt->arenas[i] = arenaAllocSized(MB(10), MB(64));
	}
}

fn void tcxt_process_debug_counters()
{
	for(u32 i = 0; i < ARRAY_LEN(tcxt->counters); i ++)
	{
		debug_cycle_counter *counter = tcxt->counters + i;
		debug_cycle_counter *counter_last = tcxt->counters_last + i;
		
		counter_last->hit_count = counter->hit_count;
		counter_last->cycle_count = counter->cycle_count;
		
		//printf("%d: %lu\n", i, counter->cycle_count);
		counter->hit_count = 0;
		counter->cycle_count = 0;
	}
}

fn void tcxt_print_debug_counters()
{
	for(u32 i = 0; i < ARRAY_LEN(tcxt->counters); i ++)
	{
		debug_cycle_counter *counter = tcxt->counters_last + i;
		
		printf("%s: %llu\n", debug_cycle_to_str[i], counter->cycle_count);
	}
}

fn Arena *tcxt_get_scratch(Arena **conflicts, u64 count)
{
	Arena *out = 0;
	for(u32 i = 0; i < ARRAY_LEN(tcxt->arenas); i ++)
	{
		b32 has_conflict = 0;
		for(u32 j = 0; j < count; j ++)
		{
			if(tcxt->arenas[i] == conflicts[j])
			{
				has_conflict = 1;
				break;
			}
		}
		if(!has_conflict)
		{
			out = tcxt->arenas[i];
		}
	}
	
	return out;
}

#define scratch_begin(conflicts, count) arenaTempBegin(tcxt_get_scratch(conflicts, count))
#define scratch_end(scratch) arenaTempEnd(scratch);

typedef struct Bitmap Bitmap;
struct Bitmap
{
	void *data;
	s32 w;
	s32 h;
	s32 n;
};

typedef enum FILE_TYPE FILE_TYPE;
enum FILE_TYPE
{
	FILE_TYPE_TEXT,
	FILE_TYPE_BINARY,
	FILE_TYPE_COUNT
};

typedef struct FileData FileData;
struct FileData
{
	u8 *bytes;
	u64 size;
};

#if defined(OS_WIN32)
#define _file_open(file, filepath, mode) fopen_s(file, filepath, mode)
#define _sleep(len) Sleep(len)
#elif defined (OS_LINUX) || defined (OS_APPLE)
#define _file_open(file, filepath, mode) *file = fopen(filepath, mode)
#define _sleep(len) sleep(len)
#endif

fn FileData read_file(Arena *arena, const char *filepath, FILE_TYPE type)
{
	FileData out = {0};
	FILE *file;
	
	read_only char *file_type_table[FILE_TYPE_COUNT] = 
	{
		"r",
		"rb"
	};
	
	_file_open(&file, filepath, file_type_table[type]);
	
	fseek(file, 0, SEEK_END);
	
	out.size = ftell(file);
	//print("%d", len);
	
	fseek(file, 0, SEEK_SET);
	
	out.bytes = push_array(arena, u8, out.size);
	fread(out.bytes, sizeof(u8), out.size, file);
	
	fclose(file);
	
	return out;
}

fn void write_file(const char *filepath, FILE_TYPE type, void *data, size_t size)
{
	FILE *file;
	
	read_only char *file_type_table[FILE_TYPE_COUNT] = 
	{
		"w",
		"wb"
	};
	
	_file_open(&file, filepath, file_type_table[type]);
	
	fwrite(data, size, 1, file);
	
	fclose(file);
	
}

fn b32 clone_file(const char* sourcePath, const char* destinationPath)
{
	b32 out = 0;
	
	FILE* sourceFile, * destinationFile;
	char buffer[4096];
	size_t bytesRead;
	
	_file_open(&sourceFile, sourcePath, "rb");
	
	if(sourceFile)
	{
		_file_open(&destinationFile, destinationPath, "wb");
		
		if(destinationFile)
		{
			while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0) 
			{
				fwrite(buffer, 1, bytesRead, destinationFile);
			}
			
			fclose(sourceFile);
			fclose(destinationFile);
			
			out = 1;
		}
	}
	
	return out;
}

fn Bitmap bitmap(Str8 path)
{
	Bitmap out = {0};
	
	stbi_set_flip_vertically_on_load(1);
	
	out.data = stbi_load((char*)path.c, &out.w, &out.h, &out.n, 0);
	
	return out;
}

fn Str8 file_name_from_path(Arena *arena, Str8 path)
{
	char *cur = (char*)&path.c[path.len - 1];
	u32 count = 0;
	
	//NOTE(mizu): pig
	while(*cur != '/' && *cur != '\\' && *cur != '\0')
	{
		cur--;
		count++;
	}
	
	Str8 file_name_cstr = {0};
	file_name_cstr.c = push_array(arena, u8, count + 1);
	file_name_cstr.len = count + 1;
	memcpy(file_name_cstr.c, cur + 1, count);
	file_name_cstr.c[count] = '\0';
	
	return file_name_cstr;
}