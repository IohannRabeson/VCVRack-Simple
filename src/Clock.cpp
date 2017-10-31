#include "Clock.hpp"
#include <utils/Conversions.hpp>
#include <algorithm>

std::chrono::nanoseconds const Clock::OneSecond{1000000000};

//////////////////////////////////////////////////////////
//
// class Clock
//

unsigned int const Clock::Resolution = 128u;
unsigned int const Clock::MaxClockPosition = Clock::Resolution * 512u * 8u;

std::vector<std::pair<unsigned int, std::string>> const Clock::Resolutions =
{
	{Clock::Resolution * 8, "2 / 1"},
	{Clock::Resolution * 4, "1 / 1"},
	{Clock::Resolution * 2, "1 / 2"},
	{Clock::Resolution, "1 / 4"},
	{Clock::Resolution / 3, "1 / 4T"},
	{Clock::Resolution / 2, "1 / 8"},
	{Clock::Resolution / 6, "1 / 4T"},
	{Clock::Resolution / 4, "1 / 16"},
	{Clock::Resolution / 12, "1 / 16T"},
	{Clock::Resolution / 8, "1 / 32"},
	{Clock::Resolution / 24, "1 / 32T"},
	{Clock::Resolution / 16, "1 / 64"},
	{Clock::Resolution / 48, "1 / 64T"},
	{Clock::Resolution / 32, "1 / 128"},
	{Clock::Resolution / 96, "1 / 128T"}
};

Clock::Clock() :
	rack::Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS),
	m_interval(OneSecond)
{
	m_machine.registerStateType<ChangeFrequencyState>(Clock::STATE_FREQUENCY);
	m_machine.registerStateType<ChangeBPMState>(Clock::STATE_BPM);

	m_machine.registerStateType<ChangeResolutionState<0u>>(Clock::STATE_RESOLUTION_0);
	m_machine.registerStateType<ChangeDivisorState<0u>>(Clock::STATE_DIVISION_0);
	m_machine.registerStateType<ChangeGateTimeState<0u>>(Clock::STATE_GATE_TIME_0);
	m_machine.registerStateType<ChangeOutputVoltageState<0u>>(Clock::STATE_VOLTAGE_0);

	m_machine.registerStateType<ChangeResolutionState<1u>>(Clock::STATE_RESOLUTION_1);
	m_machine.registerStateType<ChangeDivisorState<1u>>(Clock::STATE_DIVISION_1);
	m_machine.registerStateType<ChangeGateTimeState<1u>>(Clock::STATE_GATE_TIME_1);
	m_machine.registerStateType<ChangeOutputVoltageState<1u>>(Clock::STATE_VOLTAGE_1);

	m_machine.registerStateType<ChangeResolutionState<2u>>(Clock::STATE_RESOLUTION_2);
	m_machine.registerStateType<ChangeDivisorState<2u>>(Clock::STATE_DIVISION_2);
	m_machine.registerStateType<ChangeGateTimeState<2u>>(Clock::STATE_GATE_TIME_2);
	m_machine.registerStateType<ChangeOutputVoltageState<2u>>(Clock::STATE_VOLTAGE_2);

	m_machine.registerStateType<ChangeResolutionState<3u>>(Clock::STATE_RESOLUTION_3);
	m_machine.registerStateType<ChangeDivisorState<3u>>(Clock::STATE_DIVISION_3);
	m_machine.registerStateType<ChangeGateTimeState<3u>>(Clock::STATE_GATE_TIME_3);
	m_machine.registerStateType<ChangeOutputVoltageState<3u>>(Clock::STATE_VOLTAGE_3);

	m_machine.change(Clock::STATE_BPM, *this);
	restart();
}

void Clock::restart()
{
	m_current = std::chrono::nanoseconds{0u};
	m_lastTime = std::chrono::steady_clock::now();
	m_previousValue = params[PARAM_VALUE].value;
	m_clockPosition = 0u;
	std::for_each(m_outputs.begin(), m_outputs.end(), [](ClockOutput& output){ output.restart(); });
}

