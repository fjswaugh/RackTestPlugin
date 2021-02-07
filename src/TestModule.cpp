#include "plugin.hpp"
#include <array>
#include <cmath>

struct TestModule : rack::Module
{
    enum ParamIds
    {
        PitchParam,
        NumParams,
    };
    enum InputIds
    {
        PitchInput,
        NumInputs,
    };
    enum OutputIds
    {
        SineOutput,
        NumOutputs,
    };
    enum LightIds
    {
        BlinkLight,
        NumLights,
    };

    TestModule()
    {
        config(NumParams, NumInputs, NumOutputs, NumLights);
        configParam(PitchParam, 0.0f, 1.0f, 0.0f, "");
    }

    void process(ProcessArgs const& args) override
    {
        // Compute the frequency from the pitch parameter and input
        float pitch = params[PitchParam].getValue();
        pitch += inputs[PitchInput].getVoltage();
        pitch = rack::clamp(pitch, -4.0f, 4.0f);

        // The default pitch is C4 = 261.6256f
        float const freq = rack::dsp::FREQ_C4 * std::pow(2.0f, pitch);

        // Accumulate the phase
        phase_ += freq * args.sampleTime;
        if (phase_ >= 0.5f)
            phase_ -= 1.0f;

        // Compute the sine output
        float const sine = 0.5f * std::sin(2.0f * M_PI * phase_);

        // Audio signals are typically +/-5V
        outputs[SineOutput].setVoltage(5.0f * sine);

        // Blink light at 1Hz
        blink_phase_ += args.sampleTime;
        if (blink_phase_ >= 1.0f)
            blink_phase_ -= 1.0f;
        lights[BlinkLight].setBrightness(blink_phase_ < 0.5f ? 1.0f : 0.0f);
    }

private:
    float phase_{};
    float blink_phase_{};
};

struct TestModuleWidget : rack::ModuleWidget
{
    explicit TestModuleWidget(TestModule* module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(rack::asset::plugin(gPlugin, "res/TestModule.svg")));

        addScrewWidgets();
        addControls();
    }

private:
    void addScrewWidgets()
    {
        auto const h = rack::RACK_GRID_HEIGHT;
        auto const w = rack::RACK_GRID_WIDTH;

        auto const positions_of_screws = std::array{
            rack::Vec{h, 0},
            rack::Vec{box.size.x - 2 * w, 0},
            rack::Vec{w, h - w},
            rack::Vec{box.size.x - 2 * w, h - w},
        };

        for (auto const& pos : positions_of_screws)
            addChild(rack::createWidget<rack::ScrewSilver>(pos));
    }

    void addControls()
    {
        auto const pos_pitch_knob = rack::mm2px(rack::Vec(15.24, 46.063));
        auto const pos_pitch_in = rack::mm2px(rack::Vec(15.24, 77.478));
        auto const pos_pitch_out = rack::mm2px(rack::Vec(15.24, 108.713));
        auto const pos_blink_light = rack::mm2px(rack::Vec(15.24, 25.81));

        addParam(rack::createParamCentered<rack::RoundBlackKnob>(pos_pitch_knob, module, TestModule::PitchParam));
        addInput(rack::createInputCentered<rack::PJ301MPort>(pos_pitch_in, module, TestModule::PitchInput));
        addOutput(rack::createOutputCentered<rack::PJ301MPort>(pos_pitch_out, module, TestModule::SineOutput));
        addChild(rack::createLightCentered<rack::MediumLight<rack::RedLight>>(
            pos_blink_light, module, TestModule::BlinkLight));
    }
};

rack::Model* gTestModel = rack::createModel<TestModule, TestModuleWidget>("TestModule");
