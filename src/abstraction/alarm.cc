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
Alarm::Alarm(const Microsecond & time, Handler * handler, int times)
: _ticks(ticks(time)), _handler(handler), _times(times), _link(this, _ticks)
{
    lock();

    db<Alarm>(TRC) << "Alarm(t=" << time << ",tk=" << _ticks << ",h=" << reinterpret_cast<void *>(handler)
                   << ",x=" << times << ") => " << this << endl;

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

  _elapsed++;

  lock();
  if(Traits<Alarm>::visible) {
      Display display;
      int lin, col;
      display.position(&lin, &col);
      display.position(0, 79);
      display.putc(_elapsed);
      display.position(lin, col);
  }
  unlock();

  lock();
  if(!_request.empty()){
    Queue::Element * e = _request.head();
    unlock();

	  Alarm * alarm = e->object();
    Handler * handler = alarm->_handler;


    if(alarm->_ticks > 0)
        alarm->_ticks--;
    else {
      handler = 0;
      if(alarm->_times != -1)
          alarm->_times--;
      if(alarm->_times) {
          e->rank(alarm->_ticks);
      } else {
        _request.remove(e);
      }
    }

  	if(handler)
  	{
      (*handler)();
  	}
  } else {
    unlock();
  }
}

__END_SYS
