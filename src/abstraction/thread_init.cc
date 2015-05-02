// EPOS Thread Abstraction Initialization

#include <system.h>
#include <thread.h>
#include <alarm.h>

extern "C" { void __epos_app_entry(); }

__BEGIN_SYS

void Thread::init()
{
    int (* entry)() = reinterpret_cast<int (*)()>(__epos_app_entry);

    db<Init, Thread>(TRC) << "Thread::init(entry=" << reinterpret_cast<void *>(entry) << ")" << endl;

    // Create the application's main thread
    // This must precede idle, thus avoiding implicit rescheduling
    // For preemptive scheduling, reschedule() is called, but it will preserve MAIN as the RUNNING thread
    _running = new (SYSTEM) Thread(Configuration(RUNNING, MAIN), entry);
    new (SYSTEM) Thread(Configuration(READY, IDLE), &idle);

    if(preemptive)
        _timer = new (SYSTEM) Scheduler_Timer(QUANTUM, time_slicer);

    db<Init, Thread>(INF) << "Dispatching the first thread: " << _running << endl;

    This_Thread::not_booting();

    _running->_context->load();
}

__END_SYS
