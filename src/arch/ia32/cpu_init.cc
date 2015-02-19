// EPOS IA32 CPU Mediator Initialization

#include <cpu.h>
#include <tsc.h>
#include <mmu.h>
#include <pmu.h>
#include <system.h>
#include <system/info.h>

__BEGIN_SYS

void IA32::init()
{
    db<Init, CPU>(TRC) << "CPU::init()" << endl;

    _cpu_clock = System::info()->tm.cpu_clock;
    _bus_clock = System::info()->tm.bus_clock;

    // Initialize the MMU
    if(Traits<MMU>::enabled)
        MMU::init();
    else
        db<Init, MMU>(WRN) << "MMU is disabled!" << endl;

    // Initialize the PMU	
    if(Traits<PMU>::enabled)
        PMU::init();
}

__END_SYS
