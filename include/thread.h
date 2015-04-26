// EPOS Thread Abstraction Declarations

#ifndef __thread_h
#define __thread_h

#include <utility/queue.h>
#include <utility/handler.h>
#include <cpu.h>
#include <machine.h>
#include <system/kmalloc.h>

__BEGIN_SYS

class Thread
{
    friend class Init_First;
    friend class Synchronizer_Common;
    friend class Alarm;
    friend class IA32;

protected:
    static const bool preemptive = Traits<Thread>::preemptive;
    static const bool reboot = Traits<System>::reboot;

    static const unsigned int QUANTUM = Traits<Thread>::QUANTUM;
    static const unsigned int STACK_SIZE = Traits<Application>::STACK_SIZE;

    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Context Context;

public:
    // Thread State
    enum State {
        RUNNING   = 1,
        READY     = 2,
        SUSPENDED = 3,
        WAITING   = 4,
        FINISHING = 0
    };

    // Thread Priority
    typedef int Priority;
    enum {
        MAIN   = 0,
        HIGH   = 1,
        NORMAL = (unsigned(1) << (sizeof(int) * 8 - 1)) - 4,
        LOW    = (unsigned(1) << (sizeof(int) * 8 - 1)) - 3,
        IDLE   = (unsigned(1) << (sizeof(int) * 8 - 1)) - 2
    };

    // Thread Configuration
    struct Configuration {
        Configuration(const State & s = READY, Priority p = NORMAL, unsigned int ss = STACK_SIZE)
        : state(s), priority(p), stack_size(ss) {}

        State state;
        Priority priority;
        unsigned int stack_size;
    };

    // Thread Queue
    typedef Ordered_Queue<Thread, Priority> Queue;

public:
    template<typename ... Tn>
    Thread(int (* entry)(Tn ...), Tn ... an);
    template<typename ... Cn, typename ... Tn>
    Thread(const Configuration & conf, int (* entry)(Tn ...), Tn ... an);
    ~Thread();

    const volatile State & state() const { return _state; }

    const volatile Priority & priority() const { return _link.rank(); }
    void priority(const Priority & p);

    int join();
    void pass();
    void suspend();
    void resume();

    static Thread * volatile self() { return running(); }
    static void yield();
    static void exit(int status = 0);

protected:
    void constructor(const Log_Addr & entry, unsigned int stack_size);

    static Thread * volatile running() { return _running; }

    static void lock() { CPU::int_disable(); }
    static void unlock() { CPU::int_enable(); }
    static bool locked() { return CPU::int_enabled(); }

    static void sleep(Queue * q);
    static void wakeup(Queue * q);
    static void wakeup_all(Queue * q);

    static void reschedule();
    static void time_slicer(const IC::Interrupt_Id & interrupt);

    static void implicit_exit();

    static void dispatch(Thread * prev, Thread * next) {
        if(prev != next) {
            db<Thread>(TRC) << "Thread::dispatch(prev=" << prev << ",next=" << next << ")" << endl;
            db<Thread>(INF) << "prev={" << prev << ",ctx=" << *prev->_context << "}" << endl;
            db<Thread>(INF) << "next={" << next << ",ctx=" << *next->_context << "}" << endl;

            CPU::switch_context(&prev->_context, next->_context);
        }
    }

    static int idle();

private:
    static void init();

protected:
    char * _stack;
    Context * volatile _context;
    volatile State _state;
    Queue * _waiting;
    Thread * volatile _joining;
    Queue::Element _link;

    static volatile unsigned int _thread_count;
    static Scheduler_Timer * _timer;

private:
    static Thread * volatile _running;
    static Queue _ready;
    static Queue _suspended;
};


template<typename ... Tn>
inline Thread::Thread(int (* entry)(Tn ...), Tn ... an)
: _state(READY), _waiting(0), _joining(0), _link(this, NORMAL)
{
    lock();
    _stack = reinterpret_cast<char *>(kmalloc(STACK_SIZE));
    _context = CPU::init_stack(_stack, STACK_SIZE, &implicit_exit, entry, an ...);
    constructor(entry, STACK_SIZE); // implicit unlock
}

template<typename ... Cn, typename ... Tn>
inline Thread::Thread(const Configuration & conf, int (* entry)(Tn ...), Tn ... an)
: _state(conf.state), _waiting(0), _joining(0), _link(this, conf.priority)
{
    lock();
    _stack = reinterpret_cast<char *>(kmalloc(conf.stack_size));
    _context = CPU::init_stack(_stack, conf.stack_size, &implicit_exit, entry, an ...);
    constructor(entry, conf.stack_size); // implicit unlock
}


// An event handler that triggers a thread (see handler.h)
class Thread_Handler : public Handler
{
public:
    Thread_Handler(Thread * h) : _handler(h) {}
    ~Thread_Handler() { operator()(); }

    void operator()() {
      if(_handler)
        _handler->resume();
    }

private:
    Thread * _handler;
};


__END_SYS

#endif
