#include "rack.hpp"
#include "Simple.hpp"

#include <utils/WavWriter.hpp>
#include <utils/SimpleHelpers.hpp>
#include <utils/ExtendedButton.hpp>
#include <utils/LightControl.hpp>
#include <utils/StateMachine.hpp>
#include <utils/Memory.hpp>
#include <utils/Path.hpp>

#include <dsp/digital.hpp>
#include <../ext/osdialog/osdialog.h>

#include <iostream> // DEBUG
#include <cstdlib>
#include <array>
#include <cmath>

static constexpr std::size_t const VuMeterCount = 16u;

class VuMeter
{
public:

	void setValue(float value)
	{
		auto const rounded = std::min(VuMeterCount, static_cast<std::size_t>(std::abs(value * static_cast<float>(VuMeterCount) / 10.f)));
		auto i = 0u;

		while (i < rounded)
		{
			m_values[i] = 1.f;
			++i;
		}
		while (i < m_values.size())
		{
			m_values[i] = 0.f;
			++i;
		}
	}

	float* lightValue(std::size_t i)
	{
		return m_values.data() + (VuMeterCount - 1u - i);
	}
private:
	std::array<float, VuMeterCount> m_values;
};

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
		PARAM_SELECT_FILE,
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
		m_stateMachine.addState(INITIAL_STATE, [this](StateMachine& machine)
				{
					auto const armValue = params[PARAM_RECORD_ARM].value;

					if (hasOutputFilePath() && m_armTrigger.process(armValue))
					{
						machine.change(ARMED_STATE);
					}
				});
		m_stateMachine.addStateBegin(INITIAL_STATE, [this]()
				{
					m_redLightControl.setState<LightControl::StateOff>();
				});
		m_stateMachine.addState(ARMED_STATE, [this](StateMachine& machine)
				{
					auto const& startStopInput = inputs[INPUT_START_STOP];
					auto const armValue = params[PARAM_RECORD_ARM].value;
					auto const startStopValue = params[PARAM_START_STOP].value + getInputValue(startStopInput);

					if (m_armTrigger.process(armValue))
					{
						machine.change(INITIAL_STATE);
					}
					if (m_startStopTrigger.process(startStopValue))
					{
						machine.change(RECORD_STATE);
						outputs[OUTPUT_START_STOP].value = 1.f;
					}
				});
		m_stateMachine.addStateBegin(ARMED_STATE, [this]()
				{
					m_redLightControl.setState<LightControl::StateBlink>(0.5f, false);
				});
		m_stateMachine.addState(RECORD_STATE, [this](StateMachine& machine)
				{
					auto const& startStopInput = inputs[INPUT_START_STOP];
					auto const startStopValue = params[PARAM_START_STOP].value + getInputValue(startStopInput);
					auto const& leftInput = inputs[INPUT_LEFT_IN];
					auto const& rightInput = inputs[INPUT_RIGHT_IN];

					if (m_startStopTrigger.process(startStopValue))
					{
						machine.change(INITIAL_STATE);
						outputs[OUTPUT_START_STOP].value = 1.f;
					}

					WavWriter::Frame frame;

					frame.samples[0u] = getInputValue(leftInput) / 10.f;
					frame.samples[1u] = getInputValue(rightInput) / 10.f;
					m_writer.push(frame);
					if (m_writer.haveError())
					{
						// TODO: error notification
						std::cerr << "Recorder error: " << WavWriter::getErrorText(m_writer.error()) << std::endl;
						m_writer.clearError();
						machine.change(INITIAL_STATE);
					}
				});
		m_stateMachine.addStateBegin(RECORD_STATE, [this]()
				{
					startRecording();
					m_redLightControl.setState<LightControl::StateOn>();
				});
		m_stateMachine.addStateEnd(RECORD_STATE, [this]()
				{
					stopRecording();
				});
		m_stateMachine.change(INITIAL_STATE);
	}

	void setOutputFilePath(std::string const& path)
	{
		m_outputFilePath = path;
	}

	bool hasOutputFilePath()const
	{
		return !m_outputFilePath.empty();
	}

	bool isArmed()const
	{
		return m_stateMachine.currentIndex() == ARMED_STATE;
	}

	bool isRecording()const
	{
		return m_writer.isRunning();
	}

	void startRecording()
	{
		m_writer.start(m_outputFilePath);
	}

	void stopRecording()
	{
		m_writer.stop();
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

		outputs[OUTPUT_START_STOP].value = 0.f;
		m_stateMachine.step();
		m_redLightControl.step();
		m_vuMeterLeft = getInputValue(leftInput) / 10.f;
		m_vuMeterRight = getInputValue(rightInput) / 10.f;
		m_leftVuMeter.setValue(getInputValue(leftInput));
		m_rightVuMeter.setValue(getInputValue(rightInput));
		m_fileLight = m_outputFilePath.empty() ? 0.f : 1.f;
	}

	float* vuMeterLeft() { return &m_vuMeterLeft; }
	float* vuMeterRight() { return &m_vuMeterRight; }
	float* redLight() { return m_redLightControl.lightValue(); }
	float* fileLight() { return &m_fileLight; }
	VuMeter& leftVuMeter() { return m_leftVuMeter; }
	VuMeter& rightVuMeter() { return m_rightVuMeter; }
