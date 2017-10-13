#if!defined SIMPLE_HPP
#define SIMPLE_HPP
#include <rack.hpp>
#include <utils/ExtendedModuleWidget.hpp>

extern rack::Plugin *plugin;

struct ButtonTriggerWidget : rack::ModuleWidget
{
	ButtonTriggerWidget();
};

struct ClockDividerWidget : rack::ModuleWidget
{
	ClockDividerWidget();
};

class Recorder;

struct RecorderWidget : ExtendedModuleWidget
{
	RecorderWidget();

private:
	void onArmButtonClicked();
	bool selectOutputFile();
private:
	Recorder* const m_recorder;
};

#endif
