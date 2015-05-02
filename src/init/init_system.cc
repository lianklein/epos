// EPOS System Initializer

#include <utility/random.h>
#include <machine.h>
#include <system.h>
#include <address_space.h>
#include <segment.h>
#include <cpu.h>

extern "C" { void __epos_library_app_entry(void); }

__BEGIN_SYS

class Init_System
{
public:
    Init_System() {
        db<Init>(TRC) << "Init_System()" << endl;

        // Initialize the processor
        db<Init>(INF) << "Initializing the CPU: " << endl;
        CPU::init();
        db<Init>(INF) << "done!" << endl;

        // Initialize System's heap
        db<Init>(INF) << "Initializing system's heap: " << endl;
        Segment * segment = new (&System::_preheap[0]) Segment(Traits<System>::HEAP_SIZE);

        MMU::Page_Directory * pd = MMU::current(); // Endereço do diretório de páginas, salvo no registrador CR3
        Address_Space as = Address_Space(pd);
        CPU::Log_Addr addr = as.attach(*segment, Memory_Map<Machine>::SYS_HEAP); // SYS_HEAP é o endereço da área de dados do sistema + offset de 4MB

        System::_heap   = new (&System::_preheap[0]) Heap(addr, segment->size());

        db<Init>(INF) << "done!" << endl;

        // Initialize the machine
        db<Init>(INF) << "Initializing the machine: " << endl;
        Machine::init();
        db<Init>(INF) << "done!" << endl;

        // Initialize system abstractions
        db<Init>(INF) << "Initializing system abstractions: " << endl;
        System::init();
        db<Init>(INF) << "done!" << endl;

        // Randomize the Random Numbers Generator's seed
        if(Traits<Random>::enabled) {
            db<Init>(INF) << "Randomizing the Random Numbers Generator's seed: " << endl;
            if(Traits<TSC>::enabled)
                Random::seed(TSC::time_stamp());

            if(!Traits<TSC>::enabled)
                db<Init>(WRN) << "Due to lack of entropy, Random is a pseudo random numbers generator!" << endl;
            db<Init>(INF) << "done!" << endl;
        }

        // Initialization continues at init_first
    }
};

// Global object "init_system" must be constructed first.
Init_System init_system;

__END_SYS
