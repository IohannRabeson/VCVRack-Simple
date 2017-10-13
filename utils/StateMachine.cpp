#include "StateMachine.hpp"

void StateMachine::noOp(StateMachine&)
{
}

StateMachine::StateMachine()
{
	m_currentState = &StateMachine::noOp;
}

void StateMachine::addState(unsigned int index, State&& state)
{
	m_states[index] = std::move(state);
}

void StateMachine::change(unsigned int index)
{
	auto const it = m_states.find(index);

	assert (it != m_states.end());

	m_currentState = it->second;
}

void StateMachine::step()
{
	m_currentState(*this);
}
