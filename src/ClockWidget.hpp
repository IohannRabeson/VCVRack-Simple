#if!defined CLOCKWIDGET_HPP
#define CLOCKWIDGET_HPP
#include <utils/ExtendedModuleWidget.hpp>

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
