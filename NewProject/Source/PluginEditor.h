#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class DigitalOscilloscopeComponent : public juce::Component, public juce::Timer
{
public:
    DigitalOscilloscopeComponent (MkManSynthAudioProcessor& p) : processor (p) { startTimerHz (30); }
    ~DigitalOscilloscopeComponent() override { stopTimer(); }

    void timerCallback() override { repaint(); }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xFF0D0D12)); 
        g.setColour (juce::Colour (0xFF1A1A26));
        
        for (int i = 1; i < 4; ++i) {
            g.drawHorizontalLine (getHeight() * i / 4, 0.0f, (float)getWidth());
            g.drawVerticalLine (getWidth() * i / 4, 0.0f, (float)getHeight());
        }

        juce::Path wavePath;
        auto width  = (float) getWidth();
        auto height = (float) getHeight();
        auto midY   = height * 0.5f;

        wavePath.startNewSubPath (0.0f, midY);

        int rIdx = processor.scopeBuffer.writeIndex.load();
        
        for (int i = 0; i < 512; ++i)
        {
            int bufferDataIdx = (rIdx + i) % 512;
            float sampleValue = processor.scopeBuffer.buffer[bufferDataIdx];
            
            float x = (static_cast<float>(i) / 512.0f) * width;
            float y = midY - (sampleValue * midY * 1.8f); 
            
            y = juce::jlimit (2.0f, height - 2.0f, y);
            wavePath.lineTo (x, y);
        }

        g.setColour (juce::Colours::cyan.withAlpha(0.9f));
        g.strokePath (wavePath, juce::PathStrokeType (2.0f));
    }

private:
    MkManSynthAudioProcessor& processor;
};

class MkManSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    MkManSynthAudioProcessorEditor (MkManSynthAudioProcessor&);
    ~MkManSynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MkManSynthAudioProcessor& audioProcessor;
    DigitalOscilloscopeComponent oscilloscope; 

    juce::Slider osc1MorphSlider, osc2MorphSlider;
    juce::Slider oscMixSlider, osc1DetuneSlider, osc2DetuneSlider;
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Slider lfoRateSlider, lfoDepthSlider;
    juce::ComboBox lfoDestMenu;
    juce::Slider cutoffSlider, qSlider, distDriveSlider;
    juce::Slider eqBassSlider, eqTrebleSlider;
    juce::Slider delayTimeSlider, delayFeedbackSlider; // I controlli del Delay mancanti
    
    juce::Slider macroLeslieSlider, macroSpaceSlider, macroSpreadSlider;

    juce::Label osc1MorphLabel, osc2MorphLabel, oscMixLabel, osc1DetuneLabel, osc2DetuneLabel;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;
    juce::Label lfoRateLabel, lfoDepthLabel, lfoDestLabel;
    juce::Label cutoffLabel, qLabel, distDriveLabel;
    juce::Label eqBassLabel, eqTrebleLabel;
    juce::Label delayTimeLabel, delayFeedbackLabel;   // Etichette Delay
    juce::Label macroLeslieLabel, macroSpaceLabel, macroSpreadLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> osc1MorphAttach, osc2MorphAttach, oscMixAttach, osc1DetuneAttach, osc2DetuneAttach;
    std::unique_ptr<SliderAttachment> attackAttach, decayAttach, sustainAttach, releaseAttach;
    std::unique_ptr<SliderAttachment> lfoRateAttach, lfoDepthAttach;
    std::unique_ptr<ComboBoxAttachment> lfoDestAttach;
    std::unique_ptr<SliderAttachment> cutoffAttach, qAttach, distDriveAttach;
    std::unique_ptr<SliderAttachment> eqBassAttach, eqTrebleAttach;
    std::unique_ptr<SliderAttachment> delayTimeAttach, delayFeedbackAttach; // Allegati Delay
    std::unique_ptr<SliderAttachment> macroLeslieAttach, macroSpaceAttach, macroSpreadAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MkManSynthAudioProcessorEditor)
};