void Clock::reset()
{
	m_interval = std::chrono::nanoseconds{OneSecond / 2};
	m_machine.change(Clock::STATE_BPM, *this);
	std::for_each(m_outputs.begin(), m_outputs.end(), [](ClockOutput& output){ output.recallDefaultValues(); });
	restart();
}

void Clock::updateClockTrigger()
{
	auto const currentTime = std::chrono::steady_clock::now();
	auto const elaspedTime = currentTime - m_lastTime;
	auto const interval = getInterval();

	m_lastTime = currentTime;
	m_current += elaspedTime;
	if (m_current >= interval)
	{
		m_current -= interval;
		// The maximum divisor is 512 so we can have at most
		// Resolution * 512u ticks to count.
		m_clockPosition = (m_clockPosition + 1) % (MaxClockPosition);
		m_clockTrigger = true;
	}
	else if (m_inputResetTrigger.process(inputs.at(INPUT_RESET).value))
	{
		restart();
		m_clockTrigger = true;
	}
	else
	{
		m_clockTrigger = false;
	}
	if (m_clockTrigger)
	{
		for (auto i = 0u; i < m_outputs.size(); ++i)
		{
			auto& output = getOutput(i);
			auto const gate = output.step(m_clockPosition, elaspedTime);

			outputs.at(OUTPUT_CLOCK_0 + i).value = gate ? output.getOutputVoltage() : 0.f;
		}
	}
}

void Clock::updateCurrentState()
{
	if (m_buttonTrigger.process(params.at(Clock::PARAM_CHANGE_MODE).value))
	{
		unsigned int const newStateKey = (m_machine.currentStateKey() + 1u) % STATE_COUNT;

		m_machine.change(newStateKey, *this);
	}
}

void Clock::step()
{
	rack::Module::step();
	m_machine.step();

	updateClockTrigger();
	updateCurrentState();

	if (m_machine.hasState())
	{
		auto& currentState = static_cast<Clock::ClockState&>(m_machine.currentState());
		auto const currentValue = params.at(Clock::PARAM_VALUE).value;

		if (!(std::abs(currentValue - m_previousValue) < std::numeric_limits<float>::epsilon()))
		{
			currentState.onValueChanged(currentValue);
		}
		m_previousValue = currentValue;
	}
}

std::chrono::nanoseconds Clock::getInterval()const
{
	return m_interval / Resolution;
}

std::string Clock::getCurrentText()const
{
	std::string text;

	if (m_machine.hasState())
	{
		auto& currentState = static_cast<Clock::ClockState&>(m_machine.currentState());

		text = currentState.getCurrentText();
	}
	return text;
}

void Clock::setGateTime(unsigned int const divisorIndex, std::chrono::nanoseconds const time)
{
	getOutput(divisorIndex).setGateTime(time);
}

std::chrono::nanoseconds Clock::getGateTime(unsigned int const divisorIndex)const 
{
	return getOutput(divisorIndex).getGateTime();
}

json_t *Clock::toJson()
{
	json_t *rootNode = json_object();
	auto const nanoSecondInterval = std::to_string(m_interval.count());

	json_object_set_new(rootNode, "interval", json_string(nanoSecondInterval.c_str()));
	json_object_set_new(rootNode, "state", json_integer(m_machine.currentStateKey()));
	return rootNode;
}

void Clock::fromJson(json_t *root)
{
	json_t* const intervalNode = json_object_get(root, "interval");
	json_t* const stateNode = json_object_get(root, "state");

	if (intervalNode && json_is_string(intervalNode) &&
		stateNode && json_is_integer(stateNode))
	{
		std::string const intervalText{json_string_value(intervalNode)};

		m_interval = std::chrono::nanoseconds{std::stoull(intervalText)};
		m_machine.change(json_integer_value(stateNode), *this);
	}
}

auto Clock::getOutput(unsigned int const index) -> ClockOutput&
{
	assert( index < m_outputs.size() );

	return m_outputs[index];
}

auto Clock::getOutput(unsigned int const index)const -> ClockOutput const&
{
	assert( index < m_outputs.size() );

	return m_outputs[index];
}

void Clock::setDivisor(unsigned int const index, unsigned int const divisor)
{
	getOutput(index).setDivisor(divisor);
}

