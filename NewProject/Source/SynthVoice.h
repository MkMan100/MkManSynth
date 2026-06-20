#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice()
    {
        adsrParameters.attack = 0.05f;
        adsrParameters.decay = 0.2f;
        adsrParameters.sustain = 0.7f;
        adsrParameters.release = 0.6f;
        adsr.setParameters (adsrParameters);
        
        // Configura gli oscillatori per generare onde sinusoidali di base, modificabili dal processore
        for (int i = 0; i < 5; ++i)
        {
            osc1Group[i].initialise ([] (float x) { return std::sin (x); });
            osc2Group[i].initialise ([] (float x) { return std::sin (x); });
        }
    }

    bool canPlaySound (juce::SynthesiserSound* sound) override { return dynamic_cast<juce::SynthesiserSound*> (sound) != nullptr; }

    void prepare (double sampleRate, int samplesPerBlock)
    {
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 1 };
        for (int i = 0; i < 5; ++i)
        {
            osc1Group[i].prepare (spec);
            osc2Group[i].prepare (spec);
        }
        adsr.setSampleRate (sampleRate);
    }

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override
    {
        baseFrequency = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        adsr.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        adsr.noteOff();
        if (!allowTailOff) clearCurrentNote();
    }

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (!adsr.isActive()) { clearCurrentNote(); return; }

        juce::AudioBuffer<float> voiceLeft (1, numSamples);
        juce::AudioBuffer<float> voiceRight (1, numSamples);
        voiceLeft.clear(); voiceRight.clear();

        auto* leftData  = voiceLeft.getWritePointer (0);
        auto* rightData = voiceRight.getWritePointer (0);

        float oscMix = 0.5f; 
        float vol1 = (0.3f * (1.0f - oscMix)) / 5.0f;
        float vol2 = (0.2f * oscMix) / 5.0f;
        float masterPan1 = -0.5f;
        float masterPan2 = 0.5f;

        for (int i = 0; i < 5; ++i)
        {
            float freq1 = baseFrequency * std::pow (2.0f, ((i - 2) * 3.5f) / 1200.0f);
            float freq2 = baseFrequency * std::pow (2.0f, (15.0f + ((i - 2) * 4.0f)) / 1200.0f);
            osc1Group[i].setFrequency (freq1);
            osc2Group[i].setFrequency (freq2);
        }

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentSampleLeft = 0.0f;
            float currentSampleRight = 0.0f;

            for (int i = 0; i < 5; ++i)
            {
                float signal = osc1Group[i].processSample (0.0f) * vol1;
                float internalPan = juce::jmax (-1.0f, juce::jmin (1.0f, masterPan1 + ((i - 2) * 0.1f)));
                currentSampleLeft += signal * std::sqrt (0.5f * (1.0f - internalPan));
                currentSampleRight += signal * std::sqrt (0.5f * (1.0f + internalPan));
            }

            for (int i = 0; i < 5; ++i)
            {
                float signal = osc2Group[i].processSample (0.0f) * vol2;
                float internalPan = juce::jmax (-1.0f, juce::jmin (1.0f, masterPan2 + ((i - 2) * 0.1f)));
                currentSampleLeft += signal * std::sqrt (0.5f * (1.0f - internalPan));
                currentSampleRight += signal * std::sqrt (0.5f * (1.0f + internalPan));
            }

            leftData[sample]  = currentSampleLeft;
            rightData[sample] = currentSampleRight;
        }

        adsr.applyEnvelopeToBuffer (voiceLeft, 0, numSamples);
        adsr.applyEnvelopeToBuffer (voiceRight, 0, numSamples);

        if (outputBuffer.getNumChannels() >= 2)
        {
            outputBuffer.addFrom (0, startSample, voiceLeft, 0, 0, numSamples);
            outputBuffer.addFrom (1, startSample, voiceRight, 0, 0, numSamples);
        }
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

private:
    float baseFrequency { 440.0f };
    juce::dsp::Oscillator<float> osc1Group[5];
    juce::dsp::Oscillator<float> osc2Group[5];
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParameters;
};
