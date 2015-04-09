// EPOS Application-level Dynamic Memory Utility Declarations

#ifndef __malloc_h
#define __malloc_h

#include <utility/string.h>
#include <application.h>

extern "C"
{
    using namespace EPOS;

    // Standard C Library allocators
    inline void * malloc(size_t bytes) {
	return Application::_heap->alloc(bytes);
    }

    inline void * calloc(size_t n, unsigned int bytes) {
        void * ptr = malloc(n * bytes);
        memset(ptr, 0, n * bytes);
        return ptr;
    }

    inline void free(void * ptr) {
        Application::_heap->free(ptr);
    }
}

// C++ dynamic memory allocators and deallocators
inline void * operator new(size_t bytes) {
    return malloc(bytes);
}

inline void * operator new[](size_t bytes) {
    return malloc(bytes);
}

// Delete cannot be declared inline due to virtual destructors
void operator delete(void * ptr);
void operator delete[](void * ptr);

#endif
