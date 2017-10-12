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

	enum class Errors : std::uint8_t
	{
		NoError,
		BufferOverflow,
		UnableToOpenFile
	};

	static std::string getErrorText(Errors const error);

	WavRecorder();
	~WavRecorder();

	void start(std::string const& outputFilePath);
	void stop();
	bool isRunning()const { return m_running; }
	bool haveError()const { return m_error != Errors::NoError; }
	void clearError() { m_error = Errors::NoError; }
	Errors error()const { return m_error; }

	void push(Frame const& frame);
private:
	void run(std::string const outputFilePath);
private:
	std::vector<Frame> m_buffer;
	std::atomic_bool m_running;
	std::atomic<Errors> m_error;
	std::mutex m_mutexBuffer;
	std::thread m_thread;
};

#endif
