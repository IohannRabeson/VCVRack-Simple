#if!defined WAVRECORDER_HPP
#define WAVRECORDER_HPP
#include <dsp/ringbuffer.hpp>
#include <dsp/frame.hpp>
#include <thread>
#include <vector>
#include "write_wav.h"

class WavRecorder
{
	static constexpr unsigned int const ChannelCount = 2u;
public:
	using Frame = rack::Frame<ChannelCount>;

	WavRecorder();
	~WavRecorder();

	void start(std::string const& outputFilePath);
	void stop();
	bool isRunning()const { return m_running; }

	void push(Frame const& frame);
private:
	void run(std::string const outputFilePath);
private:
	std::vector<Frame> m_buffer;
	std::atomic_bool m_running;
	std::mutex m_mutexBuffer;
	std::thread m_thread;
};

#endif
