#include "Simple.hpp"

Plugin *plugin;

void init(rack::Plugin *p)
{
	plugin = p;
	plugin->slug = "Simple";
	plugin->name = "Simple";
	plugin->homepageUrl = "https://github.com/IohannRabeson/VCVRack-Simple";

	createModel<ButtonTriggerWidget>(plugin, "IO-ButtonTrigger", "Button Trigger");
	createModel<ClockDividerWidget>(plugin, "IO-ClockDivider", "Clock Divider");
}
