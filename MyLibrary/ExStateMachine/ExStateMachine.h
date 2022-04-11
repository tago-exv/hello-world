#pragma once

#include <string>
#include <vector>

namespace exv
{

	class StateMachine {
	public:
		StateMachine() : state_(0) {};
		virtual int step() = 0;
		virtual void monitor() = 0;
		void transState(int i) { state_ = i; };

		int state_;
	};
/*
	template <typename T>
	class StateMap {
		struct State {
			std::string name;
			int (T::* func)();
		}
		std::vector<State>(map_);
	};
//		ExtStateMachine() {};
	template <typename T>
	int step() {
		int(T:: * func)() = map_[state_].func;
		return (this->*func)();
	};
*/

	class StateMachineProcessor {
	public:
		StateMachineProcessor(int period);

		~StateMachineProcessor();

		void registerTask(exv::StateMachine* task);
		void run();
		void monitor();
	private:
		std::vector<exv::StateMachine*> taskArray_;
		int cyclePeriod_;
	};

} // namespace exv
