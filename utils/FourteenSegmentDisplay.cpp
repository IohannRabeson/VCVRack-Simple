#include "FourteenSegmentDisplay.hpp"
#include "src/Simple.hpp" // for plugin TODO: remove this relative reference

static constexpr float const HalfThickness = 0.05f;

static void drawSegment(NVGcontext* vg, NVGcolor const& color)
{
	nvgBeginPath(vg);
	nvgMoveTo(vg, 0.f, 0.f);
	nvgLineTo(vg, HalfThickness, HalfThickness);
	nvgLineTo(vg, HalfThickness, 2.f - HalfThickness);
	nvgLineTo(vg, 0.f, 2.f);
	nvgLineTo(vg, -HalfThickness, 2.f - HalfThickness);
	nvgLineTo(vg, -HalfThickness, HalfThickness);
	nvgClosePath(vg);
	nvgFillColor(vg, color);
	nvgFill(vg);
}

static void drawSmallSegment(NVGcontext* vg, NVGcolor const& color)
{
	nvgBeginPath(vg);
	nvgMoveTo(vg, 0.f, 0.f);
	nvgLineTo(vg, HalfThickness, HalfThickness);
	nvgLineTo(vg, HalfThickness, 1.f - HalfThickness);
	nvgLineTo(vg, 0.f, 1.f);
	nvgLineTo(vg, -HalfThickness, 1.f - HalfThickness);
	nvgLineTo(vg, -HalfThickness, HalfThickness);
	nvgClosePath(vg);
	nvgFillColor(vg, color);
	nvgFill(vg);
}

static void drawDiagonalSegment(NVGcontext* vg, NVGcolor const& color)
{
	/*
	nvgBeginPath(vg);
	nvgMoveTo(vg, HalfThickness, HalfThickness);
	nvgLineTo(vg, HalfThickness * 2.f, HalfThickness);
	nvgLineTo(vg, 1.f - HalfThickness, 1.f - HalfThickness);
	nvgLineTo(vg, 0.f, 1.f);
	nvgLineTo(vg, -HalfThickness, 1.f - HalfThickness);
	nvgLineTo(vg, -HalfThickness, HalfThickness);
	nvgClosePath(vg);
	nvgFillColor(vg, color);
	nvgFill(vg);
	*/

	nvgBeginPath(vg);
	nvgMoveTo(vg, 0.f, 0.f);
	nvgLineTo(vg, HalfThickness, HalfThickness);
	nvgLineTo(vg, HalfThickness, 1.f - HalfThickness);
	nvgLineTo(vg, 0.f, 1.f);
	nvgLineTo(vg, -HalfThickness, 1.f - HalfThickness);
	nvgLineTo(vg, -HalfThickness, HalfThickness);
	nvgClosePath(vg);
	nvgFillColor(vg, color);
	nvgFill(vg);
}

FourteenSegmentDisplay::FourteenSegmentDisplay(std::size_t maxCharacters) :
	m_font(rack::Font::load(rack::assetPlugin(plugin, "res/fonts/Sudo.ttf"))),
	m_maxCharacters(maxCharacters)
{
}

void FourteenSegmentDisplay::draw(NVGcontext* vg)
{
	nvgSave(vg);
	nvgSave(vg);
	nvgFillColor(vg, nvgRGBA(0x30, 0x33, 0x32, 0xFF));
	nvgBeginPath(vg);
	nvgRoundedRect(vg, box.pos.x, box.pos.y, box.size.x, 25.f, 2.5f);
	nvgFill(vg);
	nvgRestore(vg);
	nvgFillColor(vg, nvgRGBA(220,220,220,160));
	nvgFontSize(vg, 25.0f);
	nvgText(vg, box.pos.x, box.pos.y, m_displayedText.c_str(), nullptr);
	nvgRestore(vg);
}
