#pragma once
#include "ExStateMachine.h"
#include "ExNamedPipe.h"
#

namespace exv {

class StateMonitor : public StateMachine
{
public:
	StateMonitor(exv::ExNamedPipe* pipe);
//	~StateMonitor();

	int step() override;
	void monitor() override;
	void registerProcessor(StateMachineProcessor* p) { processor_ = p; };

	struct StateMap {
		std::string name;
		int (StateMonitor::* func)();
	} map_[4] = {
		{"State1", &StateMonitor::state0},
		{"State2", &StateMonitor::state1},
		{"State3", &StateMonitor::state2},
		{"State4", &StateMonitor::state3}
	};

	int state0();
	int state1();
	int state2();
	int state3();


//	int state_;
	exv::ExNamedPipe *pipe_;
	exv::StateMachineProcessor* processor_;
};

}


