// EPOS Handler Utility Declarations

#ifndef __handler_h
#define __handler_h

#include <system/config.h>

__BEGIN_UTIL

typedef void (function)();

class Handler
{
	public:
		Handler(){};
		~Handler(){};
		virtual void operator()(){};
};

class Function_Handler : public Handler
{
	public:
		Function_Handler (function* f)
		{
			_f = f;
		}
		~Function_Handler(){};

		void operator()()
		{
			(*_f)();
		};
		
		private:
			function* _f;
};

__END_UTIL

#endif
