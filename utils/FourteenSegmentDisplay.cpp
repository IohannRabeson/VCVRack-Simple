#include "FourteenSegmentDisplay.hpp"
#include "src/Simple.hpp" // for plugin TODO: remove this relative reference

FourteenSegmentDisplay::FourteenSegmentDisplay(std::size_t maxCharacters) :
	m_font(rack::Font::load(rack::assetPlugin(plugin, "res/fonts/Sudo.ttf"))),
	m_maxCharacters(maxCharacters)
{
}

void FourteenSegmentDisplay::draw(NVGcontext* vg)
{
	nvgSave(vg);
	nvgFillColor(vg, nvgRGBA(0x30, 0x33, 0x32, 0xFF));
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0.f, 0.f, box.size.x, box.size.y, 2.5f);
	nvgFill(vg);
	nvgFillColor(vg, nvgRGBA(220,220,220,160));
	nvgFontSize(vg, m_fontSize);
	nvgText(vg, 2.f, box.size.y - (box.size.y - m_fontSize), m_displayedText.c_str(), nullptr);
	nvgRestore(vg);
}

void FourteenSegmentDisplay::setText(std::string const& text)
{
	m_displayedText = text;
}
