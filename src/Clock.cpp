#include "Simple.hpp"
#include <utils/FourteenSegmentDisplay.hpp>
#include <utils/FiniteStateMachine.hpp>


class Clock : public rack::Module
{
public:
	static std::chrono::nanoseconds const OneSecond;
	class ClockState;
	class ChangeFrequencyState;
	class ChangeRatioState;

	enum InputIds
	{
		NUM_INPUTS
	};

	enum ParamIds
	{
		PARAM_CHANGE_MODE,
		NUM_PARAMS
	};

	enum OutputIds
	{
		NUM_OUTPUTS
	};

	Clock() :
		rack::Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS),
		m_interval(OneSecond)
	{
		restart();
	}

	void restart()
	{
		m_current = std::chrono::nanoseconds{0u};
		m_lastTime = std::chrono::steady_clock::now();
		m_value = 0u;
	}

	void step() override
	{
		m_machine.step();

		auto const currentTime = std::chrono::steady_clock::now();
		auto const elaspedTime = currentTime - m_lastTime;

		m_lastTime = currentTime;
		m_current += elaspedTime;
		if (m_current >= m_interval)
		{
			m_current = std::min(std::chrono::nanoseconds{0}, m_current - m_interval);
			m_clockTrigger = true;
			++m_value;
		}
		else
		{
			m_clockTrigger = false;
		}
	}

	unsigned int getValue()const
	{
		return m_value;
	}
private:
	FiniteStateMachine m_machine;
	std::chrono::nanoseconds m_interval;
	std::chrono::nanoseconds m_current;
	std::chrono::nanoseconds m_increment;
	std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> m_lastTime;
	bool m_clockTrigger = false;
	unsigned int m_value = 0u;
};

std::chrono::nanoseconds const Clock::OneSecond{1000000000};

class Clock::ClockState : public FiniteStateMachine::AState
{
public:
	explicit ClockState(std::string&& stateName, Clock* clock) :
		AState(std::move(stateName)),
		m_clock(clock)
	{
	}

	void stepState()
	{
	}

	virtual void onValueChanged(float value) = 0;
protected:
	void setInterval(std::chrono::nanoseconds interval)
	{
		m_clock->m_interval = interval;
	}

	void reset()
	{
		m_clock->restart();
	}

	void setCurrentText(std::string&& text)
	{
		m_currentText = std::move(text);
	}
private:
	Clock* const m_clock;
	std::string m_mode;
	std::string m_currentText;
};

class Clock::ChangeFrequencyState : public ClockState
{
public:
	explicit ChangeFrequencyState(Clock* clock) :
		ClockState("Frq", clock)
	{
	}

	void beginState() {}
	void stepState() {}
	void endState() {}

	void onValueChanged(float value)
	{
		auto const interval = m_maxInterval * value;
		auto const frequency = 1.f / static_cast<float>(std::chrono::duration_cast<std::chrono::seconds>(interval).count());

		setInterval(std::chrono::duration_cast<std::chrono::nanoseconds>(interval));
		setCurrentText("Freq: " + std::to_string(frequency));
	}
private:
	std::chrono::seconds m_maxInterval{60};
};

ClockWidget::ClockWidget() :
	m_clock(new Clock),
	m_segmentDisplay(new FourteenSegmentDisplay)
{
	auto* const mainPanel = new rack::LightPanel;
	auto const Margin = 5.f;

	box.size = rack::Vec(15 * 6, 380);
	m_segmentDisplay->box.pos = {Margin + 25.f, Margin};
	m_segmentDisplay->box.size = {15 * 6 - Margin - 25.f, 20.f};
	mainPanel->box.size = box.size;
	addChild(mainPanel);
	addChild(m_segmentDisplay);
	setModule(m_clock);

	auto* const button = createParam<rack::LEDButton>({0.f, 0.f}, Clock::PARAM_CHANGE_MODE, 0.f, 1.f, 0.f);
}

void ClockWidget::step()
{
	m_segmentDisplay->setText(std::to_string(m_clock->getValue()));
}
