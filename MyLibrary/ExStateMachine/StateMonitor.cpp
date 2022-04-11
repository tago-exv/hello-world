#include <iostream>
#include "ExNamedPipe.h"
#include "StateMonitor.h"

namespace exv {

	StateMonitor::StateMonitor(exv::ExNamedPipe* pipe)
	{
//		StateMachine::StateMachine();
//		ExtStateMachine<StateMonitor>map_.push_back({ "State1", &StateMonitor::state0 });

		pipe_ = pipe;
	}

/*
	StateMonitor::~StateMonitor()
	{
		delete pipe_;
	}
*/
	int StateMonitor::step()
	{
		int(StateMonitor:: * func)() = map_[state_].func;
		return (this->*func)();

	}

	void StateMonitor::monitor()
	{
	}

	int StateMonitor::state0() {
		std::cout << "  *** State Monitor ***\n";
		transState(1);
		return 1;
	}

	int StateMonitor::state1() {
		std::cout << ">> ";
		transState(2);
		return 1;

	}

	int StateMonitor::state2() {
		std::string s;
		if (pipe_->peek() > 0) {
			std::getline(pipe_->stream_, s);
			if (s == "end") {
				return -1;
			}
			else if (s == "monitor") {
				std::cout << "  === State machine monitor ===\n";
				processor_->monitor();
			}
			else if (s == "1") {
				std::cout << "  Trans state -> 1\n" << ">> ";
				transState(3);
			}
			else {
				std::cout << ">> ";
			}
		}
		return 1;
	}

	int StateMonitor::state3() {
		std::string s;
		if (pipe_->peek() > 0) {
			std::getline(pipe_->stream_, s);
			if (s == "end") {
				return -1;
			}
			else if (s == "0") {
				std::cout << "  Trans state -> 0\n" << ">> ";
				transState(2);
			}
			else {
				std::cout << ">> ";
			}
		}
		return 1;
	}

}