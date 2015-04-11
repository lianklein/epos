// EPOS Thread Abstraction Initialization

#include <system/kmalloc.h>
#include <system.h>
#include <thread.h>
#include <alarm.h>

extern "C" { void __epos_app_entry(); }

__BEGIN_SYS

void Thread::init()
{
	unsigned int IDLE_PRIORITY = (unsigned int)(-1);

    int (* entry)() = reinterpret_cast<int (*)()>(__epos_app_entry);

    db<Init, Thread>(TRC) << "Thread::init(entry=" << reinterpret_cast<void *>(entry) << ")" << endl;

    _running = new (kmalloc(sizeof(Thread))) Thread(Configuration(RUNNING, NORMAL), entry);

    new (kmalloc(sizeof(Thread))) Thread(Configuration(READY, IDLE_PRIORITY), &idle);

    if(preemptive)
        _timer = new (kmalloc(sizeof(Scheduler_Timer))) Scheduler_Timer(QUANTUM, time_slicer);

    db<Init, Thread>(INF) << "Dispatching the first thread: " << _running << endl;

    This_Thread::not_booting();

    _running->_context->load();

}

__END_SYS
