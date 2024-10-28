typedef union v2f v2f;
union v2f
{
	f32 e[2];
	struct
	{
		f32 x;
		f32 y;
	};
};

typedef union v2s v2s;
union v2s
{
	s32 e[2];
	struct
	{
		s32 x;
		s32 y;
	};
};

typedef struct Rect Rect;
struct Rect
{
	v2f min;
	v2f max;
};

function Rect rect(f32 min_x, f32 min_y, f32 max_x, f32 max_y)
{
	Rect out = {0};
	
	out.min.x = min_x;
	out.min.y = min_y;
	
	out.max.x = max_x;
	out.max.y = max_y;
	
	return out;
}

#define rect_min_varg(v) (v).min.x, (v).min.y
#define rect_max_varg(v) (v).max.x, (v).max.y
#define rect_varg(v) rect_min_varg(v), rect_max_varg(v)

#define v4f_varg(v) (v).x, (v).y, (v).z, (v).w

function Rect rectFromDim(v2f pos, v2f scale)
{
	Rect out = {0};
	out.min.x = pos.x;
	out.min.y = pos.y;
	
	out.max.x = out.min.x + scale.x;
	out.max.y = out.min.y + scale.y;
	
	return out;
}

function v2f sizeFromRect(Rect rect)
{
	v2f out = {0};
	out.x = rect.max.x - rect.min.x;
	out.y = rect.max.y - rect.min.y;
	
	return out;
}

function v2f centerFromRect(Rect rect)
{
	v2f out = {0};
	
	out.x = (rect.max.x + rect.min.x) / 2.f;
	out.y = (rect.max.y + rect.min.y) / 2.f;
	
	return out;
}