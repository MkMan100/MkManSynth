/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MkManSynthAudioProcessor::MkManSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts (*this, nullptr, "Parameters", createParameterLayout()) // INIZIALIZZAZIONE APVTS
#endif
{
    // Generiamo le wavetable personalizzate
    createCustomWavetables();

    // Creiamo 8 voci polifoniche per il nostro synth
    mySynth.clearVoices();
    for (int i = 0; i < 8; ++i)
    {
        auto* voice = new SynthVoice();
        
        // Carichiamo di default la wavetable dell'organo su tutti e 10 gli oscillatori della voce
        for (int osc = 0; osc < 5; ++osc)
        {
            voice->setWavetable (osc, wavetableOrgan, true);  // Gruppo 1
            voice->setWavetable (osc, wavetableOrgan, false); // Gruppo 2
        }
        
        mySynth.addVoice (voice);
    }

    mySynth.clearSounds();
    mySynth.addSound (new juce::SynthesiserSound());
}

//==============================================================================
const juce::String MkManSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MkManSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MkManSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MkManSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MkManSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MkManSynthAudioProcessor::getNumPrograms()
{
    return 1;
}

int MkManSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MkManSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MkManSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void MkManSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MkManSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mySynth.setCurrentPlaybackSampleRate (sampleRate);

    // Inizializziamo i parametri DSP di ciascuna voce allocata
    for (int i = 0; i < mySynth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<SynthVoice*> (mySynth.getVoice(i)))
        {
            v->prepare (sampleRate, samplesPerBlock);
        }
    }
        
    // --- CONFIGURAZIONE EFFETTI MASTER ---
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 }; // Stereo
    
    // 1. Prepara il modulo Distorsione
    distortionModule.prepare (spec);
    distortionModule.functionToUse = [] (float inputSample) 
    {
        float drive = 4.0f; 
        float distortedSample = std::tanh (inputSample * drive);
        float compensation = 1.0f / std::sqrt (drive);
        return distortedSample * compensation;
    }; 

    // 2. Prepara l'LFO del Leslie
    leslieLFO.prepare (spec);
    leslieLFO.initialise ([] (float x) { return std::sin (x); });
    
    // 3. Prepara il Filtro
    mainFilter.prepare (spec);
    mainFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    // 4. Prepara il Delay Stereo
    stereoDelay.prepare (spec);
    stereoDelay.setMaximumDelayInSamples (static_cast<int>(sampleRate * 2.0));
}

void MkManSynthAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MkManSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MkManSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // --- LETTURA DEI PARAMETRI DALL'APVTS IN TEMPO REALE ---
    float macroLeslie   = apvts.getRawParameterValue ("macro_leslie")->load();
    float baseCutoff    = apvts.getRawParameterValue ("filter_cutoff")->load();
    float baseResonance = apvts.getRawParameterValue ("filter_q")->load();
    float distDrive     = apvts.getRawParameterValue ("dist_drive")->load();
    
    delayTimeSec  = apvts.getRawParameterValue ("delay_time")->load();
    delayFeedback = apvts.getRawParameterValue ("delay_feedback")->load();

    // Gestione Macro 3 (Leslie Speed)
    if (macroLeslie > 0.0f) {
        float leslieSpeed = 1.2f + (macroLeslie * 5.6f);
        leslieLFO.setFrequency (leslieSpeed);
    }

    // 1. Generiamo l'audio dell'unisono dalle voci
    mySynth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    // 2. Applicazione Filtro + Leslie
    auto* leftChannel  = buffer.getWritePointer (0);
    auto* rightChannel = buffer.getWritePointer (1);
    int numSamples = buffer.getNumSamples();

    mainFilter.setResonance (baseResonance);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float lfoVal = leslieLFO.processSample (0.0f);

        // Modulazione Volume Leslie
        float volumeModulation = 0.875f + (lfoVal * 0.125f);
        leftChannel[sample]  *= volumeModulation;
        rightChannel[sample] *= volumeModulation;

        // Modulazione Cutoff Leslie
        float modulatedCutoff = baseCutoff + (lfoVal * 600.0f);
        modulatedCutoff = juce::jlimit (20.0f, 20000.0f, modulatedCutoff);
        mainFilter.setCutoffFrequency (modulatedCutoff);

        float samples[2] = { leftChannel[sample], rightChannel[sample] };
        leftChannel[sample]  = mainFilter.processSample (0, samples[0]);
        rightChannel[sample] = mainFilter.processSample (1, samples[1]);
    }

    // 3. Aggiorniamo dinamicamente il drive della distorsione iperbolica prima del calcolo
    distortionModule.functionToUse = [distDrive] (float x) {
        float distortedSample = std::tanh (x * distDrive);
        return distortedSample * (1.0f / std::sqrt (distDrive));
    };

    juce::dsp::AudioBlock<float> audioBlock (buffer);
    juce::dsp::ProcessContextReplacing<float> context (audioBlock);
    distortionModule.process (context);

    // 4. Applicazione Delay Stereo
    float delayInSamples = delayTimeSec * static_cast<float> (getSampleRate());
    stereoDelay.setDelay (delayInSamples);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float delayedLeft  = stereoDelay.popSample (0);
        float delayedRight = stereoDelay.popSample (1);

        float dryLeft  = leftChannel[sample];
        float dryRight = rightChannel[sample];

        stereoDelay.pushSample (0, dryLeft  + (delayedRight * delayFeedback));
        stereoDelay.pushSample (1, dryRight + (delayedLeft  * delayFeedback));

        leftChannel[sample]  = dryLeft  + (delayedLeft  * 0.3f);
        rightChannel[sample] = dryRight + (delayedRight * 0.3f);
    }
}

