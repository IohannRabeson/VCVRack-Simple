#include "WavRecorder.hpp"
#include "write_wav.h"

#include <samplerate.h>
#include <engine.hpp>

#include <iostream>

WavRecorder::WavRecorder()
{
	m_running = false;
	// The buffer can store 1 seconds
	m_buffer.reserve(rack::gSampleRate);
}

WavRecorder::~WavRecorder()
{
	stop();
	if (m_thread.joinable())
		m_thread.join();
}

void WavRecorder::start(std::string const& outputFilePath)
{
	if (m_thread.joinable())
		m_thread.join();
	m_buffer.clear();
	m_thread = std::thread(&WavRecorder::run, this, outputFilePath);
}

void WavRecorder::stop()
{
	if (m_running)
	{
		m_running = false;
	}
}

void WavRecorder::push(Frame const& frame)
{
	std::unique_lock<std::mutex> lock(m_mutexBuffer);

	m_buffer.push_back(frame);
}

void WavRecorder::run(std::string const outputFilePath)
{
	static std::chrono::milliseconds const WriteTimeInterval{500};
	std::vector<short> buffer(rack::gSampleRate * ChannelCount, 0);
	WAV_Writer writer;

	if (Audio_WAV_OpenWriter(&writer, outputFilePath.c_str(), rack::gSampleRate, ChannelCount) >= 0)
	{
		std::cout << "Creating file '" << outputFilePath << "'" << std::endl;
		m_running = true;
	}
	while (m_running)
	{
		std::this_thread::sleep_for(WriteTimeInterval);
		std::unique_lock<std::mutex> lock(m_mutexBuffer);
		auto const frameCount = m_buffer.size();
		auto const sampleCount = frameCount * ChannelCount;

		assert( buffer.size() >= sampleCount );

		src_float_to_short_array(m_buffer.data()->samples, buffer.data(), sampleCount);
		m_buffer.clear();
		lock.unlock();

		std::cout << "Writing " << frameCount << " frames" << std::endl;
		Audio_WAV_WriteShorts(&writer, buffer.data(), sampleCount);
	}
	Audio_WAV_CloseWriter(&writer);
	std::cout << "Closing file" << std::endl;
}
