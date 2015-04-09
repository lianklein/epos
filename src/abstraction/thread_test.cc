// EPOS Thread Test Program

#include <utility/ostream.h>
#include <thread.h>
#include <alarm.h>

using namespace EPOS;

const int iterations = 6;

int func_a(void);
int func_b(void);

Thread * a;
Thread * b;
Thread * m;

OStream cout;

int main()
{
    cout << "Thread test" << endl;

    m = Thread::self();

    cout << "I'm the first thread of the first task created in the system." << endl;
    cout << "I'll now create two threads and then wait for them to finish ..." << endl;

    a = new Thread(&func_a);
    b = new Thread(&func_b);

    cout << "MAIN: " << m << endl;
    cout << "A: " << a << endl;
    cout << "B: " << b << endl;


    int status_a = a->join();
    int status_b = b->join();

    cout << "Thread A exited with status " << status_a 
         << " and thread B exited with status " << status_b << "" << endl;

    delete a;
    delete b;
    delete m;
    
    cout << "It should not be shown on the display!" << endl;

    return 0;
}

int func_a(void)
{
    for(int i = iterations; i > 0; i--) {
        for(int i = 0; i < 149; i++)
            cout << "a";
        cout << "" << endl;
        Alarm::delay(500000);
    }

    return 'A';   
}

int func_b(void)
{
    for(int i = iterations; i > 0; i--) {
        for(int i = 0; i < 149; i++)
            cout << "b";
        cout << "" << endl;
        Alarm::delay(500000);
    }

    return 'B';   
}
