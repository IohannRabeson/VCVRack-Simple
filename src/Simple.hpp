#if!defined SIMPLE_HPP
#define SIMPLE_HPP
#include "rack.hpp"

using namespace rack;

extern Plugin *plugin;

struct ButtonTriggerWidget : ModuleWidget
{
	ButtonTriggerWidget();
};

struct ClockDividerWidget : ModuleWidget
{
	ClockDividerWidget();
};

#endif
