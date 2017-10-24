#include "Simple.hpp"
#include <utils/FourteenSegmentDisplay.hpp>

class Clock : public rack::Module
{
};

ClockWidget::ClockWidget()
{
	auto* const clock = new Clock;
	auto* const mainPanel = new rack::LightPanel;
	auto* const displayTest = new FourteenSegmentDisplay;
	auto const Margin = 5.f;

	box.size = rack::Vec(15 * 6, 380);
	displayTest->box.pos = {Margin + 10, Margin + 5};
	displayTest->box.size = box.size.minus({Margin * 4.f, Margin * 4.f});
	mainPanel->box.size = box.size;
	addChild(mainPanel);
	addChild(displayTest);
	setModule(clock);
}
