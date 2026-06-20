#include "PluginProcessor.h"
#include "PluginEditor.h"

class SynthSound : public juce::SynthesiserSound
{
public:
    SynthSound() {}
    bool appliesToNote (int) override  { return true; }
    bool appliesToChannel (int) override  { return true; }
};

MkManSynthAudioProcessor::MkManSynthAudioProcessor()
     : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    mySynth.clearVoices();
    for (int i = 0; i < 8; ++i) { mySynth.addVoice (new SynthVoice()); }
    mySynth.clearSounds();
    mySynth.addSound (new SynthSound());
}

MkManSynthAudioProcessor::~MkManSynthAudioProcessor() {}
const juce::String MkManSynthAudioProcessor::getName() const { return JucePlugin_Name; }
bool MkManSynthAudioProcessor::acceptsMidi() const { return true; }
bool MkManSynthAudioProcessor::producesMidi() const { return false; }
bool MkManSynthAudioProcessor::isMidiEffect() const { return false; }
double MkManSynthAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int MkManSynthAudioProcessor::getNumPrograms() { return 1; }
int MkManSynthAudioProcessor::getCurrentProgram() { return 0; }
void MkManSynthAudioProcessor::setCurrentProgram (int) {}
const juce::String MkManSynthAudioProcessor::getProgramName (int) { return {}; }
void MkManSynthAudioProcessor::changeProgramName (int, const juce::String&) {}
juce::AudioProcessorEditor* MkManSynthAudioProcessor::createEditor() { return new MkManSynthAudioProcessorEditor (*this); }
bool MkManSynthAudioProcessor::hasEditor() const { return true; }

void MkManSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mySynth.setCurrentPlaybackSampleRate (sampleRate);
    for (int i = 0; i < mySynth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<SynthVoice*> (mySynth.getVoice(i)))
            v->prepare (sampleRate, samplesPerBlock);
    }
        
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    
    distortionModule.prepare (spec);
    leslieLFO.prepare (spec);
    leslieLFO.initialise ([] (float x) { return std::sin (x); });

    globalLFO.prepare (spec);
    globalLFO.initialise ([] (float x) { return std::sin (x); });
    
    mainFilter.prepare (spec);
    mainFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    stereoDelay.prepare (spec);
    stereoDelay.setMaximumDelayInSamples (static_cast<int>(sampleRate * 2.0));
}

void MkManSynthAudioProcessor::releaseResources() {}

