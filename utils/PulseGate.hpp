#if!defined PULSEGATE_HPP
#define PULSEGATE_HPP

class PulseGate
{
public:
	explicit PulseGate(unsigned int pulseDuration = 16u);

	void reset();
	bool process(bool gate);
private:
	/*! The pulse duration in frame. */
	unsigned int m_pulseDuration;
	unsigned int m_currentCount = 0u;
	bool m_pulse = false;
};

#endif
