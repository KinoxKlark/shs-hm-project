#define internal static
#define local_persist static
#define global_variable static

#ifdef DEBUG

template<bool test> class CompileAssertClass;
template<> class CompileAssertClass<true>{ };
#define CompileAssert(value) (CompileAssertClass<value>())

//#define Assert(value) { if(!(value)) { *(int*)0 = 0; } }

#define InvalidCodePath assert(("Invalid code path",false))
#define InvalidDefaultCase default: assert(("Invalid default case", false))
#define NotImplemented assert(("Not implemented", false))

#else

#define CompileAssert(ignore) ((void)0)
#define Assert(ignore) ((void)0)
#define InvalidCodePath ((void)0)
#define InvalidDefaultCase default: {}
#define NotImplemented ((void)0)

#endif

#define ArraySize(array) (sizeof(array) / sizeof( array[0] ))

typedef float r32;
typedef double r64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uintptr_t umm;

#define MAX_U8  255
#define MAX_U16 65535
#define MAX_U32 4294967295
#define MAX_U64 18446744073709551615

#define MAX_i8 127
#define MIN_i8 -128

#define MAX_i16 32767
#define MIN_i16 -32768

#define MAX_i32 2147483647
#define MIN_i32 -2147483648

#define MAX_i64 9223372036854775807
#define MIN_i64 -9223372036854775808

#ifdef DEBUG
inline void checkTypes()
{
	// Check byte size
	
	CompileAssert(sizeof(r32) == 32/8);
	CompileAssert(sizeof(r64) == 64/8);
	
	CompileAssert(sizeof(i8) == 8/8);
	CompileAssert(sizeof(i16) == 16/8);
	CompileAssert(sizeof(i32) == 32/8);
	CompileAssert(sizeof(i64) == 64/8);
	
	CompileAssert(sizeof(u8) == 8/8);
	CompileAssert(sizeof(u16) == 16/8);
	CompileAssert(sizeof(u32) == 32/8);
	CompileAssert(sizeof(u64) == 64/8);
	
	CompileAssert(sizeof(bool) == 8/8);

	// Check overflow
	
	CompileAssert((u8)(MAX_U8+1) == 0);
	CompileAssert((u16)(MAX_U16+1) == 0);
	CompileAssert((u32)(MAX_U32+1) == 0);
	CompileAssert((u64)(MAX_U64+1) == 0);
}

#endif
