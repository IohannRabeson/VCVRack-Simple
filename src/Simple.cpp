#include "Simple.hpp"

rack::Plugin* plugin;

void init(rack::Plugin *p)
{
	plugin = p;
	plugin->slug = "Simple";
	plugin->name = "Simple";
	plugin->homepageUrl = "https://github.com/IohannRabeson/VCVRack-Simple";

	rack::createModel<ButtonTriggerWidget>(plugin, "IO-ButtonTrigger", "Button Trigger");
	rack::createModel<ClockDividerWidget>(plugin, "IO-ClockDivider", "Clock Divider");
	rack::createModel<RecorderWidget>(plugin, "IO-Recorder", "Recorder");
}
