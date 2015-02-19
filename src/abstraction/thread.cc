// EPOS Thread Abstraction Implementation

#include <machine.h>
#include <thread.h>
#include <alarm.h>

__BEGIN_SYS

// Class attributes
volatile unsigned int Thread::_thread_count;
Scheduler_Timer * Thread::_timer;
Scheduler<Thread> Thread::_scheduler;
Spin Thread::_lock;


// This_Thread class attributes
bool This_Thread::_not_booting;


// Methods
void Thread::common_constructor(Log_Addr entry, unsigned int stack_size) 
{
    db<Thread>(TRC) << "Thread(entry=" << entry
                    << ",state=" << _state
                    << ",priority=" << _link.rank()
                    << ",stack={b=" << reinterpret_cast<void *>(_stack)
                    << ",s=" << stack_size
                    << "},context={b=" << _context
                    << "," << *_context << "}) => " << this << "@" << _link.rank().queue() << endl;

    _thread_count++;

    _scheduler.insert(this);
    if((_state != READY) && (_state != RUNNING))
        _scheduler.suspend(this);

    if(preemptive && (_state == READY) && (_link.rank() != IDLE))
        reschedule(_link.rank().queue());
    else
        unlock();
}


Thread::~Thread()
{
    lock();

    db<Thread>(TRC) << "~Thread(this=" << this
                    << ",state=" << _state
                    << ",priority=" << _link.rank()
                    << ",stack={b=" << reinterpret_cast<void *>(_stack)
                    << ",context={b=" << _context
                    << "," << *_context << "})" << endl;

    // The running thread cannot delete itself!
    assert(_state != RUNNING);
    
    switch(_state) {
    case RUNNING:  // For switch completion only: the running thread would have deleted itself! Stack wouldn't have been released!
        exit(-1);
        break;
    case READY:
        _scheduler.remove(this);
        _thread_count--;
        break;
    case SUSPENDED:
        _scheduler.resume(this);
        _scheduler.remove(this);
        _thread_count--;
        break;
    case WAITING:
        _waiting->remove(this);
        _scheduler.resume(this);
        _scheduler.remove(this);
        _thread_count--;
        break;
    case FINISHING: // Already called exit()
        break;
    }

    if(_joining)
        _joining->resume();

    unlock();

    delete _stack;
}


void Thread::priority(const Priority & c)
{
    lock();

    db<Thread>(TRC) << "Thread::priority(this=" << this << ",prio=" << c << ")" << endl;

    unsigned int old_cpu = _link.rank().queue();

    _link.rank(Criterion(c));

    if(_state != RUNNING) {
        _scheduler.remove(this);
        _scheduler.insert(this);
    }

    if(preemptive) {
        reschedule(old_cpu);
        if(smp) {
            lock();
            reschedule(_link.rank().queue());
        }
    }
}


int Thread::join()
{
    lock();

    db<Thread>(TRC) << "Thread::join(this=" << this << ",state=" << _state << ")" << endl;

    // Precondition: no Thread::self()->join()
    assert(running() != this);

    // Precondition: a single joiner
    assert(!_joining);

    if(_state != FINISHING) {
        _joining = running();
        _joining->suspend(true);
    } else
        unlock();

    return *reinterpret_cast<int *>(_stack);
}


void Thread::pass()
{
    lock();

    db<Thread>(TRC) << "Thread::pass(this=" << this << ")" << endl;

    Thread * prev = running();
    Thread * next = _scheduler.choose(this);

    if(next)
        dispatch(prev, next, false);
    else {
        db<Thread>(WRN) << "Thread::pass => thread (" << this << ") not ready!" << endl;
        unlock();
    }
}


void Thread::suspend(bool locked)
{
    if(!locked)
        lock();

    db<Thread>(TRC) << "Thread::suspend(this=" << this << ")" << endl;

    Thread * prev = running();

    _scheduler.suspend(this);
    _state = SUSPENDED;

    Thread * next = running();

    dispatch(prev, next);
}


void Thread::resume()
{
    lock();

    db<Thread>(TRC) << "Thread::resume(this=" << this << ")" << endl;

    if(_state == SUSPENDED) {
        _state = READY;
        _scheduler.resume(this);
        
        if(preemptive)
            reschedule(_link.rank().queue());
    } else {
        db<Thread>(WRN) << "Resume called for unsuspended object!" << endl;

        unlock();
    }
}


