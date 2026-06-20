#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

class MkManSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    MkManSynthAudioProcessorEditor (MkManSynthAudioProcessor&);
    ~MkManSynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override; // Nota: usiamo il resized() standard di JUCE

private:
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    // --- CONTROLLI GRAFICI ---
    // Macro
    juce::Slider macro1Slider, macro2Slider, macro3Slider;
    // Sintesi & Filtro
    juce::Slider oscMixSlider, filterCutoffSlider, filterQSlider;
    // Effetti
    juce::Slider distDriveSlider, delayTimeSlider, delayFeedbackSlider;

    // --- ALLEGATI APVTS ---
    std::unique_ptr<Attachment> macro1Attachment, macro2Attachment, macro3Attachment;
    std::unique_ptr<Attachment> oscMixAttachment, filterCutoffAttachment, filterQAttachment;
    std::unique_ptr<Attachment> distDriveAttachment, delayTimeAttachment, delayFeedbackAttachment;

    MkManSynthAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MkManSynthAudioProcessorEditor)
};
