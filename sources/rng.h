
inline
void random_init(unsigned int seed)
{
	srand(seed);
}

inline
int random_between(int min, int max)
{
	int delta = max - min;
	int result = rand() % delta + min;
	return result;
}