// Class methods
void Thread::yield()
{
    lock();

    db<Thread>(TRC) << "Thread::yield(running=" << running() << ")" << endl;

    Thread * prev = running();
    Thread * next = _scheduler.choose_another();

    dispatch(prev, next);
}


void Thread::exit(int status)
{
    lock();

    db<Thread>(TRC) << "Thread::exit(running=" << running() <<",status=" << status << ")" << endl;

    Thread * prev = running();
    _scheduler.remove(prev);
    *reinterpret_cast<int *>(prev->_stack) = status;
    prev->_state = FINISHING;

    _thread_count--;

    if(prev->_joining) {
        prev->_joining->_state = READY;
        _scheduler.resume(prev->_joining);
        prev->_joining = 0;
    }

    dispatch(prev, _scheduler.choose());
}


void Thread::sleep(Queue * q)
{
    db<Thread>(TRC) << "Thread::sleep(running=" << running() << ",q=" << q << ")" << endl;

    // lock() must be called before entering this method
    assert(locked());

    Thread * prev = running();
    _scheduler.suspend(prev);
    prev->_state = WAITING;
    q->insert(&prev->_link);
    prev->_waiting = q;

    dispatch(prev, _scheduler.chosen());
}


void Thread::wakeup(Queue * q)
{
    db<Thread>(TRC) << "Thread::wakeup(running=" << running() << ",q=" << q << ")" << endl;

    // lock() must be called before entering this method
    assert(locked());

    if(!q->empty()) {
        Thread * t = q->remove()->object();
        t->_state = READY;
        t->_waiting = 0;
        _scheduler.resume(t);

        if(preemptive)
            reschedule(t->_link.rank().queue());
    } else
        unlock();
}


void Thread::wakeup_all(Queue * q)
{
    db<Thread>(TRC) << "Thread::wakeup_all(running=" << running() << ",q=" << q << ")" << endl;

    // lock() must be called before entering this method
    assert(locked());

    if(!q->empty())
        while(!q->empty()) {
            Thread * t = q->remove()->object();
            t->_state = READY;
            t->_waiting = 0;
            _scheduler.resume(t);

            if(preemptive) {
                reschedule(t->_link.rank().queue());
                lock();
            }
         }
    else
        unlock();
}


void Thread::reschedule()
{
    db<Scheduler<Thread> >(TRC) << "Thread::reschedule()" << endl;

    // lock() must be called before entering this method
    assert(locked());

    Thread * prev = running();
    Thread * next = _scheduler.choose();

    dispatch(prev, next);
}


void Thread::reschedule(unsigned int cpu)
{
    if(!smp || (cpu == Machine::cpu_id()))
        reschedule();
    else {
        db<Scheduler<Thread> >(TRC) << "Thread::reschedule(cpu=" << cpu << ")" << endl;
        IC::ipi_send(cpu, IC::INT_RESCHEDULER);
        unlock();
    }
}


void Thread::rescheduler(const IC::Interrupt_Id & interrupt)
{
    lock();

    reschedule();
}


void Thread::time_slicer()
{
    lock();

    reschedule();
}


void Thread::implicit_exit() 
{
    exit(CPU::fr()); 
}


int Thread::idle()
{
    while(true) {
        if(Traits<Thread>::trace_idle)
            db<Thread>(TRC) << "Thread::idle(CPU=" << Machine::cpu_id() << ",this=" << running() << ")" << endl;

        if(_thread_count <= Machine::n_cpus()) { // Only idle is left
            CPU::int_disable();
            if(Machine::cpu_id() == 0) {
                db<Thread>(WRN) << "The last thread has exited!" << endl;
                if(reboot) {
                    db<Thread>(WRN) << "Rebooting the machine ..." << endl;
                    Machine::reboot();
                } else
                    db<Thread>(WRN) << "Halting the machine ..." << endl;
            }
            CPU::halt();
        } else {
            CPU::int_enable();
            CPU::halt();
            if(_scheduler.schedulables() > 0) // A thread might have been woken up by another CPU
                yield();
        }
    }

    return 0;
}


// Id forwarder to the spin lock
unsigned int This_Thread::id() 
{ 
    return _not_booting ? reinterpret_cast<volatile unsigned int>(Thread::self()) : Machine::cpu_id() + 1;
}

__END_SYS