bool MkManSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void MkManSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // 1. Caricamento di tutti i 15 parametri dall'APVTS
    int waveType1 = static_cast<int> (apvts.getRawParameterValue ("osc1_wave")->load());
    int waveType2 = static_cast<int> (apvts.getRawParameterValue ("osc2_wave")->load());
    float oscMix  = apvts.getRawParameterValue ("osc_mix")->load();
    float detune1 = apvts.getRawParameterValue ("osc1_detune")->load();
    float detune2 = apvts.getRawParameterValue ("osc2_detune")->load();
    
    float attack  = apvts.getRawParameterValue ("env_attack")->load();
    float decay   = apvts.getRawParameterValue ("env_decay")->load();
    float sustain = apvts.getRawParameterValue ("env_sustain")->load();
    float release = apvts.getRawParameterValue ("env_release")->load();

    float lfoRate   = apvts.getRawParameterValue ("lfo_rate")->load();
    float lfoDepth  = apvts.getRawParameterValue ("lfo_depth")->load();

    float baseCutoff    = apvts.getRawParameterValue ("filter_cutoff")->load();
    float baseResonance = apvts.getRawParameterValue ("filter_q")->load();
    float distDrive     = apvts.getRawParameterValue ("dist_drive")->load();
    
    delayTimeSec  = apvts.getRawParameterValue ("delay_time")->load();
    delayFeedback = apvts.getRawParameterValue ("delay_feedback")->load();
    float macroLeslie = apvts.getRawParameterValue ("macro_leslie")->load();

    // 2. Configura LFO Globale ed Envelope per ogni voce attiva
    globalLFO.setFrequency (lfoRate);
    if (macroLeslie > 0.0f) {
        leslieLFO.setFrequency (1.2f + (macroLeslie * 5.6f));
    }

    for (int i = 0; i < mySynth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<SynthVoice*> (mySynth.getVoice(i)))
        {
            v->updateWaveform (1, waveType1);
            v->updateWaveform (2, waveType2);
            v->updateAdsr (attack, decay, sustain, release);
        }
    }

    // 3. Gestione MIDI e rendering audio delle voci
    buffer.clear();
    
    // Invece del render automatico standard, passiamo noi i parametri alle voci attive
    for (int i = 0; i < mySynth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<SynthVoice*> (mySynth.getVoice(i)))
        {
            float lfoValue = globalLFO.processSample (0.0f) * lfoDepth * 0.05f; // Modulazione pitch leggera
            v->renderVoice (buffer, 0, buffer.getNumSamples(), oscMix, detune1, detune2, lfoValue);
        }
    }
    
    // Gestisce gli eventi midi per accendere/spegnere le note
    mySynth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    auto* leftChannel  = buffer.getWritePointer (0);
    auto* rightChannel = buffer.getWritePointer (1);
    int numSamples = buffer.getNumSamples();

    // 4. Catena Effetti DSP (Filtro modulato da LFO Leslie, Distorsione, Delay)
    mainFilter.setResonance (baseResonance);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float leslieVal = leslieLFO.processSample (0.0f);
        if (macroLeslie > 0.0f) {
            float volumeMod = 0.85f + (leslieVal * 0.15f * macroLeslie);
            leftChannel[sample] *= volumeMod;
            rightChannel[sample] *= volumeMod;
        }

        float lfoFilterMod = globalLFO.processSample (0.0f) * lfoDepth * 300.0f;
        float modulatedCutoff = juce::jlimit (20.0f, 20000.0f, baseCutoff + lfoFilterMod + (leslieVal * macroLeslie * 400.0f));
        mainFilter.setCutoffFrequency (modulatedCutoff);

        float samples[2] = { leftChannel[sample], rightChannel[sample] };
        leftChannel[sample]  = mainFilter.processSample (0, samples[0]);
        rightChannel[sample] = mainFilter.processSample (1, samples[1]);
    }

    float comp = 1.0f / std::sqrt (distDrive);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] = std::tanh (channelData[sample] * distDrive) * comp;
        }
    }

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

juce::AudioProcessorValueTreeState::ParameterLayout MkManSynthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // I 15 PARAMETRI COMPLETI
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("osc1_wave", "Osc 1 Waveform", juce::StringArray{"Sine", "Saw", "Square", "Triangle"}, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("osc2_wave", "Osc 2 Waveform", juce::StringArray{"Sine", "Saw", "Square", "Triangle"}, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc_mix", "Osc Mix", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc1_detune", "Osc 1 Unison Detune", 0.0f, 50.0f, 5.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2_detune", "Osc 2 Unison Detune", 0.0f, 50.0f, 10.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env_attack", "Attack Time", 0.01f, 3.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env_decay", "Decay Time", 0.01f, 3.0f, 0.3f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env_sustain", "Sustain Level", 0.0f, 1.0f, 0.7f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env_release", "Release Time", 0.01f, 5.0f, 0.5f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo_rate", "LFO Frequency", 0.1f, 20.0f, 5.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo_depth", "LFO Amount", 0.0f, 1.0f, 0.2f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filter_cutoff", "Base Cutoff", 20.0f, 15000.0f, 2000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filter_q", "Filter Q (Res)", 1.0f, 15.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("dist_drive", "Distortion Drive", 1.0f, 12.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("delay_time", "Delay Time", 0.05f, 1.0f, 0.3f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("delay_feedback", "Delay Feedback", 0.0f, 0.95f, 0.4f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("macro_leslie", "Macro: Leslie", 0.0f, 1.0f, 0.0f));

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

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MkManSynthAudioProcessor(); }
