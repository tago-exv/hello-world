#include <chrono>
#include <thread>

#include "ExStateMachine.h"


int exv::StateMachine::step()
{
//	return state_ = this->stateMap_[state_].func();
	return state_;
}

exv::StateMachineProcessor::StateMachineProcessor(int period) {
	cyclePeriod_ = period;

}

exv::StateMachineProcessor::~StateMachineProcessor()
{
}

void exv::StateMachineProcessor::registerTask(StateMachine *task)
{
	taskArray_.push_back(task);
}

void exv::StateMachineProcessor::run()
{
	for( bool exit = false; exit == false ;) {
		for each (auto task in taskArray_)
		{
			if (task->step() == -1) {
				exit = true;
				break;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(cyclePeriod_));
	}
}

void exv::StateMachineProcessor::monitor()
{
	for each (auto task in taskArray_) {
		task->monitor();
	}
}
