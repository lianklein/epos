// EPOS Application Initializer

#include <utility/heap.h>
#include <mmu.h>
#include <machine.h>
#include <application.h>
#include <address_space.h>
#include <segment.h>
#include <cpu.h>

__BEGIN_SYS


class Init_Application
{
public:

      Init_Application() {
      db<Init>(TRC) << "Init_Application()" << endl;

	    // Initialize Application's heap
	    db<Init>(INF) << "Initializing application's heap" << endl;

      // Obter o espaço de endereçamento a partir do registrador CR3
      MMU::Page_Directory * pd = MMU::current(); // Endereço do diretório de páginas, salvo no registrador CR3
      Address_Space as = Address_Space(pd);

      // Heap regular
      Segment * segment = new (&Application::_preheap) Segment(Traits<Application>::HEAP_SIZE);
      CPU::Log_Addr addr = as.attach(*segment);
      Application::_heap = new (&Application::_preheap) Heap(addr, segment->size());

      //Heap uncached
      MMU::Flags flags = MMU::Flags::CD | MMU::Flags::APP; // Desativar caching
      Segment * uncached_segment = new (&Application::_uncached_preheap) Segment(Traits<Application>::HEAP_SIZE, flags);
      CPU::Log_Addr addr_uncached = as.attach(*uncached_segment);
      Application::_uncached_heap = new (&Application::_uncached_preheap) Heap(addr_uncached, uncached_segment->size());

      db<Init>(INF) << "done!" << endl;
    }
};


// Global object "init_application"  must be linked to the application (not
// to the system) and there constructed at first.
Init_Application init_application;

__END_SYS
