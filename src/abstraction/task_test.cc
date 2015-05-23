// EPOS Task Test Program

#include <utility/ostream.h>
#include <alarm.h>
#include <thread.h>
#include <task.h>

using namespace EPOS;

const int iterations = 10;

int func_a(void);
int func_b(void);

Thread * a;
Thread * b;
Thread * m;

OStream cout;

int fn_a(){
  cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;
  return 3;
}


int main()
{
    cout << "Task test" << endl;

    m = Thread::self();

    cout << "I'll try to clone myself:" << endl;

    const Task * task0 = Task::self();
    Address_Space * as0 = task0->address_space();
    cout << "My address space's page directory is located at " << as0->pd() << endl;

    const Segment * cs0 = task0->code_segment();
    CPU::Log_Addr code0 = task0->code();
    cout << "My code segment is located at "
         << static_cast<void *>(code0)
         << " and it is " << cs0->size() << " bytes long" << endl;

    const Segment * ds0 = task0->data_segment();
    CPU::Log_Addr data0 = task0->data();
    cout << "My data segment is located at "
         << static_cast<void *>(data0)
         << " and it is " << ds0->size() << " bytes long" << endl;

    cout << "Creating and attaching segments:" << endl;
    Segment cs1(cs0->size());
    CPU::Log_Addr code1 = as0->attach(cs1);
    cout << " code => " << code1 << " done!" << endl;
    Segment ds1(ds0->size());
    CPU::Log_Addr data1 = as0->attach(ds1);
    cout << " data => " << data1 << " done!" << endl;

    cout << "Copying segments:";
    memcpy(code1, code0, cs1.size());
    cout << " code => done!" << endl;
    memcpy(data1, data0, ds1.size());
    cout << " data => done!" << endl;

    cout << "Detaching segments:";
    as0->detach(cs1);
    as0->detach(ds1);
    cout << " done!" << endl;

    cout << "Creating a clone task:";
    Task * task1 = new Task(cs1, ds1);
    cout << " done!" << endl;

    Thread * t = new Thread(task1, &fn_a);
    t->join();

    cout << "Deleting the cloned task:";
    delete task1;
    cout << " done!" << endl;

    cout << "I'm also done, bye!" << endl;

    return 0;
}
