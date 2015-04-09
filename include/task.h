// EPOS Task Abstraction Declarations

#include <thread.h>

#ifndef __task_h
#define __task_h

#include <utility/malloc.h>
#include <address_space.h>
#include <segment.h>

__BEGIN_SYS

class Task
{
    friend class System;
    friend class Thread;

private:
    static const bool multitask = Traits<System>::multitask;
    
    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Context Context;
    typedef class Queue<Thread> Queue;

protected:
    Task(Address_Space * as, const Segment * cs, const Segment * ds, Log_Addr code, Log_Addr data)
    : _as(as), _cs(cs), _ds(ds), _code(code), _data(data) {}

public:
    Task(const Segment & cs, const Segment & ds);
    ~Task();

    Address_Space * address_space() const { return _as; }

    const Segment * code_segment() const { return _cs; }
    const Segment * data_segment() const { return _ds; }

    Log_Addr code() const { return _code; }
    Log_Addr data() const { return _data; }

    static const Task * self() { assert(_master || !Traits<Task>::enabled); return multitask ? Thread::self()->task() : _master; }

private:
    void activate() const { _as->activate(); }

    static void init();

private:
    Address_Space * _as;
    const Segment * _cs;
    const Segment * _ds;
    Log_Addr _code;
    Log_Addr _data;

    Queue _threads;

    static Task * _master;
};

__END_SYS

#endif