private:
	WavWriter m_writer;
	StateMachine m_stateMachine;
	LightControl m_redLightControl;
	rack::SchmittTrigger m_startStopTrigger;
	rack::SchmittTrigger m_armTrigger;
	VuMeter m_leftVuMeter;
	VuMeter m_rightVuMeter;
	std::string m_outputFilePath;
	float m_vuMeterLeft = 0.f;
	float m_vuMeterRight = 0.f;
	float m_fileLight = 0.f;
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
	m_recorder(new Recorder),
	m_label(new rack::Label)
{
	static constexpr float const PortSize = 24.6146f;
	static constexpr float const Spacing = 10.f;
	static constexpr float const Width = 15.f * 6.f;

	auto* const mainPanel = new rack::SVGPanel;

	box.size = rack::Vec(15 * 6, 380);
	mainPanel->box.size = box.size;
	mainPanel->setBackground(rack::SVG::load(rack::assetPlugin(plugin, "res/recorder.svg")));
	addChild(mainPanel);

	addChild(rack::createScrew<rack::ScrewSilver>({15, 0}));
	addChild(rack::createScrew<rack::ScrewSilver>({box.size.x - 30, 0}));
	addChild(rack::createScrew<rack::ScrewSilver>({15, box.size.y - 15}));
	addChild(rack::createScrew<rack::ScrewSilver>({box.size.x - 30, box.size.y - 15}));

	setModule(m_recorder);
	{
		static constexpr float const Left = (Width - (PortSize * 2.f + Spacing)) / 2.f;

		Helpers::addAudioInput<rack::PJ301MPort>(this, m_recorder, Recorder::INPUT_LEFT_IN, {Left, 315}, "L", m_recorder->vuMeterLeft());
		Helpers::addAudioInput<rack::PJ301MPort>(this, m_recorder, Recorder::INPUT_RIGHT_IN, {Left + Spacing + PortSize, 315}, "R", m_recorder->vuMeterRight());
	}

	static constexpr float const Top = 90;

	auto* const selectFileButton = createParam<ExtendedButton<rack::LEDButton>>({10, Top - 30}, Recorder::PARAM_SELECT_FILE, 0.f, 1.f, 0.f);

	createParam<rack::LEDButton>({10, Top}, Recorder::PARAM_RECORD_ARM, 0.f, 1.f, 0.f);
	createParam<rack::LEDButton>({40, Top}, Recorder::PARAM_START_STOP, 0.f, 1.f, 0.f);
	createInput<rack::PJ301MPort>({37, Top + 25}, Recorder::INPUT_START_STOP);
	createOutput<rack::PJ301MPort>({37, Top + 55}, Recorder::OUTPUT_START_STOP);

	m_label->text = "<none>";
	m_label->box.pos.x = 22;
	m_label->box.pos.y = Top - 32;
	selectFileButton->setCallback(std::bind(&RecorderWidget::onSelectFileButtonClicked, this));

	addChild(rack::createValueLight<rack::SmallLight<rack::RedValueLight>>(rack::Vec{68, Top + 6}, m_recorder->redLight()));
	addChild(rack::createValueLight<rack::TinyLight<rack::GreenValueLight>>(rack::Vec{16.5f, Top - 23.5f}, m_recorder->fileLight()));
	addChild(m_label);

	static constexpr float const SmallLightSize = 8.f;
	static constexpr float const LeftLightPosX = (Width - (SmallLightSize * 2.f + 25.f)) / 2.f;
	static constexpr float const RightLightPosX = Width - LeftLightPosX - 5.f;

	float lightPosY = 180;
	auto& leftVuMeter = m_recorder->leftVuMeter();
	auto& rightVuMeter = m_recorder->rightVuMeter();
	for (auto i = 0u; i < VuMeterCount; ++i)
	{
		addChild(rack::createValueLight<rack::TinyLight<rack::RedValueLight>>(rack::Vec{LeftLightPosX, lightPosY}, leftVuMeter.lightValue(i)));
		addChild(rack::createValueLight<rack::TinyLight<rack::RedValueLight>>(rack::Vec{RightLightPosX, lightPosY}, rightVuMeter.lightValue(i)));
		lightPosY += SmallLightSize;
	}
}

void RecorderWidget::onSelectFileButtonClicked()
{
	if (!m_recorder->isArmed())
	{
		selectOutputFile();
	}
}

bool RecorderWidget::selectOutputFile()
{
	std::unique_ptr<char[], FreeDeleter<char>> path{osdialog_file(OSDIALOG_SAVE, ".", "output.wav", nullptr)};
	bool result = false;

	if (path)
	{
		std::string pathStr{path.get()};

		if (Path::extractExtension(pathStr).empty())
		{
			pathStr.append(".wav");
		}
		setOutputFilePath(pathStr);
		result = true;
	}
	else
	{
		setOutputFilePath("<none>");
	}
	glfwFocusWindow(rack::gWindow);
	return result;
}

void RecorderWidget::setOutputFilePath(std::string const& outputFilePath)
{
	m_recorder->setOutputFilePath(outputFilePath);
	m_label->text = Path::extractFileName(outputFilePath);
}
