// EPOS Application Scaffold and Application Abstraction Implementation

#include <utility/ostream.h>
#include <application.h>

// Application class attributes
__BEGIN_SYS
char Application::_preheap[];
char Application::_uncached_preheap[];
Heap * Application::_heap;
Heap * Application::_uncached_heap;
__END_SYS

// Global objects
__BEGIN_API
__USING_UTIL
OStream cout;
OStream cerr;
__END_API
