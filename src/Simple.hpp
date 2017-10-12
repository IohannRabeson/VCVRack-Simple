#if!defined SIMPLE_HPP
#define SIMPLE_HPP
#include "rack.hpp"

extern rack::Plugin *plugin;

struct ButtonTriggerWidget : rack::ModuleWidget
{
	ButtonTriggerWidget();
};

struct ClockDividerWidget : rack::ModuleWidget
{
	ClockDividerWidget();
};

struct RecorderWidget : rack::ModuleWidget
{
	RecorderWidget();
};

#endif
