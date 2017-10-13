#if!defined PULSEGATE_HPP
#define PULSEGATE_HPP

class PulseGate
{
public:
	explicit PulseGate(float pulseDuration = 250.f);

	void reset();
	bool process(bool gate);
private:
	/*! The pulse duration in second. */
	float m_pulseDuration;
	float m_current = 0.f;
	bool m_pulse = false;
};

#endif
