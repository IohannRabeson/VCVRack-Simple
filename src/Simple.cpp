#include "Simple.hpp"

#include "ClockWidget.hpp"

rack::Plugin* plugin;

void init(rack::Plugin *p)
{
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);
    p->website = "https://iohannrabeson.github.io/VCVRack-Simple/";

	p->addModel(modelButtonTrigger);
	p->addModel(modelClockDivider);
	p->addModel(modelClock);
}