unsigned int Clock::getDivisor(unsigned int const index)const
{
	return getOutput(index).getDivisor();
}

void Clock::setOutputVoltage(unsigned int const index, float const voltage)
{
	getOutput(index).setOutputVoltage(voltage);
}

float Clock::getOutputVoltage(unsigned int const index) const
{
	return getOutput(index).getOutputVoltage();
}

void Clock::setResolutionIndex(unsigned int const index, std::size_t const resolutionIndex)
{
	getOutput(index).setResolutionIndex(resolutionIndex);
}

std::size_t Clock::getResolutionIndex(unsigned int const index)
{
	return getOutput(index).getResolutionIndex();
}

//////////////////////////////////////////////////////////
//
// class Clock::Output
//
bool Clock::ClockOutput::step(int const clockPosition, std::chrono::nanoseconds const dt)
{
	auto const resolution = Resolutions[m_resolutionIndex].first;

	if (clockPosition % (m_divisor * resolution) == 0)
	{
		m_currentGateTime = std::chrono::nanoseconds{0u};
	}
	return gateStep(dt);
}

void Clock::ClockOutput::restart()
{
	m_currentGateTime = std::chrono::nanoseconds{0u};
}

void Clock::ClockOutput::recallDefaultValues()
{
	m_gateTime = std::chrono::nanoseconds{0};
	m_outputVoltage = 10.f;
	m_divisor = 1u;
	m_resolutionIndex = 3u;
}

void Clock::ClockOutput::setDivisor(unsigned int divisor)
{
	assert (divisor > 0);

	m_divisor = divisor;
}

unsigned int Clock::ClockOutput::getDivisor()const
{
	return m_divisor;
}

void Clock::ClockOutput::setGateTime(std::chrono::nanoseconds const& time)
{
	m_gateTime = time;
}

std::chrono::nanoseconds Clock::ClockOutput::getGateTime()const
{
	return m_gateTime;
}

void Clock::ClockOutput::setOutputVoltage(float const voltage)
{
	m_outputVoltage = voltage;
}

float Clock::ClockOutput::getOutputVoltage()const
{
	return m_outputVoltage;
}

void Clock::ClockOutput::setResolutionIndex(std::size_t const index)
{
	assert( index < Resolutions.size() );
	m_resolutionIndex = index;
}

std::size_t Clock::ClockOutput::getResolutionIndex()const
{
	return m_resolutionIndex;
}

bool Clock::ClockOutput::gateStep(std::chrono::nanoseconds const dt)
{
	bool result = false;

	if (m_currentGateTime <= m_gateTime)
	{
		m_currentGateTime += dt;
		result = true;
	}
	return result;
}

//////////////////////////////////////////////////////////
//
// class Clock::ChangeFrequencyState
//
class Clock::ChangeFrequencyState : public ClockState
{
	using Seconds = std::chrono::duration<float>;
	static constexpr char const* const Format = "Main\n  %.2fHz";
public:
	explicit ChangeFrequencyState(Clock& clock) :
		ClockState("Freq", clock)
	{
	}

	void beginState()
	{
		setCurrentText(formatValue(Format, nanosecondsToFrequency(getInterval())));
	}

	void onValueChanged(float value)
	{
		auto const f = m_minFrequency + (m_maxFrequency - m_minFrequency) * value;
		auto const interval = secondsToNanoseconds(Seconds{1.f / f});

		setInterval(interval);
		setCurrentText(formatValue(Format, nanosecondsToFrequency(interval)));
	}
private:
	float const m_minFrequency = 0.01f;
	float const m_maxFrequency = 20.f;
};

//////////////////////////////////////////////////////////
//
// class Clock::ChangeBPMState
//
class Clock::ChangeBPMState : public ClockState
{
	using Seconds = std::chrono::duration<float>;
	static constexpr char const* const Format = "Main\n  %u BPM";
public:
	explicit ChangeBPMState(Clock& clock) :
		ClockState("BPM", clock)
	{
	}

	void beginState()
	{
		auto const bpm = nanosecondToBpm(getInterval());

		setCurrentText(formatValue(Format, static_cast<unsigned int>(bpm)));
	}

