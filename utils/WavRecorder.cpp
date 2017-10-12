#include "WavRecorder.hpp"
#include "write_wav.h"

#include <samplerate.h>
#include <engine.hpp>

#include <iostream>

std::string WavRecorder::getErrorText(Errors const error)
{
	std::string text;

	switch (error)
	{
	case Errors::BufferOverflow:
		text = "buffer overflow";
		break;
	case Errors::UnableToOpenFile:
		text = "unable to open file for writing";
		break;
	default:
		break;
	}
	return text;
}

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
	m_error = Errors::NoError;
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

	if (Audio_WAV_OpenWriter(&writer, outputFilePath.c_str(), rack::gSampleRate, ChannelCount) < 0)
	{
		m_error = Errors::UnableToOpenFile;
		return;
	}
	else
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

		if( sampleCount > buffer.size())
		{
			m_running = false;
			m_error = Errors::BufferOverflow;
			break;
		}

		src_float_to_short_array(m_buffer.data()->samples, buffer.data(), sampleCount);
		m_buffer.clear();
		lock.unlock();

		Audio_WAV_WriteShorts(&writer, buffer.data(), sampleCount);
	}
	Audio_WAV_CloseWriter(&writer);
	std::cout << "Closing file" << std::endl;
}
