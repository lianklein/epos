#ifndef __task_h
#define __task_h

#include <address_space.h>
#include <segment.h>
#include <mmu.h>
#include <system.h>
#include <utility/list.h>

__BEGIN_SYS

class Task
{
	friend class System;
	friend class Thread;

public:
	Task(const Segment & cs, const Segment & ds);
	~Task();

	Address_Space * address_space() const;
	const Segment * code_segment() const;
	const Segment * data_segment() const;
	CPU::Phy_Addr code() const;
	CPU::Phy_Addr data() const;
	const static Task * self();

	void insert(Thread * t);


private:
	Task();

	static void init() {
		System_Info<Machine> * si = System::info();

		_master = new (SYSTEM) Task();
		_master->_address_space = new (SYSTEM) Address_Space (MMU::current());
		_master->_code_segment = new (SYSTEM) Segment (CPU::Phy_Addr(si->lm.app_code), si->lm.app_code_size);
		_master->_data_segment = new (SYSTEM) Segment (CPU::Phy_Addr(si->lm.app_data), si->lm.app_data_size);
		_master->_code = CPU::Phy_Addr(Memory_Map<Machine>::APP_CODE);
		_master->_data = CPU::Phy_Addr(Memory_Map<Machine>::APP_DATA);
	}


private:
	//Manter uma referência das threads da instância
	Address_Space * _address_space;
	const Segment * _code_segment;
	const Segment * _data_segment;
	CPU::Phy_Addr _code;
	CPU::Phy_Addr _data;

	void activate() { _address_space->activate(); };

	static Simple_List<Thread> _threads;
	static Task * _master;
};

__END_SYS

#include <thread.h>

#endif
