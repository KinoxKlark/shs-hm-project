
inline
void random_init(u32 seed)
{
	std::srand(seed);
}

inline
u32 get_random_number()
{
	u32 result = std::rand();
	return result;
}

inline
i32 get_random_number_between(i32 min, i32 max)
{
	assert(min <= max);
	i32 result = get_random_number() % (max+1-min) + min;
	return result;
}

inline
r32 get_random_number_between(r32 min, r32 max)
{
	assert(min < max);
	r32 result = (r32)(get_random_number())/RAND_MAX;
	result = min + (max-min)*result;
	return result;
}

template<typename T>
inline T get_random_element(std::vector<T>& vec)
{
	u32 idx = get_random_number_between(0, vec.size()-1);
	return vec[idx];
}
