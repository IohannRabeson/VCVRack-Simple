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
		OUTPUT_RESET,
		OUTPUT_CLOCK,
		NUM_OUTPUTS
	};

	static constexpr unsigned int const ClockDividerCount = 8u;

	ClockDivider() :
		Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
	{
		auto initClockDivider = 1u;

		for (auto i = 0u; i < ClockDividerCount; ++i)
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

		outputs.at(OUTPUT_CLOCK).value = inputClock.value;
		outputs.at(OUTPUT_RESET).value = inputReset.value;
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
		return "/" + std::to_string(static_cast<unsigned int>(value));
	}
private:
	rack::Label* linkedLabel = nullptr;
};

namespace Helpers
{
	template <class InputPortClass>
	static rack::Port* addInput(rack::ModuleWidget* const widget, rack::Module* module, int const inputId,
								rack::Vec const& position, std::string const& label, float labelOffset,
								float* light)
	{
		auto* const port = rack::createInput<InputPortClass>(position, module, inputId);

		if (!label.empty())
		{
			rack::Label* const labelWidget = new rack::Label;

			labelWidget->box.pos = position;
			labelWidget->box.pos.x += labelOffset;
			labelWidget->text = label;
			widget->addChild(labelWidget);

//			position.y += labelWidget->box.size.y;
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

	auto* const mainPanel = new rack::SVGPanel;

	mainPanel->box.size = box.size;
	mainPanel->setBackground(rack::SVG::load("plugins/VCVRack-Simple/res/clock_divider.svg"));
	addChild(mainPanel);

	addChild(rack::createScrew<rack::ScrewSilver>({15, 0}));
	addChild(rack::createScrew<rack::ScrewSilver>({box.size.x - 30, 0}));
	addChild(rack::createScrew<rack::ScrewSilver>({15, box.size.y - 15}));
	addChild(rack::createScrew<rack::ScrewSilver>({box.size.x - 30, box.size.y - 15}));

	// Setup input ports
	addInput(rack::createInput<rack::PJ301MPort>({30, 45}, module, ClockDivider::INPUT_RESET));
	addOutput(rack::createOutput<rack::PJ301MPort>({30, 90}, module, ClockDivider::OUTPUT_RESET));

	addInput(rack::createInput<rack::PJ301MPort>({70, 45}, module, ClockDivider::INPUT_CLOCK));
	addOutput(rack::createOutput<rack::PJ301MPort>({70, 90}, module, ClockDivider::OUTPUT_CLOCK));

	auto defaultDividerValue = 1u;
	auto const left = 10;
	rack::Vec pos{left, 125};

	// Setup clock outputs port and controls
	for (auto i = 0u; i < 8u; ++i)
	{
		auto* clockControl = dynamic_cast<ClockDividerKnob*>(rack::createParam<ClockDividerKnob>(pos, module, ClockDivider::OUTPUT_CLOCK_0 + i, 1.f, MaxDivider, defaultDividerValue));

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

		clockControl->box.size = portWidget->box.size;
		clockControl->connectLabel(textWidget);
		textWidget->box.pos = pos;
		textWidget->box.pos.y += 2;

		addChild(textWidget);

		pos.x = left;
		pos.y += 30;
	}
	initialize();
}