	void onValueChanged(float value)
	{
		auto const bpm = m_minBPM + (m_maxBPM - m_minBPM) * value;

		setInterval(bpmToNanoseconds(bpm));
		setCurrentText(formatValue(Format, static_cast<unsigned int>(bpm)));
	}
private:
	unsigned int const m_minBPM = 1u;
	unsigned int const m_maxBPM = 300u;
};

//////////////////////////////////////////////////////////
//
// class Clock::ChangeDivisorState
//
template <unsigned int Index>
class Clock::ChangeDivisorState : public ClockState
{
	static constexpr char const* const Format = "Divisor %u\n/%u";
public:
	explicit ChangeDivisorState(Clock& clock) :
		ClockState("Divisor", clock)
	{
	}

	void beginState()
	{
		setCurrentText(formatValue(Format, Index, getDivisor(Index)));
	}

	void onValueChanged(float value)
	{
		auto const divisor = m_minDivisor + static_cast<unsigned int>(static_cast<float>(m_maxDivisor - m_minDivisor) * value);

		setDivisor(Index, divisor);
		setCurrentText(formatValue(Format, Index, divisor));
	}
private:
	unsigned int m_minDivisor = 1u;
	unsigned int m_maxDivisor = 512u;
};


//////////////////////////////////////////////////////////
//
// class Clock::ChangeResolutionState
//
template <unsigned int Index>
class Clock::ChangeResolutionState : public ClockState
{
	static constexpr char const* const Format = "Resolution %u\n  %s";
public:
	explicit ChangeResolutionState(Clock& clock) :
		ClockState("Resolution", clock)
	{
	}

	void beginState()
	{
		auto const resolutionIndex = clock().getResolutionIndex(Index);

		setCurrentText(formatValue(Format, Index, Resolutions[resolutionIndex].second.c_str()));
	}

	void onValueChanged(float value)
	{
		assert(Resolutions.size() > 0);

		std::size_t resolutionIndex = static_cast<float>(Clock::Resolutions.size() - 1) * value;

		clock().setResolutionIndex(Index, resolutionIndex);
		setCurrentText(formatValue(Format, Index, Resolutions[resolutionIndex].second.c_str()));
	}
};

//////////////////////////////////////////////////////////
//
// class Clock::ChangeGateTimeState
//
template <unsigned int Index>
class Clock::ChangeGateTimeState : public ClockState
{
	using Seconds = std::chrono::duration<float>;
	static constexpr char const* const Format = "Gate time %u\n  %.3fs";
public:
	explicit ChangeGateTimeState(Clock& clock) :
		ClockState("Gate time", clock)
	{
	}

	void beginState()
	{
		auto const gateTime = std::chrono::duration_cast<Seconds>(getGateTime(Index));

		setCurrentText(formatValue(Format, Index, gateTime.count()));
	}

	void onValueChanged(float value)
	{
		auto const gateTime = m_minTime + (m_maxTime - m_minTime) * value;

		setGateTime(Index, std::chrono::duration_cast<std::chrono::nanoseconds>(gateTime));
		setCurrentText(formatValue(Format, Index, gateTime.count()));
	}
private:
	Seconds const m_minTime{0.f};
	Seconds const m_maxTime{10.f};
};

//////////////////////////////////////////////////////////
//
// class Clock::ChangeOutputVoltageState
//
template <unsigned int Index>
class Clock::ChangeOutputVoltageState : public ClockState
{
	static constexpr char const* const Format = "Voltage %u\n  %.2fv";
public:
	explicit ChangeOutputVoltageState(Clock& clock) :
		ClockState("Output voltage", clock)
	{
	}

	void beginState()
	{
		auto const voltage = getOutputVoltage(Index);

		setCurrentText(formatValue(Format, Index, voltage));
	}

	void onValueChanged(float value)
	{
		auto const voltage = m_min + (m_max - m_min) * value;

		setOutputVoltage(Index, voltage);
		setCurrentText(formatValue(Format, Index, voltage));
	}
private:
	float const m_min = 0.f;
	float const m_max = 10.f;
};
