// EPOS System Abstraction Initialization

#include <system.h>
#include <alarm.h>
#include <task.h>

__BEGIN_SYS

void System::init()
{
    Task::init();
    
    if(Traits<Alarm>::enabled)
        Alarm::init();
}

__END_SYS
