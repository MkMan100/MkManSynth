#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice()
    {
        updateWaveform (1, 0); // Default: Sine
        updateWaveform (2, 0);
    }

    bool canPlaySound (juce::SynthesiserSound* sound) override { return dynamic_cast<juce::SynthesiserSound*> (sound) != nullptr; }

    void updateWaveform (int group, int type)
    {
        auto selectWave = [] (int t) -> std::function<float(float)> {
            if (t == 1) return [] (float x) { return x / juce::MathConstants<float>::pi; }; // Saw
            if (t == 2) return [] (float x) { return x > 0.0f ? 1.0f : -1.0f; };            // Square
            if (t == 3) return [] (float x) { return 1.0f - (2.0f * std::abs (x / juce::MathConstants<float>::pi)); }; // Triangle
            return [] (float x) { return std::sin (x); };                                   // Sine
        };

        for (int i = 0; i < 5; ++i)
        {
            if (group == 1) osc1Group[i].initialise (selectWave (type));
            else            osc2Group[i].initialise (selectWave (type));
        }
    }

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

    void updateAdsr (float a, float d, float s, float r)
    {
        adsrParams.attack = a;
        adsrParams.decay = d;
        adsrParams.sustain = s;
        adsrParams.release = r;
        adsr.setParameters (adsrParams);
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
        // Metodo vuoto richiesto, la logica reale usa i parametri passati dal Processor
    }

    // Rendering reale orchestrato dal processore per mappare i parametri in tempo reale
    void renderVoice (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples,
                      float oscMix, float detune1, float detune2, float lfoMod)
    {
        if (!adsr.isActive()) { clearCurrentNote(); return; }

        juce::AudioBuffer<float> voiceLeft (1, numSamples);
        juce::AudioBuffer<float> voiceRight (1, numSamples);
        voiceLeft.clear(); voiceRight.clear();

        auto* leftData  = voiceLeft.getWritePointer (0);
        auto* rightData = voiceRight.getWritePointer (0);

        float vol1 = (0.25f * (1.0f - oscMix)) / 5.0f;
        float vol2 = (0.25f * oscMix) / 5.0f;

        // Applica frequenze con Detune e modulazione LFO
        for (int i = 0; i < 5; ++i)
        {
            float d1 = 1.0f + ((i - 2) * (detune1 * 0.01f) + lfoMod);
            float d2 = 1.0f + ((i - 2) * (detune2 * 0.01f) + lfoMod);
            osc1Group[i].setFrequency (baseFrequency * d1);
            osc2Group[i].setFrequency (baseFrequency * d2);
        }

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentSampleLeft = 0.0f;
            float currentSampleRight = 0.0f;

            for (int i = 0; i < 5; ++i)
            {
                float signal = osc1Group[i].processSample (0.0f) * vol1;
                float pan = (i - 2) * 0.25f; 
                currentSampleLeft  += signal * std::sqrt (0.5f * (1.0f - pan));
                currentSampleRight += signal * std::sqrt (0.5f * (1.0f + pan));
            }

            for (int i = 0; i < 5; ++i)
            {
                float signal = osc2Group[i].processSample (0.0f) * vol2;
                float pan = (i - 2) * 0.25f;
                currentSampleLeft  += signal * std::sqrt (0.5f * (1.0f - pan));
                currentSampleRight += signal * std::sqrt (0.5f * (1.0f + pan));
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
    juce::ADSR::Parameters adsrParams;
};
