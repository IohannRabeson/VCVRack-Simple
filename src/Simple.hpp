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

class ClockDividerWidget : public ExtendedModuleWidget
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

class Clock;
class FourteenSegmentDisplay;

class ClockWidget : public ExtendedModuleWidget
{
public:
	ClockWidget();

	void step() override;
private:
	Clock* const m_clock;
	FourteenSegmentDisplay* const m_segmentDisplay;

};

#endif
