#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "SynthVoice.h"

// Struttura circolare per passare i dati audio alla GUI in modo thread-safe
struct AudioVisualiserBuffer {
    float buffer[512] { 0.0f };
    std::atomic<int> writeIndex { 0 };
    void pushSample (float sample) {
        int idx = writeIndex.load();
        buffer[idx] = sample;
        writeIndex.store ((idx + 1) % 512);
    }
};

class MkManSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    MkManSynthAudioProcessor();
    ~MkManSynthAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    AudioVisualiserBuffer scopeBuffer; // Istanza accessibile dall'Editor

private:
    juce::Synthesiser mySynth;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    juce::dsp::Oscillator<float> globalLFO;
    juce::dsp::Oscillator<float> leslieLFO;
    juce::dsp::StateVariableTPTFilter<float> mainFilter;
    juce::dsp::DelayLine<float> stereoDelay;
    
    juce::dsp::IIR::Filter<float> lowShelfFilter;
    juce::dsp::IIR::Filter<float> highShelfFilter;

    float delayFeedback { 0.4f };
    float delayTimeSec { 0.3f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MkManSynthAudioProcessor)
};
