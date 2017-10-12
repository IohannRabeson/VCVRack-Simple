#include "WavRecorder.hpp"
#include "write_wav.h"

#include <samplerate.h>
#include <engine.hpp>

WavRecorder::WavRecorder()
{
	m_running = false;
	m_buffer.reserve(rack::gSampleRate);
}

WavRecorder::~WavRecorder()
{
	stop();
}

void WavRecorder::start(std::string const& outputFilePath)
{
	m_thread = std::thread(&WavRecorder::run, this, outputFilePath);
}

void WavRecorder::stop()
{
	if (m_running)
	{
		m_running = false;
		m_thread.join();
	}
}

void WavRecorder::push(Frame const& frame)
{
	std::unique_lock<std::mutex> lock(m_mutexBuffer);

	m_buffer.push_back(frame);
	++m_bufferSize;
}

void WavRecorder::run(std::string const outputFilePath)
{
	static std::chrono::milliseconds const WriteTimeInterval{250u};
	std::vector<short> buffer(rack::gSampleRate * 2, 0);
	WAV_Writer writer;

	if (Audio_WAV_OpenWriter(&writer, outputFilePath.c_str(), rack::gSampleRate, ChannelCount) >= 0)
	{
		m_running = true;
	}
	while (m_running)
	{
		if (m_bufferSize > rack::gSampleRate / 2)
		{
			std::unique_lock<std::mutex> lock(m_mutexBuffer);
			auto const frameCount = m_buffer.size();

			assert( buffer.size() >= frameCount * ChannelCount );
			src_float_to_short_array(m_buffer.data()->samples,
					buffer.data(),
					frameCount * ChannelCount);
			m_buffer.clear();
			m_bufferSize = 0u;
			lock.unlock();

			Audio_WAV_WriteShorts(&writer, buffer.data(), frameCount * ChannelCount);
			std::this_thread::sleep_for(WriteTimeInterval);
		}
	}
	Audio_WAV_CloseWriter(&writer);
}
