/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "SynthVoice.h"

//==============================================================================
class MkManSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    MkManSynthAudioProcessor();
    ~MkManSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // L'APVTS deve essere pubblica per permettere all'Editor (UI) di connettersi
    juce::AudioProcessorValueTreeState apvts;

private:
    juce::Synthesiser mySynth;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Le 3 tabelle in memoria per le nostre Wavetable complesse
    juce::dsp::Oscillator<float>::WaveTable wavetableOrgan;
    juce::dsp::Oscillator<float>::WaveTable wavetableBrass;
    juce::dsp::Oscillator<float>::WaveTable wavetableCosmic;
    
    juce::dsp::WaveShaper<float> distortionModule;
    juce::dsp::Oscillator<float> leslieLFO;
    juce::dsp::StateVariableTPTFilter<float> mainFilter;
    
    float lastLFOValue { 0.0f };
    
    juce::dsp::DelayLine<float> stereoDelay;
    
    float delayFeedback { 0.4f };
    float delayTimeSec { 0.3f };

    void createCustomWavetables();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MkManSynthAudioProcessor)
};
