#include "Simple.hpp"
#include <utils/PulseGate.hpp>
#include <dsp/digital.hpp>

#include <array>
#include <algorithm>
#include <iostream>

namespace
{
	static constexpr unsigned int const MaxDivider = 128u;

	class ClockDividerImp
	{
	public:
		explicit ClockDividerImp(unsigned int index, unsigned int limit = 1u) :
			m_lightPulse(4096u),
			m_index(index),
			m_limit(limit)
		{
		}

		void setLimit(unsigned int limit){ m_limit = limit;	}

		void reset()
		{
			m_current = 0u;
			m_lightPulse.reset();
		}

		void process(bool const clockTrigger,
					 std::vector<rack::Param> const& params,
					 std::vector<rack::Output>& outputs)
		{
			auto const& param = params.at(m_index);
			auto& output = outputs.at(m_index);
			auto gate = false;

			m_limit = static_cast<unsigned int>(param.value);
			if (clockTrigger)
			{
				++m_current;
				if (m_current >= m_limit)
				{
					gate = true;
					m_current = 0u;
				}
			}
			output.value = gate ? 10.f : 0.f;
			m_lightState = m_lightPulse.process(gate) ? 1.0 : 0.f;
		}

		float* lightState()
		{
			return &m_lightState;
		}
	private:
		PulseGate m_lightPulse;
		unsigned int const m_index;
		unsigned int m_current = 0u;
		unsigned int m_limit = 1u;
		float m_lightState = 0.f;
	};
}

struct ClockDivider : rack::Module
{
	enum ParamsIds
	{
		CLOCK_DIVIDER_0,
		CLOCK_DIVIDER_1,
		CLOCK_DIVIDER_2,
		CLOCK_DIVIDER_3,
		CLOCK_DIVIDER_4,
		CLOCK_DIVIDER_5,
		CLOCK_DIVIDER_6,
		CLOCK_DIVIDER_7,
		NUM_PARAMS
	};

	enum InputIds
	{
		INPUT_CLOCK,
		INPUT_RESET,
		NUM_INPUTS
	};

	enum OutputIds
	{
		OUTPUT_CLOCK_0,
		OUTPUT_CLOCK_1,
		OUTPUT_CLOCK_2,
		OUTPUT_CLOCK_3,
		OUTPUT_CLOCK_4,
		OUTPUT_CLOCK_5,
		OUTPUT_CLOCK_6,
		OUTPUT_CLOCK_7,
		NUM_OUTPUTS
	};

	ClockDivider() :
		Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
	{
		auto initClockDivider = 1u;

		for (int i = OUTPUT_CLOCK_0; i < NUM_OUTPUTS; ++i)
		{
			m_clockDividers.emplace_back(i, std::min(128u, initClockDivider));
			initClockDivider *= 2u;
		};
	}

	void reset()
	{
		std::for_each(m_clockDividers.begin(), m_clockDividers.end(),
					  [](ClockDividerImp& imp) { imp.reset(); });
	}

	void step() override
	{
		auto const& inputClock = inputs.at(INPUT_CLOCK);
		auto const& inputReset = inputs.at(INPUT_RESET);
		bool const clockTick = inputClock.active && m_clockTrigger.process(inputClock.value);

		if (inputReset.active && m_resetTrigger.process(inputReset.value))
		{
			reset();
		}
		for (auto& divider : m_clockDividers)
		{
			divider.process(clockTick, params, outputs);
		}
	}

	float* lightState(std::size_t const i)
	{
		return m_clockDividers[i].lightState();
	}

	void initialize() override
	{
		reset();
	}

	void randomize() override
	{
		reset();
	}
private:
	std::vector<ClockDividerImp> m_clockDividers;
	rack::SchmittTrigger m_resetTrigger;
	rack::SchmittTrigger m_clockTrigger;
};

struct ClockDividerKnob : rack::Davies1900hSmallBlackSnapKnob
{
	void connectLabel(rack::Label* label)
	{
		linkedLabel = label;
		if (linkedLabel)
		{
			linkedLabel->text = formatCurrentValue();
		}
	}

	void onChange() override
	{
		rack::Davies1900hSmallBlackSnapKnob::onChange();
		if (linkedLabel)
		{
			linkedLabel->text = formatCurrentValue();
		}
	}
private:
	std::string formatCurrentValue()const
	{
		return "1 / " + std::to_string(static_cast<unsigned int>(value));
	}
private:
	rack::Label* linkedLabel = nullptr;
};

namespace Helpers
{
	template <class InputPortClass>
	static rack::Port* addInput(rack::ModuleWidget* const widget, rack::Module* module, int const inputId, rack::Vec& position, std::string const& label, float labelOffset, float* light)
	{
		auto* const port = rack::createInput<InputPortClass>(position, module, inputId);

		if (!label.empty())
		{
			rack::Label* const labelWidget = new rack::Label;

			labelWidget->box.pos = position;
			labelWidget->box.pos.x += labelOffset;
			labelWidget->text = label;
			widget->addChild(labelWidget);

			position.y += labelWidget->box.size.y;
		}

		port->box.pos = position;

		widget->addInput(port);
		return port;
	}
}

ClockDividerWidget::ClockDividerWidget()
{
	static constexpr float const Margin = 5.f;

	auto* const module = new ClockDivider;

	box.size = rack::Vec(15 * 8, 380);

	setModule(module);

	auto* const mainPanel = new rack::LightPanel;

	mainPanel->box.size = box.size;
	addChild(mainPanel);

	// Setup input ports
	rack::Vec pos(Margin, Margin);

	auto* const resetInputWidget = Helpers::addInput<rack::PJ301MPort>(this, module, ClockDivider::INPUT_RESET, pos, "Reset", -10.f, nullptr);

	pos.x += Margin * 2.f + resetInputWidget->box.size.y;
	pos.y = Margin;

	auto* const clockInputWidget = Helpers::addInput<rack::PJ301MPort>(this, module, ClockDivider::INPUT_CLOCK, pos, "Clock", -10.f, nullptr);

	pos.x = Margin;
	pos.y += clockInputWidget->box.size.y + Margin * 2.f + 20.f;

	auto defaultDividerValue = 1u;

	// Setup clock outputs port and controls
	for (auto i = 0u; i < ClockDivider::NUM_OUTPUTS; ++i)
	{
		auto* clockControl = dynamic_cast<ClockDividerKnob*>(rack::createParam<ClockDividerKnob>(pos, module, i, 1.f, MaxDivider, defaultDividerValue));

		defaultDividerValue *= 2u;
		addParam(clockControl);

		pos.x += clockControl->box.size.x + Margin;

		auto* const portWidget = rack::createOutput<rack::PJ301MPort>(pos, module, i);

		rack::Vec lightPos = pos;

		lightPos.x += 20.f;
		lightPos.y += 2.f;

		auto* const light = rack::createValueLight<rack::TinyLight<rack::RedValueLight>>(lightPos, module->lightState(i));

		addOutput(portWidget);
		addChild(light);

		pos.x += portWidget->box.size.x;

		auto* const textWidget = new rack::Label;

		clockControl->connectLabel(textWidget);
		textWidget->box.pos = pos;

		addChild(textWidget);

		pos.x = Margin;
		pos.y += std::max({portWidget->box.size.y, clockControl->box.size.y, textWidget->box.size.y}) + Margin;
	}
	initialize();
}
