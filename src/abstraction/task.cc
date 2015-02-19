// EPOS Task Abstraction Implementation

#include <task.h>

__BEGIN_SYS

// Class attributes
Task * Task::_master;


// Methods
Task::Task(const Segment & code, const Segment & data)
: _as (new (SYSTEM) Address_Space), _cs(const_cast<Segment *>(&code)), _ds(const_cast<Segment *>(&data)), _code(_as->attach(*_cs)), _data(_as->attach(*_ds))
{
    db<Task>(TRC) << "Task(cs=" << _cs << ",ds=" << _ds << ") => " << this << endl;
}


Task::~Task()
{
    db<Task>(TRC) << "~Task(this=" << this << ")" << endl;

    while(!_threads.empty())
        delete _threads.remove()->object();
}

__END_SYS
