#if!defined SIMPLE_HPP
#define SIMPLE_HPP
#include <rack.hpp>
#include <widgets.hpp>

#include <utils/ExtendedModuleWidget.hpp>

extern rack::Plugin *plugin;

class ButtonTriggerWidget : public rack::ModuleWidget
{
public:
	ButtonTriggerWidget();
};

class ClockDividerWidget : public rack::ModuleWidget
{
public:
	ClockDividerWidget();
};

class Recorder;

class RecorderWidget : public ExtendedModuleWidget
{
public:
	RecorderWidget();
private:
	void onSelectFileButtonClicked();
	bool selectOutputFile();
	void setOutputFilePath(std::string const& outputFilePath);
private:
	Recorder* const m_recorder;
	rack::Label* const m_label;
};

#endif
