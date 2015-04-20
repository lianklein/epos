// EPOS Alarm Abstraction Test Program

#include <utility/ostream.h>
#include <alarm.h>
#include <mutex.h>

using namespace EPOS;

const int iterations = 10;

void func_a(void);
void func_b(void);

OStream cout;

int main()
{
    cout << "Alarm test" << endl;

    cout << "I'm the first thread of the first task created in the system." << endl;
    cout << "I'll now create two alarms and put myself in a delay ..." << endl;

	  Function_Handler fa (&func_a);
	  Function_Handler fb (&func_b);

	  Alarm alarm_a(2000000, &fa, iterations);
    Alarm alarm_b(1000000, &fb, iterations);

    // Note that in case of idle-waiting, this thread will go into suspend
    // and the alarm handlers above will trigger the functions in the context
    // of the idle thread!
    Alarm::delay(1000000 * (iterations + 1));

    cout << "I'm done, bye!" << endl;

    return 0;
}

void func_a()
{
    for(int i = 0; i < 79; i++)
        cout << "a";
    cout << endl;
}

 void func_b(void)
{
    for(int i = 0; i < 79; i++)
        cout << "b";
    cout << endl;
}
