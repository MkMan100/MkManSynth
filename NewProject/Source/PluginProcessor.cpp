/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// Classe concreta necessaria per far funzionare il sintetizzatore JUCE
class SynthSound : public juce::SynthesiserSound
{
public:
    SynthSound() {}
    bool appliesToNote (int /*midiNoteNumber*/) override  { return true; }
    bool appliesToChannel (int /*midiChannel*/) override  { return true; }
};

//==============================================================================
MkManSynthAudioProcessor::MkManSynthAudioProcessor()
     : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    mySynth.clearVoices();
    for (int i = 0; i < 8; ++i)
    {
        mySynth.addVoice (new SynthVoice());
    }
    mySynth.clearSounds();
    mySynth.addSound (new SynthSound()); // Utilizza la classe concreta corretta
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
    
    float macroLeslie   = apvts.getRawParameterValue ("macro_leslie")->load();
    float baseCutoff    = apvts.getRawParameterValue ("filter_cutoff")->load();
    float baseResonance = apvts.getRawParameterValue ("filter_q")->load();
    float distDrive     = apvts.getRawParameterValue ("dist_drive")->load();
    delayTimeSec  = apvts.getRawParameterValue ("delay_time")->load();
    delayFeedback = apvts.getRawParameterValue ("delay_feedback")->load();

    if (macroLeslie > 0.0f) {
        leslieLFO.setFrequency (1.2f + (macroLeslie * 5.6f));
    }

    mySynth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    auto* leftChannel  = buffer.getWritePointer (0);
    auto* rightChannel = buffer.getWritePointer (1);
    int numSamples = buffer.getNumSamples();

    mainFilter.setResonance (baseResonance);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float lfoVal = leslieLFO.processSample (0.0f);
        float volumeModulation = 0.875f + (lfoVal * 0.125f);
        leftChannel[sample]  *= volumeModulation;
        rightChannel[sample] *= volumeModulation;

        float modulatedCutoff = juce::jlimit (20.0f, 20000.0f, baseCutoff + (lfoVal * 600.0f));
        mainFilter.setCutoffFrequency (modulatedCutoff);

        float samples[2] = { leftChannel[sample], rightChannel[sample] };
        leftChannel[sample]  = mainFilter.processSample (0, samples[0]);
        rightChannel[sample] = mainFilter.processSample (1, samples[1]);
    }

    // Risolto errore MSVC: applichiamo la distorsione direttamente sui campioni del buffer 
    // bypassando i limiti della lambda con cattura nel modulo WaveShaper di JUCE 7
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

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MkManSynthAudioProcessor(); }
