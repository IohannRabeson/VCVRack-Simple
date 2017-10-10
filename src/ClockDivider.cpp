#include "Simple.hpp"
#include <array>
#include <algorithm>
#include <iostream>

namespace
{
	class ClockDividerImp
	{
	public:
		explicit ClockDividerImp(unsigned int index, unsigned int limit = 1u) :
			m_index(index),
			m_limit(limit)
		{
		}

		void setLimit(unsigned int limit){ m_limit = limit;	}

		void reset(){ m_current = 0u; }

		void process(bool const clockTrigger,
					 std::vector<Param> const& params,
					 std::vector<Output>& outputs)
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
					if (output.active)
					{
						output.value = 10.f;
					}
					m_current = 0u;
				}
			}
			else
			{
				output.value = 0.f;
			}
		}

	private:
		PulseGenerator m_pulseGenerator;
		unsigned int const m_index;
		unsigned int m_current = 0u;
		unsigned int m_limit = 1u;
	};
}

struct ClockDivider : Module
{
	enum ParamsIds
	{
		CLOCK_DIVIDER_0,
		CLOCK_DIVIDER_1,
		CLOCK_DIVIDER_2,
		CLOCK_DIVIDER_3,
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
		NUM_OUTPUTS
	};

	ClockDivider() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
	{
		m_clockDividers.emplace_back(OUTPUT_CLOCK_0, 1u);
		m_clockDividers.emplace_back(OUTPUT_CLOCK_1, 2u);
		m_clockDividers.emplace_back(OUTPUT_CLOCK_2, 4u);
		m_clockDividers.emplace_back(OUTPUT_CLOCK_3, 8u);
	}

	void step()
	{
		auto const& inputClock = inputs.at(INPUT_CLOCK);
		auto const& inputReset = inputs.at(INPUT_RESET);
		bool const clockTick = inputClock.active && m_clockTrigger.process(inputClock.value);

		if (inputReset.active && m_resetTrigger.process(inputReset.value))
		{
			std::for_each(m_clockDividers.begin(), m_clockDividers.end(), [](ClockDividerImp& imp) { imp.reset(); });
		}
		for (auto& divider : m_clockDividers)
		{
			divider.process(clockTick, params, outputs);
		}
	}
private:
	std::vector<ClockDividerImp> m_clockDividers;
	SchmittTrigger m_resetTrigger;
	SchmittTrigger m_clockTrigger;
};

struct ClockDividerKnob : Davies1900hSmallBlackSnapKnob
{
	Label* linkedLabel = nullptr;

	void onChange()
	{
		Davies1900hSmallBlackSnapKnob::onChange();
		if (linkedLabel)
		{
			linkedLabel->text = "1 / " + std::to_string(static_cast<unsigned int>(value));
		}
	}
};

ClockDividerWidget::ClockDividerWidget()
{
	static constexpr float const Margin = 5.f;
	ClockDivider* const module = new ClockDivider;

	box.size = Vec(15 * 8, 380);

	setModule(module);

	LightPanel* mainPanel = new LightPanel;
	mainPanel->box.size = box.size;
	addChild(mainPanel);

	Vec pos(Margin, Margin);

	auto* resetInputWidget = createInput<PJ301MPort>(pos, module, ClockDivider::INPUT_RESET);
	auto* clockInputWidget = createInput<PJ301MPort>(pos, module, ClockDivider::INPUT_CLOCK);

	addInput(resetInputWidget);
	pos.x += Margin + resetInputWidget->box.size.x;
	clockInputWidget->box.pos = pos;
	addInput(clockInputWidget);
	pos.x = Margin;
	pos.y += clockInputWidget->box.size.y + Margin;
	for (auto i = 0u; i < ClockDivider::NUM_OUTPUTS; ++i)
	{
		auto* clockControl = dynamic_cast<ClockDividerKnob*>(createParam<ClockDividerKnob>(pos, module, i, 1.f, 128.f, 1.f));

		addParam(clockControl);

		pos.x += clockControl->box.size.x + Margin;

		auto* portWidget = createOutput<PJ301MPort>(pos, module, i);

		addOutput(portWidget);

		pos.x += portWidget->box.size.x;

		auto* textWidget = new Label;

		clockControl->linkedLabel = textWidget;
		textWidget->box.pos = pos;

		addChild(textWidget);

		pos.x = Margin;
		pos.y += std::max({portWidget->box.size.y, clockControl->box.size.y, textWidget->box.size.y}) + Margin;
	}
}
