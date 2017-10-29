#include "Clock.hpp"
#include <utils/Conversions.hpp>

std::chrono::nanoseconds const Clock::OneSecond{1000000000};

//////////////////////////////////////////////////////////
//
// class Clock
//

Clock::Clock() :
	rack::Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS),
	m_interval(OneSecond)
{
	m_machine.registerStateType<ChangeFrequencyState>(Clock::STATE_FREQUENCY);
	m_machine.registerStateType<ChangeBPMState>(Clock::STATE_BPM);
	m_machine.change(Clock::STATE_FREQUENCY, *this);
	restart();
}

void Clock::restart()
{
	m_current = std::chrono::nanoseconds{0u};
	m_lastTime = std::chrono::steady_clock::now();
	m_previousValue = params[PARAM_VALUE].value;
}

void Clock::reset()
{
	m_interval = std::chrono::nanoseconds{OneSecond / 2};
	m_current = std::chrono::nanoseconds{0u};
	m_lastTime = std::chrono::steady_clock::now();
	m_previousValue = params[PARAM_VALUE].value;
}

void Clock::step()
{
	rack::Module::step();
	m_machine.step();

	auto const currentTime = std::chrono::steady_clock::now();
	auto const elaspedTime = currentTime - m_lastTime;

	m_lastTime = currentTime;
	m_current += elaspedTime;
	if (m_current >= m_interval)
	{
		m_current -= m_interval;
		m_clockTrigger = true;
		++m_value;
	}
	else
	{
		m_clockTrigger = false;
	}

	if (m_inputResetTrigger.process(inputs.at(INPUT_RESET).value))
	{
		m_current = std::chrono::nanoseconds{0u};
		m_clockTrigger = true;
	}

	outputs.at(OUTPUT_MAIN_CLOCK).value = m_clockTrigger ? 1.f : 0.f;

	if (m_buttonTrigger.process(params.at(Clock::PARAM_CHANGE_MODE).value))
	{
		unsigned int const newStateKey = (m_machine.currentStateKey() + 1u) % STATE_COUNT;

		m_machine.change(newStateKey, *this);
	}
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
	return m_interval;
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

//////////////////////////////////////////////////////////
//
// class Clock::ChangeFrequencyState
//
class Clock::ChangeFrequencyState : public ClockState
{
	using Seconds = std::chrono::duration<float>;
public:
	explicit ChangeFrequencyState(Clock& clock) :
		ClockState("Freq", clock)
	{
	}

	void beginState()
	{
		setCurrentText(formatValue(nanosecondsToFrequency(getInterval()), "Main\n%.2fHz"));
	}

	void onValueChanged(float value)
	{
		auto const f = m_minFrequency + (m_maxFrequency - m_minFrequency) * value;
		auto const interval = secondsToNanoseconds(Seconds{1.f / f});

		setInterval(interval);
		setCurrentText(formatValue(nanosecondsToFrequency(interval), "Main\n%.2fHz"));
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
public:
	explicit ChangeBPMState(Clock& clock) :
		ClockState("BPM", clock)
	{
	}

	void beginState()
	{
		auto const bpm = nanosecondToBpm(getInterval());

		setCurrentText(formatValue(static_cast<unsigned int>(bpm), "Main\n%uBPM"));
	}

	void onValueChanged(float value)
	{
		auto const bpm = m_minBPM + (m_maxBPM - m_minBPM) * value;

		setInterval(bpmToNanoseconds(bpm));
		setCurrentText(formatValue(static_cast<unsigned int>(bpm), "Main\n%uBPM"));
	}
private:
	unsigned int const m_minBPM = 1u;
	unsigned int const m_maxBPM = 300u;
};
