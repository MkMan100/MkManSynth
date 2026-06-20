#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class MkManSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    MkManSynthAudioProcessorEditor (MkManSynthAudioProcessor&);
    ~MkManSynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MkManSynthAudioProcessor& audioProcessor;

    // Sliders e ComboBox per i 15 parametri
    juce::ComboBox osc1WaveMenu, osc2WaveMenu;
    juce::Slider oscMixSlider, osc1DetuneSlider, osc2DetuneSlider;
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Slider lfoRateSlider, lfoDepthSlider;
    juce::Slider cutoffSlider, qSlider, distDriveSlider, delayTimeSlider, delayFeedbackSlider;
    juce::Slider macroLeslieSlider;

    // Labels per i nomi dei controlli
    juce::Label osc1WaveLabel, osc2WaveLabel, oscMixLabel, osc1DetuneLabel, osc2DetuneLabel;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;
    juce::Label lfoRateLabel, lfoDepthLabel;
    juce::Label cutoffLabel, qLabel, distDriveLabel, delayTimeLabel, delayFeedbackLabel;
    juce::Label macroLeslieLabel;

    // Attachments per legare la grafica ai parametri reali dell'APVTS
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<ComboBoxAttachment> osc1WaveAttach, osc2WaveAttach;
    std::unique_ptr<SliderAttachment> oscMixAttach, osc1DetuneAttach, osc2DetuneAttach;
    std::unique_ptr<SliderAttachment> attackAttach, decayAttach, sustainAttach, releaseAttach;
    std::unique_ptr<SliderAttachment> lfoRateAttach, lfoDepthAttach;
    std::unique_ptr<SliderAttachment> cutoffAttach, qAttach, distDriveAttach, delayTimeAttach, delayFeedbackAttach;
    std::unique_ptr<SliderAttachment> macroLeslieAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MkManSynthAudioProcessorEditor)
};
