#include "rack.hpp"
#include "Simple.hpp"
#include "../utils/WavRecorder.hpp"

#include <iostream> // DEBUG

class Recorder : public rack::Module
{
public:
	enum InputIds
	{
		INPUT_LEFT_IN = 0,
		INPUT_RIGHT_IN,
		INPUT_START_STOP,
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
		OUTPUT_RECORD_START,
		NUM_OUTPUTS
	};

	Recorder() :
		rack::Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
	{
	}

	~Recorder()
	{
	}

	void startRecording()
	{
		m_recorder.start(m_outputFilePath);
	}

	void stopRecording()
	{
		m_recorder.stop();
	}

	void step() override
	{
		auto& leftInput = inputs[INPUT_LEFT_IN];
		auto& rightInput = inputs[INPUT_RIGHT_IN];
		auto const startStopValue = params[PARAM_START_STOP].value;

		m_vuMeterLeft = leftInput.active ? leftInput.value / 10.f : 0.f;
		m_vuMeterRight = rightInput.active ? rightInput.value / 10.f : 0.f;

		if (m_startStopTrigger.process(startStopValue))
		{
			if (m_recorder.isRunning())
			{
				stopRecording();
			}
			else
			{
				startRecording();
			}
		}
		if (m_recorder.isRunning())
		{
			WavRecorder::Frame frame;

			frame.samples[0u] = leftInput.value;
			frame.samples[1u] = rightInput.value;
			m_recorder.push(frame);
		}
	}

	float* vuMeterLeft() { return &m_vuMeterLeft; }
	float* vuMeterRight() { return &m_vuMeterRight; }
private:
	SchmittTrigger m_startStopTrigger;
	std::string m_outputFilePath = "yolo.wav";
	float m_vuMeterLeft = 0.f;
	float m_vuMeterRight = 0.f;
	WavRecorder m_recorder;
};

namespace Helpers
{
	template <class InputPortClass>
	static Port* addAudioInput(ModuleWidget* const widget, Module* const module,
								 int const inputId, Vec const& position,
								 std::string const& label, float* lightValue)
	{
		auto* const port = rack::createInput<InputPortClass>(position, module, inputId);
		auto* const labelWidget = new rack::Label;
		auto* const light = rack::createValueLight<SmallLight<GreenValueLight>>(position, lightValue);

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

	Recorder* const module = new Recorder;
	LightPanel* const mainPanel = new LightPanel;

	box.size = Vec(Width, Height);
	mainPanel->box.size = box.size;
	addChild(mainPanel);
	setModule(module);
	{
		static constexpr float const Left = (Width - (PortSize * 2.f + Spacing)) / 2.f;
		static constexpr float const Top = Margin + 25;

		Helpers::addAudioInput<rack::PJ301MPort>(this, module, Recorder::INPUT_LEFT_IN, Vec{Left, Top}, "L", module->vuMeterLeft());
		Helpers::addAudioInput<rack::PJ301MPort>(this, module, Recorder::INPUT_RIGHT_IN, Vec{Left + Spacing + PortSize, Top}, "R", module->vuMeterRight());
	}
	addParam(rack::createParam<rack::CKD6>(Vec(Margin, Margin), module, Recorder::PARAM_START_STOP, 0.f, 1.f, 0.f));
}
