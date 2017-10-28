#include "ClockWidget.hpp"
#include "Clock.hpp"

#include <utils/FourteenSegmentDisplay.hpp>

ClockWidget::ClockWidget() :
	m_clock(new Clock),
	m_segmentDisplay(new FourteenSegmentDisplay)
{

	auto* const mainPanel = new rack::LightPanel;
	auto const Margin = 5.f;

	box.size = rack::Vec(15 * 6, 380);
	m_segmentDisplay->box.pos = {25.f, 5.f};
	m_segmentDisplay->box.size = {15 * 6 - Margin - 25.f, 25.f};
	m_segmentDisplay->setFontSize(10.f);
	mainPanel->box.size = box.size;
	setModule(m_clock);
	addChild(mainPanel);
	addChild(m_segmentDisplay);

	auto* const port = createOutput<rack::PJ301MPort>({}, Clock::OUTPUT_MAIN_CLOCK);
	auto* const knob = createParam<rack::RoundBlackKnob>({}, Clock::PARAM_VALUE, 0.0001f, 1.f, 0.5f);
	createParam<rack::LEDButton>({5.f, 8.f}, Clock::PARAM_CHANGE_MODE, 0.f, 1.f, 0.f);

	port->box.pos.x = (15.f * 6.f - port->box.size.x) / 2.f;
	port->box.pos.y = 50.f;
	knob->box.pos.x = (15.f * 6.f - knob->box.size.x) / 2.f;
	knob->box.pos.y = port->box.pos.y + 50.f;
}

void ClockWidget::step()
{
	ExtendedModuleWidget::step();
	m_segmentDisplay->setText(m_clock->getCurrentText());
}
