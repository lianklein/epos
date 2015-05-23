#include <system.h>
#include <task.h>

__BEGIN_SYS

Task * Task::_master;
Simple_List<Thread> Task::_threads;


Task::Task() { }

Task::Task(const Segment & cs, const Segment & ds)
{
	_address_space = new (SYSTEM) Address_Space (MMU::current());
	_code_segment = &cs;
	_data_segment = &ds;
	_code = CPU::Phy_Addr(Memory_Map<Machine>::APP_CODE);
	_data = CPU::Phy_Addr(Memory_Map<Machine>::APP_DATA);
}

Task::~Task()
{
	delete _address_space;
	delete _code_segment;
	delete _data_segment;

	while(!_threads.empty()){
		Thread * t = _threads.remove()->object();
		delete t;
	}
}

const Task * Task::self(){
		return Thread::self()->_task;
}

void Task::insert(Thread * t){
	_threads.insert(&t->_link_task);
	t->_task = this;
}


Address_Space * Task::address_space() const
{
	return _address_space;
}

const Segment * Task::code_segment() const
{
	return _code_segment;
}

const Segment * Task::data_segment() const
{
	return _data_segment;
}

CPU::Phy_Addr Task::code() const
{
	return _code;
}

CPU::Phy_Addr Task::data() const
{
	return _data;
}

__END_SYS
