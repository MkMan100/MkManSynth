#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MkManSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    MkManSynthAudioProcessorEditor (MkManSynthAudioProcessor&);
    ~MkManSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void cooledResized() override; // Nota: JUCE standard usa resized(), assicurati che sia coerente con il template generato

private:
    // Scorciatoia per non dover scrivere ogni volta il tipo lunghissimo dell'allegato
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    // 1. Gli elementi grafici (Le manopole delle 3 Macro)
    juce::Slider macro1Slider;
    juce::Slider macro2Slider;
    juce::Slider macro3Slider;

    // 2. Gli allegati che collegano i componenti grafici all'APVTS del processore
    std::unique_ptr<Attachment> macro1Attachment;
    std::unique_ptr<Attachment> macro2Attachment;
    std::unique_ptr<Attachment> macro3Attachment;

    // Riferimento al processore audio
    MkManSynthAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MkManSynthAudioProcessorEditor)
};
