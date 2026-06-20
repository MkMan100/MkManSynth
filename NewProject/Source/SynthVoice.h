#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice() {}

    bool canPlaySound (juce::SynthesiserSound* sound) override { return dynamic_cast<juce::SynthesiserSound*> (sound) != nullptr; }

    void setMorphValues (float morph1, float morph2) { m1 = morph1; m2 = morph2; }

    float generateMorphedSample (float phase, float morphVal)
    {
        float sine     = std::sin (phase);
        float saw      = phase / juce::MathConstants<float>::pi;
        float square   = phase > 0.0f ? 1.0f : -1.0f;
        float triangle = 1.0f - (2.0f * std::abs (phase / juce::MathConstants<float>::pi));

        if (morphVal <= 1.0f) return sine     + (saw - sine) * morphVal;
        if (morphVal <= 2.0f) return saw      + (square - saw) * (morphVal - 1.0f);
        return square   + (triangle - square) * (morphVal - 2.0f);
    }

    void prepare (double sampleRate, int samplesPerBlock)
    {
        adsr.setSampleRate (sampleRate);
        currentSampleRate = sampleRate;
    }

    void updateAdsr (float a, float d, float s, float r)
    {
        adsrParams.attack = a;
        adsrParams.decay = d;
        adsrParams.sustain = s;
        adsrParams.release = r;
        adsr.setParameters (adsrParams);
    }

    void startNote (int midiNoteNumber, float, juce::SynthesiserSound*, int) override
    {
        baseFrequency = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        adsr.noteOn();
        for (int i = 0; i < 5; ++i) { phases1[i] = 0.0f; phases2[i] = 0.0f; }
    }

    void stopNote (float, bool allowTailOff) override
    {
        adsr.noteOff();
        if (!allowTailOff) clearCurrentNote();
    }

    void renderNextBlock (juce::AudioBuffer<float>&, int, int) override {}

    void renderVoice (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples,
                      float oscMix, float detune1, float detune2, float pitchMod, float panMod, float macroSpread)
    {
        if (!adsr.isActive()) { clearCurrentNote(); return; }

        juce::AudioBuffer<float> voiceLeft (1, numSamples);
        juce::AudioBuffer<float> voiceRight (1, numSamples);
        voiceLeft.clear(); voiceRight.clear();

        auto* leftData  = voiceLeft.getWritePointer (0);
        auto* rightData = voiceRight.getWritePointer (0);

        float vol1 = (0.22f * (1.0f - oscMix)) / 5.0f;
        float vol2 = (0.22f * oscMix) / 5.0f;

        float invSampleRate = 1.0f / static_cast<float> (currentSampleRate);
        float twopi = juce::MathConstants<float>::twoPi;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float outL = 0.0f;
            float outR = 0.0f;

            for (int i = 0; i < 5; ++i)
            {
                float freqFactor = 1.0f + ((i - 2) * (detune1 * 0.008f)) + pitchMod;
                phases1[i] += baseFrequency * freqFactor * twopi * invSampleRate;
                if (phases1[i] > juce::MathConstants<float>::pi)  phases1[i] -= twopi;

                float sig = generateMorphedSample (phases1[i], m1) * vol1;
                float pan = juce::jlimit (-1.0f, 1.0f, ((i - 2) * 0.25f) * macroSpread + panMod);
                
                outL += sig * std::sqrt (0.5f * (1.0f - pan));
                outR += sig * std::sqrt (0.5f * (1.0f + pan));
            }

            for (int i = 0; i < 5; ++i)
            {
                float freqFactor = 1.0f + ((i - 2) * (detune2 * 0.008f)) + pitchMod;
                phases2[i] += baseFrequency * freqFactor * twopi * invSampleRate;
                if (phases2[i] > juce::MathConstants<float>::pi)  phases2[i] -= twopi;

                float sig = generateMorphedSample (phases2[i], m2) * vol2;
                float pan = juce::jlimit (-1.0f, 1.0f, ((i - 2) * 0.25f) * macroSpread + panMod);

                outL += sig * std::sqrt (0.5f * (1.0f - pan));
                outR += sig * std::sqrt (0.5f * (1.0f + pan));
            }

            leftData[sample]  = outL;
            rightData[sample] = outR;
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
    double currentSampleRate { 44100.0 };
    float phases1[5] { 0.0f };
    float phases2[5] { 0.0f };
    float m1 { 1.0f }, m2 { 2.0f };
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
};
