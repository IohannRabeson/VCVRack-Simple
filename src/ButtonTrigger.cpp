#include "Simple.hpp"

struct ButtonTrigger : Module
{
	enum ParamIds
	{
		TRIGGER,
		NUM_PARAMS
	};

	enum OutputIds
	{
		OUTPUT_0,
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		NUM_OUTPUTS
	};

	ButtonTrigger() : Module(NUM_PARAMS, 0u, NUM_OUTPUTS){}

	void step();

private:
	SchmittTrigger m_buttonTrigger;
};

void ButtonTrigger::step()
{
	auto const buttonValue = params[TRIGGER].value;
	auto const schmittValue = m_buttonTrigger.process(buttonValue);

	for (auto& output : outputs)
	{
		if (output.active)
		{
			output.value = schmittValue;
		}
	}
}

ButtonTriggerWidget::ButtonTriggerWidget()
{
	static constexpr float const PanelWidth = 70.f;
	static constexpr float const Margin = 5.f;
	static constexpr float const ButtonSize = 28.f;
	static constexpr float const ConnectorSize = 30.f;

	ButtonTrigger* const module = new ButtonTrigger;

	box.size = Vec(15 * 4, 380);

	setModule(module);

	Panel* mainPanel = new LightPanel();
	mainPanel->box.size = box.size;
	addChild(mainPanel);

	addParam(createParam<CKD6>(Vec(Margin, Margin), module, ButtonTrigger::TRIGGER, 0.f, 1.f, 0.f));

	Vec pos(0, ButtonSize + Margin * 2.f);

	for (auto i = 0u; i < ButtonTrigger::NUM_OUTPUTS; ++i)
	{
		addOutput(createOutput<PJ3410Port>(pos, module, i));
		pos.y += ButtonSize + Margin;
	}
}
