#include "rack.hpp"
#include "Simple.hpp"

#include <utils/WavWriter.hpp>
#include <utils/SimpleHelpers.hpp>
#include <dsp/digital.hpp>

#include <iostream> // DEBUG

class Recorder : public rack::Module
{
public:
	enum InputIds
	{
		INPUT_LEFT_IN = 0,
		INPUT_RIGHT_IN,
		INPUT_START_STOP,
		INPUT_RECORD_ARM,
		NUM_INPUTS
	};

	enum ParamIds
	{
		PARAM_RECORD_ARM = 0,
		PARAM_START_STOP,
		PARAM_INPUT_VOLUME,
		NUM_PARAMS
	};

	enum OutputIds
	{
		OUTPUT_START_STOP,
		OUTPUT_RECORD_ARM,
		NUM_OUTPUTS
	};

	Recorder() :
		rack::Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
	{
	}

	void startRecording()
	{
		m_writer.start(m_outputFilePath);
	}

	void stopRecording()
	{
		m_writer.stop();
	}

	void step() override
	{
		auto const& leftInput = inputs[INPUT_LEFT_IN];
		auto const& rightInput = inputs[INPUT_RIGHT_IN];
		auto const& startStopInput = inputs[INPUT_START_STOP];
		auto const startStopValue = params[PARAM_START_STOP].value + getInputValue(startStopInput);

		m_vuMeterLeft = getInputValue(leftInput) / 10.f;
		m_vuMeterRight = getInputValue(rightInput) / 10.f;
		if (m_startStopTrigger.process(startStopValue))
		{
			if (m_writer.isRunning())
			{
				stopRecording();
			}
			else
			{
				startRecording();
			}
		}
		if (m_writer.isRunning())
		{
			WavWriter::Frame frame;

			frame.samples[0u] = getInputValue(leftInput);
			frame.samples[1u] = getInputValue(rightInput);
			m_writer.push(frame);
		}
		if (m_writer.haveError())
		{
			// TODO: error notification	
			std::cerr << "Recorder error: " << WavWriter::getErrorText(m_writer.error()) << std::endl;
			m_writer.clearError();
		}
	}

	float* vuMeterLeft() { return &m_vuMeterLeft; }
	float* vuMeterRight() { return &m_vuMeterRight; }
private:
	WavWriter m_writer;
	rack::SchmittTrigger m_startStopTrigger;
	std::string m_outputFilePath = "yolo.wav";
	float m_vuMeterLeft = 0.f;
	float m_vuMeterRight = 0.f;
};

namespace Helpers
{
	template <class InputPortClass>
	static rack::Port* addAudioInput(rack::ModuleWidget* const widget, rack::Module* const module,
								 	 int const inputId, rack::Vec const& position,
								 	 std::string const& label, float* lightValue)
	{
		auto* const port = rack::createInput<InputPortClass>(position, module, inputId);
		auto* const labelWidget = new rack::Label;
		auto* const light = rack::createValueLight<rack::SmallLight<rack::GreenValueLight>>(position, lightValue);

		labelWidget->text = label;
		widget->addInput(port);
		widget->addChild(labelWidget);
		widget->addChild(light);

		float const portSize = port->box.size.x;

		labelWidget->box.pos.x = position.x - 4.f;
		labelWidget->box.pos.y = position.y + portSize;
		light->box.pos.x = position.x + 14.f;
		light->box.pos.y = position.y + portSize + 6.f;
		return port;
	}
}

RecorderWidget::RecorderWidget()
{
	static constexpr float const PortSize = 24.6146f;
	static constexpr float const Margin = 5.f;
	static constexpr float const Spacing = 10.f;
	static constexpr float const Width = 15.f * 6.f;
	static constexpr float const Height = 380.f;

	auto* const module = new Recorder;
	auto* const mainPanel = new rack::LightPanel;

	box.size = rack::Vec(Width, Height);
	mainPanel->box.size = box.size;
	addChild(mainPanel);
	setModule(module);
	{
		static constexpr float const Left = (Width - (PortSize * 2.f + Spacing)) / 2.f;
		static constexpr float const Top = Margin + 25;

		Helpers::addAudioInput<rack::PJ301MPort>(this, module, Recorder::INPUT_LEFT_IN, rack::Vec{Left, Top}, "L", module->vuMeterLeft());
		Helpers::addAudioInput<rack::PJ301MPort>(this, module, Recorder::INPUT_RIGHT_IN, rack::Vec{Left + Spacing + PortSize, Top}, "R", module->vuMeterRight());
	}
	addParam(rack::createParam<rack::CKD6>(rack::Vec(Margin, Margin), module, Recorder::PARAM_START_STOP, 0.f, 1.f, 0.f));
}
