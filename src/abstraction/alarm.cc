// EPOS Alarm Abstraction Implementation

#include <semaphore.h>
#include <alarm.h>
#include <display.h>

__BEGIN_SYS

// Class attributes
Alarm_Timer * Alarm::_timer;
volatile Alarm::Tick Alarm::_elapsed;
Alarm::Queue Alarm::_request;


// Methods
Alarm::Alarm(const Microsecond & time, Handler * handler, unsigned long times)
: _ticks(ticks(time)), _handler(handler), _times(times), _link(this, _ticks)
{
    lock();

    db<Alarm>(TRC) << "Alarm(t=" << time << ",tk=" << _ticks << ",h=" << reinterpret_cast<void *>(handler)
                   << ",x=" << times << ") => " << this << endl;

     _original_ticks = _ticks;

    if(_ticks) {
        _request.insert(&_link);
        unlock();
    } else {
        unlock();
        (*handler)();
    }
}


Alarm::~Alarm()
{
    lock();

    db<Alarm>(TRC) << "~Alarm(this=" << this << ")" << endl;

    _request.remove(this);

    unlock();
}

// Class methods
void Alarm::delay(const Microsecond & time)
{
    db<Alarm>(TRC) << "Alarm::delay(time=" << time << ")" << endl;

	Semaphore semaphore(0);
	Semaphore_Handler semaphore_handler(&semaphore);
	Alarm alarm(time, &semaphore_handler, 1);
	semaphore.p();
}

void Alarm::handler(const IC::Interrupt_Id & i)
{
  lock();
  Handler * handler = 0;

  _elapsed++;

  if(Traits<Alarm>::visible) {
      Display display;
      int lin, col;
      display.position(&lin, &col);
      display.position(0, 79);
      display.putc(_elapsed);
      display.position(lin, col);
  }
  if(!_request.empty()){
    Queue::Element * e = _request.head();
	  Alarm * alarm = e->object();

    if(alarm->_ticks > 1)
        alarm->_ticks--;
    else {
      if(alarm->_handler) {
        handler = alarm->_handler;
      }
      _request.remove(e);
      if(alarm->_times != -1)
          alarm->_times--;
      if(alarm->_times) {
          db<Alarm>(TRC) << "Alarm::handler(ticks=" << alarm->_ticks << ")" << endl;
          alarm->_ticks = alarm->_original_ticks;
          e->rank(alarm->_ticks);
          _request.insert(e);
      }
    }
  }
  unlock();
  if(handler) {
    (*handler)();
  }
}

__END_SYS
