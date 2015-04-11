// EPOS Global System Abstraction Declarations

#ifndef __system_h
#define __system_h

#include <utility/heap.h>

extern "C"
{
    void * malloc(size_t);
    void free(void *);
}

__BEGIN_SYS

class System
{
    friend class Init_System;
    friend class Init_Application;
    friend void * kmalloc(size_t);
    friend void kfree(void *);

public:
    static System_Info<Machine> * const info() { return _si; }

private:
    static void init();

private:
    static System_Info<Machine> * _si;
    static char _preheap[sizeof(Heap)];
    static Heap * _heap;
};

__END_SYS

#endif
