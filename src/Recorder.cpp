#include "rack.hpp"
#include "Simple.hpp"

#include <utils/WavWriter.hpp>
#include <utils/SimpleHelpers.hpp>
#include <utils/ExtendedButton.hpp>
#include <utils/LightControl.hpp>
#include <utils/StateMachine.hpp>

#include <dsp/digital.hpp>
#include <../ext/osdialog/osdialog.h>

#include <iostream> // DEBUG
#include <cstdlib>

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

	enum StateIds
	{
		INITIAL_STATE = 0u,
		ARMED_STATE,
		RECORD_STATE
	};

	Recorder() :
		rack::Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
	{
		m_stateMachine.addState(INITIAL_STATE, [this](StateMachine&){});
		m_stateMachine.addState(ARMED_STATE, [this](StateMachine& machine)
				{
					auto const& recordArmInput = inputs[INPUT_RECORD_ARM];
					auto const& startStopInput = inputs[INPUT_START_STOP];
					auto const armValue = params[PARAM_RECORD_ARM].value + getInputValue(recordArmInput);
					auto const startStopValue = params[PARAM_START_STOP].value + getInputValue(startStopInput);

					if (m_armTrigger.process(armValue))
					{
						machine.change(INITIAL_STATE);
						m_redLightControl.setState<LightControl::StateOff>();
						m_armState = false;
					}
					if (m_startStopTrigger.process(startStopValue))
					{
						machine.change(RECORD_STATE);
						m_redLightControl.setState<LightControl::StateOn>();
						startRecording();
					}
				});
		m_stateMachine.addState(RECORD_STATE, [this](StateMachine& machine)
				{
					auto const& startStopInput = inputs[INPUT_START_STOP];
					auto const startStopValue = params[PARAM_START_STOP].value + getInputValue(startStopInput);
					auto const& leftInput = inputs[INPUT_LEFT_IN];
					auto const& rightInput = inputs[INPUT_RIGHT_IN];

					if (m_startStopTrigger.process(startStopValue))
					{
						machine.change(ARMED_STATE);
						m_redLightControl.setState<LightControl::StateBlink>(0.5f, false);
						stopRecording();
					}

					WavWriter::Frame frame;

					frame.samples[0u] = getInputValue(leftInput);
					frame.samples[1u] = getInputValue(rightInput);
					m_writer.push(frame);
					if (m_writer.haveError())
					{
						// TODO: error notification	
						std::cerr << "Recorder error: " << WavWriter::getErrorText(m_writer.error()) << std::endl;
						m_writer.clearError();
					}
				});
		m_stateMachine.change(INITIAL_STATE);
	}

	void setOutputFilePath(std::string const& path)
	{
		m_outputFilePath = path;
		m_stateMachine.change(ARMED_STATE);
		m_redLightControl.setState<LightControl::StateBlink>(0.5f, true);
		m_armState = true;
	}

	bool isArmed()const
	{
		return m_armState;
	}

	bool isRecording()const
	{
		return m_writer.isRunning();
	}

	void startRecording()
	{
		m_writer.start(m_outputFilePath);
		std::cout << "Start recording..." << std::endl;
	}

	void stopRecording()
	{
		m_writer.stop();
		std::cout << "Stop recording..." << std::endl;
	}

	void onSampleRateChange() override
	{
		m_writer.stop();
		// TODO: error notification
		std::cerr << "Recorder error: the sample rate has changed during the recording" << std::endl;
	}

	void step() override
	{
		auto const& leftInput = inputs[INPUT_LEFT_IN];
		auto const& rightInput = inputs[INPUT_RIGHT_IN];

		m_stateMachine.step();
		m_redLightControl.step();
		m_vuMeterLeft = getInputValue(leftInput) / 10.f;
		m_vuMeterRight = getInputValue(rightInput) / 10.f;
	}

	float* vuMeterLeft() { return &m_vuMeterLeft; }
	float* vuMeterRight() { return &m_vuMeterRight; }
	float* redLight() { return m_redLightControl.lightValue(); }
private:
	WavWriter m_writer;
	StateMachine m_stateMachine;
	LightControl m_redLightControl;
	rack::SchmittTrigger m_startStopTrigger;
	rack::SchmittTrigger m_armTrigger;
	std::string m_outputFilePath;
	bool m_armState = false;
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

RecorderWidget::RecorderWidget() :
	m_recorder(new Recorder)
{
	static constexpr float const PortSize = 24.6146f;
	static constexpr float const Margin = 5.f;
	static constexpr float const Spacing = 10.f;
	static constexpr float const Width = 15.f * 6.f;
	static constexpr float const Height = 380.f;

	auto* const mainPanel = new rack::LightPanel;

	box.size = rack::Vec(Width, Height);
	mainPanel->box.size = box.size;
	addChild(mainPanel);
	setModule(m_recorder);
	{
		static constexpr float const Left = (Width - (PortSize * 2.f + Spacing)) / 2.f;
		static constexpr float const Top = 315;

		Helpers::addAudioInput<rack::PJ301MPort>(this, m_recorder, Recorder::INPUT_LEFT_IN, {Left, Top}, "L", m_recorder->vuMeterLeft());
		Helpers::addAudioInput<rack::PJ301MPort>(this, m_recorder, Recorder::INPUT_RIGHT_IN, {Left + Spacing + PortSize, Top}, "R", m_recorder->vuMeterRight());
	}
	auto* const armButton = createParam<ExtendedButton<rack::LEDButton>>({Margin, Margin}, Recorder::PARAM_RECORD_ARM, 0.f, 1.f, 0.f);

	armButton->setCallback(std::bind(&RecorderWidget::onArmButtonClicked, this));
	createParam<rack::LEDButton>({Margin, Margin + 30}, Recorder::PARAM_START_STOP, 0.f, 1.f, 0.f);
	addChild(rack::createValueLight<rack::SmallLight<rack::RedValueLight>>(rack::Vec{Margin, 80}, m_recorder->redLight()));
}

void RecorderWidget::onArmButtonClicked()
{
	if (!m_recorder->isArmed())
	{
		selectOutputFile();
	}
}

bool RecorderWidget::selectOutputFile()
{
	char* path = osdialog_file(OSDIALOG_SAVE, ".", "output.wav", nullptr);
	bool result = false;

	if (path)
	{
		m_recorder->setOutputFilePath(path);
		std::free(path);
		result = true;
	}
	return result;
}
