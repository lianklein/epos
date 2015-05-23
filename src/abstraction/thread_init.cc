// EPOS Thread Abstraction Initialization

#include <system.h>
#include <thread.h>
#include <alarm.h>

extern "C" { void __epos_app_entry(); }

__BEGIN_SYS

void Thread::init()
{
    int (* entry)();
    if(Traits<Build>::MODE == Traits<Build>::LIBRARY)
        entry = reinterpret_cast<int (*)()>(__epos_app_entry);
    else
        entry = reinterpret_cast<int (*)()>(System::info()->lm.app_entry);

    db<Init, Thread>(TRC) << "Thread::init(entry=" << reinterpret_cast<void *>(entry) << ")" << endl;

    // The installation of the scheduler timer handler must precede the
    // creation of threads, since the constructor can induce a reschedule
    // and this in turn can call timer->reset()
    // Letting reschedule() happen during thread creation is harmless, since
    // MAIN is created first and dispatch won't replace it nor by itself
    // neither by IDLE (that has a lower priority)
    if(Criterion::timed && (Machine::cpu_id() == 0))
        _timer = new (SYSTEM) Scheduler_Timer(QUANTUM, time_slicer);

    Thread * first;
    // Create the application's main thread
    // This must precede idle, thus avoiding implicit rescheduling
    // For preemptive scheduling, reschedule() is called, but it will preserve MAIN as the RUNNING thread
    first = new (SYSTEM) Thread(Task::_master, Configuration(RUNNING, MAIN), entry);
    new (SYSTEM) Thread(Task::_master, Configuration(READY, IDLE), &idle);

    db<Init, Thread>(INF) << "Dispatching the first thread: " << first << endl;

    This_Thread::not_booting();

    first->_context->load();
}

__END_SYS
