
inline
void random_init(u32 seed)
{
	srand(seed);
}

inline
i32 random_between(i32 min, i32 max)
{
	i32 delta = max - min;
	i32 result = rand() % delta + min;
	return result;
}