//==============================================================================
void MkManSynthAudioProcessor::createCustomWavetables()
{
    const int tableSize = 2048;

    std::vector<float> harmonicsOrgan = { 0.0f, 1.0f, 0.8f, 0.6f, 0.4f, 0.2f, 0.1f, 0.05f };
    juce::AudioBuffer<float> bufferOrgan (1, tableSize);
    auto* dataOrgan = bufferOrgan.getWritePointer (0);
    
    std::vector<float> harmonicsBrass = { 0.0f, 1.0f, 0.5f, 0.33f, 0.25f, 0.20f, 0.16f, 0.14f, 0.12f, 0.10f };
    juce::AudioBuffer<float> bufferBrass (1, tableSize);
    auto* dataBrass = bufferBrass.getWritePointer (0);

    std::vector<float> harmonicsCosmic = { 0.0f, 0.2f, 0.8f, 0.1f, 0.9f, 0.0f, 0.5f, 0.0f, 0.3f };
    juce::AudioBuffer<float> bufferCosmic (1, tableSize);
    auto* dataCosmic = bufferCosmic.getWritePointer (0);

    for (int i = 0; i < tableSize; ++i)
    {
        float phase = (static_cast<float>(i) / static_cast<float>(tableSize)) * juce::MathConstants<float>::twoPi;
        
        float sampleOrgan = 0.0f;
        float sampleBrass = 0.0f;
        float sampleCosmic = 0.0f;

        for (size_t h = 1; h < harmonicsOrgan.size(); ++h)
            sampleOrgan += harmonicsOrgan[h] * std::sin (phase * h);

        for (size_t h = 1; h < harmonicsBrass.size(); ++h)
            sampleBrass += harmonicsBrass[h] * std::sin (phase * h);

        for (size_t h = 1; h < harmonicsCosmic.size(); ++h)
            sampleCosmic += harmonicsCosmic[h] * std::sin (phase * h);

        dataOrgan[i] = sampleOrgan;
        dataBrass[i] = sampleBrass;
        dataCosmic[i] = sampleCosmic;
    }

    wavetableOrgan = juce::dsp::Oscillator<float>::WaveTable (bufferOrgan);
    wavetableBrass = juce::dsp::Oscillator<float>::WaveTable (bufferBrass);
    wavetableCosmic = juce::dsp::Oscillator<float>::WaveTable (bufferCosmic);
}

juce::AudioProcessorValueTreeState::ParameterLayout MkManSynthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc_mix", "Osc Mix", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filter_cutoff", "Base Cutoff", 100.0f, 8000.0f, 2000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filter_q", "Filter Q", 1.0f, 20.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("dist_drive", "Distortion Drive", 1.0f, 15.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("delay_time", "Delay Time", 0.05f, 1.0f, 0.3f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("delay_feedback", "Delay Feedback", 0.0f, 0.95f, 0.4f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("macro_dark_reverb", "Macro 1: Dark Reverb", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("macro_cyber_punch", "Macro 2: Cyber Punch", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("macro_leslie", "Macro 3: Leslie Simulator", 0.0f, 1.0f, 0.0f));

    return { params.begin(), params.end() };
}

void MkManSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MkManSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MkManSynthAudioProcessor();
}