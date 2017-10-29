#if!defined CLOCK_HPP
#define CLOCK_HPP
#include <rack.hpp>
#include <dsp/digital.hpp>
#include <utils/FiniteStateMachine.hpp>
#include <chrono>

class Clock : public rack::Module
{
public:
	static std::chrono::nanoseconds const OneSecond;
	class ClockState;
	class ChangeFrequencyState;
	class ChangeBPMState;
	class ChangeRatioState;

	enum InputIds
	{
		INPUT_RESET,
		NUM_INPUTS
	};

	enum ParamIds
	{
		PARAM_CHANGE_MODE,
		PARAM_VALUE,
		NUM_PARAMS
	};

	enum OutputIds
	{
		OUTPUT_MAIN_CLOCK,
		NUM_OUTPUTS
	};

	enum LightIds
	{
		NUM_LIGHTS
	};

	enum StateIds : unsigned int
	{
		STATE_FREQUENCY,
		STATE_BPM,
		STATE_COUNT
	};

	using FSM = FiniteStateMachine<unsigned int, Clock&>;

	Clock();

	void restart();
	void reset() override;
	void step() override;

	std::chrono::nanoseconds getInterval()const;
	std::string getCurrentText()const;

	json_t *toJson() override;
	void fromJson(json_t *root) override;
private:
	FSM m_machine;
	std::chrono::nanoseconds m_interval;
	std::chrono::nanoseconds m_current;
	std::chrono::nanoseconds m_increment;
	std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> m_lastTime;
	float m_previousValue = 0.f;
	rack::SchmittTrigger m_inputResetTrigger;
	rack::SchmittTrigger m_buttonTrigger;
	bool m_clockTrigger = false;
	unsigned int m_value = 0u;
};

//////////////////////////////////////////////////////////
//
// class Clock::ClockState
//
class Clock::ClockState : public Clock::FSM::AState
{
public:
	explicit ClockState(std::string&& stateName, Clock& clock) :
		AState(std::move(stateName)),
		m_clock(clock)
	{
	}

	void stepState()
	{
	}

	void endState() {}

	virtual void onValueChanged(float value) = 0;

	std::string const& getCurrentText()const
	{
		return m_currentText;
	}
protected:
	template <class T>
	static std::string formatValue(T const& value, std::string const& format)
	{
		static std::unique_ptr<char[]> Buffer{new char[64]};

		std::snprintf(Buffer.get(), 64, format.c_str(), value);
		return Buffer.get();
	}

	void setInterval(std::chrono::nanoseconds interval)
	{
		m_clock.m_interval = interval;
	}

	std::chrono::nanoseconds getInterval()const
	{
		return m_clock.getInterval();
	}

	void reset()
	{
		m_clock.restart();
	}

	void setCurrentText(std::string const& text)
	{
		m_currentText = text;
	}
private:
	Clock& m_clock;
	std::string m_mode;
	std::string m_currentText;
};

#endif
