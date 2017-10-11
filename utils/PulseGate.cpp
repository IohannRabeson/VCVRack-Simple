#include "PulseGate.hpp"
#include <iostream>

PulseGate::PulseGate(unsigned int pulseDuration) :
	m_pulseDuration(pulseDuration)
{
}

void PulseGate::reset()
{
	m_currentCount = 0u;
	m_pulse = false;
}

bool PulseGate::process(bool gate)
{
	if (gate)
	{
		m_currentCount = 0u;
		m_pulse = true;
	}
	if (m_currentCount >= m_pulseDuration)
	{
		m_pulse = false;
	}
	++m_currentCount;
	return m_pulse;
}
