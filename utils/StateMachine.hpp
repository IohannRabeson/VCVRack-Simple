#if!defined STATEMACHINE_HPP
#define STATEMACHINE_HPP
#include <memory>
#include <map>

#include <rack.hpp>

class StateMachine
{
public:
	using State = std::function<void(StateMachine&)>;

	static void noOp(StateMachine&);

	StateMachine();

	void addState(unsigned int index, State&& state);
	void change(unsigned int index);

	void step();
private:
	std::map<unsigned int, State> m_states;
	State m_currentState;
};

#endif
