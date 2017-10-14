#if!defined EXTENDEDMODULEWIDGET_HPP
#define EXTENDEDMODULEWIDGET_HPP
#include <app.hpp>

class ExtendedModuleWidget : public rack::ModuleWidget
{
public:
	template <class TInput>
	TInput* createInput(rack::Vec const& pos, int inputId)
	{
		assert( this->module != nullptr );

		TInput* const input = new TInput;

		input->box.pos = pos;
		input->module = this->module;
		input->type = rack::Port::INPUT;
		input->portId = inputId;
		rack::ModuleWidget::addInput(input);
		return input;
	}

	template <class TOutput>
	TOutput* createOutput(rack::Vec const& pos, int outputId)
	{
		assert( this->module != nullptr );

		TOutput* const output = new TOutput;

		output->box.pos = pos;
		output->module = module;
		output->type = rack::Port::OUTPUT;
		output->portId = outputId;
		rack::ModuleWidget::addOutput(output);
		return output;
	}

	template <class TParam>
	TParam* createParam(rack::Vec const& pos, int paramId, float minValue, float maxValue, float defaultValue)
	{
		assert( this->module != nullptr );

		TParam* const param = new TParam;

		param->box.pos = pos;
		param->module = this->module;
		param->paramId = paramId;
		param->setLimits(minValue, maxValue);
		param->setDefaultValue(defaultValue);
		rack::ModuleWidget::addParam(param);
		return param;
	}
};

#endif
