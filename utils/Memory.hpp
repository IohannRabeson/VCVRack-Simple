#if!defined MEMORY_HPP
#define MEMORY_HPP
#include <cstdlib>

template <class T>
struct FreeDeleter
{
	void operator()(T* ptr)const
	{
		std::free(ptr);
	}
};

#endif
