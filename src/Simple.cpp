#include "Simple.hpp"

rack::Plugin* plugin;

void init(rack::Plugin *p)
{
	plugin = p;
	plugin->slug = "IO-Simple";

#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->addModel(rack::createModel<ButtonTriggerWidget>("IO-Simple", "Simple", "IO-ButtonTrigger", "Button Trigger"));
	p->addModel(rack::createModel<ClockDividerWidget>("IO-Simple", "Simple", "IO-ClockDivider", "Clock Divider"));
	p->addModel(rack::createModel<RecorderWidget>("IO-Simple", "Simple", "IO-Recorder", "Recorder"));
	p->addModel(rack::createModel<ClockWidget>("IO-Simple", "Simple", "IO-Clock", "Clock"));
}
