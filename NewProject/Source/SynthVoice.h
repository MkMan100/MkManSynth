/*
  ==============================================================================

    SynthVoice.h
    Created: 20 Jun 2026 4:21:11pm
    Author:  User

  ==============================================================================
*/

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice()
    {
        // Inizializzazione dei parametri ADSR di default
        adsrParameters.attack = 0.05f;
        adsrParameters.decay = 0.2f;
        adsrParameters.sustain = 0.7f;
        adsrParameters.release = 0.6f;
        adsr.setParameters (adsrParameters);
    }

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<juce::SynthesiserSound*> (sound) != nullptr;
    }

    // Configura la frequenza di campionamento prima della riproduzione
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

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override
    {
        baseFrequency = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        adsr.noteOn();
    }

    void stopNote (float velocity, bool allowTailOff) override
    {
        adsr.noteOff();
        if (!allowTailOff) clearCurrentNote();
    }

    // --- IL CUORE MATEMATICO DEL SYNTH ---
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (!adsr.isActive())
        {
            clearCurrentNote();
            return;
        }

        // Buffer temporanei separati per i canali Left e Right di questa specifica voce
        juce::AudioBuffer<float> voiceLeft (1, numSamples);
        juce::AudioBuffer<float> voiceRight (1, numSamples);
        voiceLeft.clear();
        voiceRight.clear();

        auto* leftData  = voiceLeft.getWritePointer (0);
        auto* rightData = voiceRight.getWritePointer (0);

        // Recuperiamo i parametri globali (nel VST reale arriveranno dal processore)
        float oscMix = 0.5f; 
        float vol1 = (0.3f * (1.0f - oscMix)) / 5.0f; // Divisione per 5 per evitare il clipping
        float vol2 = (0.2f * oscMix) / 5.0f;
        
        float masterPan1 = -0.5f; // Osc 1 spostato a sinistra
        float masterPan2 = 0.5f;  // Osc 2 spostato a destra
        float detuneCentsMaster = 15.0f;

        // Impostiamo le frequenze scordate a ventaglio per l'Unisono
        for (int i = 0; i < 5; ++i)
        {
            // Calcolo detune interno all'unisono (es. -7, -3.5, 0, 3.5, 7 cents)
            float uDetune1 = (i - 2) * 3.5f;
            float uDetune2 = detuneCentsMaster + ((i - 2) * 4.0f);

            // Traduzione matematica da Cents a moltiplicatore di Frequenza: f = f0 * 2^(cents/1200)
            float freq1 = baseFrequency * std::pow (2.0f, uDetune1 / 1200.0f);
            float freq2 = baseFrequency * std::pow (2.0f, uDetune2 / 1200.0f);

            osc1Group[i].setFrequency (freq1);
            osc2Group[i].setFrequency (freq2);
        }

        // Calcolo dei campioni audio fotogramma per fotogramma
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentSampleLeft = 0.0f;
            float currentSampleRight = 0.0f;

            // --- GRUPPO OSCILLATORI 1 ---
            for (int i = 0; i < 5; ++i)
            {
                float signal = osc1Group[i].processSample (0.0f) * vol1;
                
                // Distribuzione Stereo Pan Lineare
                // In un unisono reale distribuiamo i 5 oscillatori da sinistra a destra
                float internalPan = masterPan1 + ((i - 2) * 0.1f); 
                internalPan = jmax (-1.0f, jmin (1.0f, internalPan)); // Hard limit tra -1 e 1

                float gainL = std::sqrt (0.5f * (1.0f - internalPan));
                float gainR = std::sqrt (0.5f * (1.0f + internalPan));

                currentSampleLeft += signal * gainL;
                currentSampleRight += signal * gainR;
            }

            // --- GRUPPO OSCILLATORI 2 ---
            for (int i = 0; i < 5; ++i)
            {
                float signal = osc2Group[i].processSample (0.0f) * vol2;
                
                float internalPan = masterPan2 + ((i - 2) * 0.1f);
                internalPan = jmax (-1.0f, jmin (1.0f, internalPan));

                float gainL = std::sqrt (0.5f * (1.0f - internalPan));
                float gainR = std::sqrt (0.5f * (1.0f + internalPan));

                currentSampleLeft += signal * gainL;
                currentSampleRight += signal * gainR;
            }

            leftData[sample]  = currentSampleLeft;
            rightData[sample] = currentSampleRight;
        }

        // Applichiamo l'inviluppo ADSR in modo indipendente ai due canali
        adsr.applyEnvelopeToBuffer (voiceLeft, 0, numSamples);
        adsr.applyEnvelopeToBuffer (voiceRight, 0, numSamples);

        // Sommiamo il risultato finale nei canali d'uscita principali del VST
        if (outputBuffer.getNumChannels() >= 2)
        {
            outputBuffer.addFrom (0, startSample, voiceLeft, 0, 0, numSamples); // Canale 0 = Left
            outputBuffer.addFrom (1, startSample, voiceRight, 0, 0, numSamples); // Canale 1 = Right
        }
    }

    // Metodo per aggiornare la tabella delle forme d'onda (Wavetable)
    void setWavetable (int oscInstance, const juce::dsp::Oscillator<float>::WaveTable& table, bool isOsc1)
    {
        if (isOsc1)
            osc1Group[oscInstance].setWaveTable (table);
        else
            osc2Group[oscInstance].setWaveTable (table);
    }

    void pitchWheelMoved (int newPitchWheelValue) override {}
    void controllerMoved (int controllerNumber, int newControllerValue) override {}

private:
    float baseFrequency { 440.0f };

    juce::dsp::Oscillator<float> osc1Group[5];
    juce::dsp::Oscillator<float> osc2Group[5];
    
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParameters;
};
