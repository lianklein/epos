// EPOS Application Scaffold and Application Abstraction Implementation

#include <utility/ostream.h>
#include <application.h>

// Application class attributes
__BEGIN_SYS
char Application::_preheap[];
Heap * Application::_heap;
__END_SYS

// Global objects
__BEGIN_API
__USING_UTIL
OStream cout;
OStream cerr;
__END_API
