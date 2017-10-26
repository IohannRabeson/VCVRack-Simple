#include "FiniteStateMachine.hpp"
#include <cassert>

void FiniteStateMachine::push(unsigned int key)
{
	m_stack.emplace(createState(key));
	m_stack.top()->beginState();
}

void FiniteStateMachine::change(unsigned int key)
{
	pushEvent([this, key]()
	{
		pop();
		m_stack.emplace(createState(key));
		m_stack.top()->beginState();
	});
}

void FiniteStateMachine::pop()
{
	pushEvent([this]()
	{
		if (!m_stack.empty())
		{
			m_stack.top()->endState();
			m_stack.pop();
		}
	});
}

void FiniteStateMachine::clear()
{
	pushEvent([this]()
	{
		while (!m_stack.empty())
		{
			pop();
		}
	});
}

auto FiniteStateMachine::createState(unsigned int key)const -> StatePointer
{
	auto const creatorIt = m_creators.find(key);

	assert( creatorIt != m_creators.end() );

	return creatorIt->second();
}

void FiniteStateMachine::processEvents()
{
	while (!m_eventQueue.empty())
	{
		m_eventQueue.front()();
		m_eventQueue.pop();
	}
}

void FiniteStateMachine::pushEvent(Event&& event)
{
	m_eventQueue.emplace(std::move(event));
}

void FiniteStateMachine::step()
{
	processEvents();
	if (!m_stack.empty())
	{
		m_stack.top()->stepState();
	}
}
