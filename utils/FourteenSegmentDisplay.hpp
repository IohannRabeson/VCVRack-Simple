#if!defined FOURTEENSEGMENTDISPLAY_HPP
#define FOURTEENSEGMENTDISPLAY_HPP
#include <rack.hpp>

class FourteenSegmentDisplay : public rack::Widget
{
public:
	explicit FourteenSegmentDisplay(std::size_t maxCharacters = 2);

	void setText(std::string const& text);
	void draw(NVGcontext* vg) override;
private:
	std::shared_ptr<rack::Font> const m_font;
	std::size_t const m_maxCharacters;
	std::string m_displayedText;
	float m_fontSize = 20.f;
	NVGcolor m_onColor = nvgRGB(0, 0x0, 0);
	NVGcolor m_offColor = nvgRGB(0xFF, 0x0, 0x0);
};

#endif
