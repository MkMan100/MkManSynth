#include "PluginProcessor.h"
#include "PluginEditor.h"

#define JUCE_EXPORTED_FUNCTION __declspec(dllexport)

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
    
    globalLFO.prepare (spec);
    globalLFO.initialise ([] (float x) { return std::sin (x); });

    leslieLFO.prepare (spec);
    leslieLFO.initialise ([] (float x) { return std::sin (x); });
    
    mainFilter.prepare (spec);
    mainFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    stereoDelay.prepare (spec);
    stereoDelay.setMaximumDelayInSamples (static_cast<int>(sampleRate * 2.0));

    lowShelfFilter.prepare (spec);
    highShelfFilter.prepare (spec);
}

void MkManSynthAudioProcessor::releaseResources() {}

bool MkManSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void MkManSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    double sampleRate = getSampleRate();
    
    float waveMorph1 = apvts.getRawParameterValue ("osc1_morph")->load();
    float waveMorph2 = apvts.getRawParameterValue ("osc2_morph")->load();
    float oscMix     = apvts.getRawParameterValue ("osc_mix")->load();
    float detune1    = apvts.getRawParameterValue ("osc1_detune")->load();
    float detune2    = apvts.getRawParameterValue ("osc2_detune")->load();
    
    float attack  = apvts.getRawParameterValue ("env_attack")->load();
    float decay   = apvts.getRawParameterValue ("env_decay")->load();
    float sustain = apvts.getRawParameterValue ("env_sustain")->load();
    float release = apvts.getRawParameterValue ("env_release")->load();

    float lfoRate   = apvts.getRawParameterValue ("lfo_rate")->load();
    float lfoDepth  = apvts.getRawParameterValue ("lfo_depth")->load();
    int lfoDest     = static_cast<int> (apvts.getRawParameterValue ("lfo_dest")->load());

    float baseCutoff    = apvts.getRawParameterValue ("filter_cutoff")->load();
    float baseResonance = apvts.getRawParameterValue ("filter_q")->load();
    float distDrive     = apvts.getRawParameterValue ("dist_drive")->load();
    
    float eqBass  = apvts.getRawParameterValue ("eq_bass")->load();
    float eqTreble = apvts.getRawParameterValue ("eq_treble")->load();

    float macroLeslie = apvts.getRawParameterValue ("macro_leslie")->load();
    float macroSpace  = apvts.getRawParameterValue ("macro_space")->load();
    float macroSpread = apvts.getRawParameterValue ("macro_spread")->load();

    globalLFO.setFrequency (lfoRate);
    if (macroLeslie > 0.0f) {
        leslieLFO.setFrequency (1.5f + (macroLeslie * 6.0f));
    }

    for (int i = 0; i < mySynth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<SynthVoice*> (mySynth.getVoice(i)))
        {
            v->setMorphValues (waveMorph1, waveMorph2);
            v->updateAdsr (attack, decay, sustain, release);
        }
    }

    buffer.clear();
    
    float lfoSignal = globalLFO.processSample (0.0f);
    float pitchMod = (lfoDest == 3) ? (lfoSignal * lfoDepth * 0.3f) : 0.0f;
    float panMod   = (lfoDest == 2) ? (lfoSignal * lfoDepth) : 0.0f;
    float volModLFO   = (lfoDest == 1) ? (1.0f - (lfoDepth * 0.5f) + (lfoSignal * lfoDepth * 0.5f)) : 1.0f;

    for (int i = 0; i < mySynth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<SynthVoice*> (mySynth.getVoice(i)))
        {
            v->renderVoice (buffer, 0, buffer.getNumSamples(), oscMix, detune1, detune2, pitchMod, panMod, macroSpread);
        }
    }
    
    mySynth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    auto* leftChannel  = buffer.getWritePointer (0);
    auto* rightChannel = buffer.getWritePointer (1);
    int numSamples = buffer.getNumSamples();

    *lowShelfFilter.coefficients  = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (sampleRate, 200.0f, 0.707f, juce::Decibels::decibelsToGain(eqBass));
    *highShelfFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate, 4000.0f, 0.707f, juce::Decibels::decibelsToGain(eqTreble));

    mainFilter.setResonance (baseResonance);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float leslieVal = leslieLFO.processSample (0.0f);
        
        float currentVolMod = volModLFO;
        if (macroLeslie > 0.0f) {
            currentVolMod *= (0.8f + (leslieVal * 0.2f * macroLeslie));
        }
        leftChannel[sample]  *= currentVolMod;
        rightChannel[sample] *= currentVolMod;

        float filterLfoSignal = (lfoDest == 0) ? (lfoSignal * lfoDepth * 4000.0f) : 0.0f;
        float finalCutoff = baseCutoff + filterLfoSignal;
        
        if (macroSpace > 0.0f) {
            finalCutoff = juce::jmax (40.0f, finalCutoff * (1.0f - (macroSpace * 0.85f)));
        }
        if (macroLeslie > 0.0f) {
            finalCutoff += (leslieVal * macroLeslie * 300.0f);
        }
        
        mainFilter.setCutoffFrequency (juce::jlimit (20.0f, 20000.0f, finalCutoff));

        float samples[2] = { leftChannel[sample], rightChannel[sample] };
        leftChannel[sample]  = mainFilter.processSample (0, samples[0]);
        rightChannel[sample] = mainFilter.processSample (1, samples[1]);

        leftChannel[sample]  = lowShelfFilter.processSample (leftChannel[sample]);
        leftChannel[sample]  = highShelfFilter.processSample (leftChannel[sample]);
        rightChannel[sample] = lowShelfFilter.processSample (rightChannel[sample]);
        rightChannel[sample] = highShelfFilter.processSample (rightChannel[sample]);
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

    delayTimeSec = 0.35f; 
    float dynamicFeedback = delayFeedback + (macroSpace * (0.95f - delayFeedback));
    float delayInSamples  = delayTimeSec * static_cast<float> (sampleRate);
    stereoDelay.setDelay (delayInSamples);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float delayedLeft  = stereoDelay.popSample (0);
        float delayedRight = stereoDelay.popSample (1);
        float dryLeft  = leftChannel[sample];
        float dryRight = rightChannel[sample];

        stereoDelay.pushSample (0, dryLeft  + (delayedRight * dynamicFeedback));
        stereoDelay.pushSample (1, dryRight + (delayedLeft  * dynamicFeedback));

        float mixAmt = 0.25f + (macroSpace * 0.25f);
        leftChannel[sample]  = dryLeft  + (delayedLeft  * mixAmt);
        rightChannel[sample] = dryRight + (delayedRight * mixAmt);
        
        // Invia il segnale mixato finale all'oscilloscopio
        scopeBuffer.pushSample ((leftChannel[sample] + rightChannel[sample]) * 0.5f);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout MkManSynthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc1_morph", "Osc 1 Wave Morph", 0.0f, 3.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2_morph", "Osc 2 Wave Morph", 0.0f, 3.0f, 2.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc_mix", "Osc Mix", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc1_detune", "Osc 1 Detune", 0.0f, 50.0f, 8.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2_detune", "Osc 2 Detune", 0.0f, 50.0f, 12.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env_attack", "Attack", 0.01f, 3.0f, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env_decay", "Decay", 0.01f, 3.0f, 0.3f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env_sustain", "Sustain", 0.0f, 1.0f, 0.7f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env_release", "Release", 0.01f, 5.0f, 0.5f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo_rate", "LFO Rate", 0.1f, 25.0f, 6.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo_depth", "LFO Depth", 0.0f, 1.0f, 0.3f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("lfo_dest", "LFO Destination", juce::StringArray{"Cutoff", "Volume", "Pan", "Pitch"}, 0));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filter_cutoff", "Cutoff", 20.0f, 18000.0f, 3000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("filter_q", "Resonance", 1.0f, 15.0f, 1.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("dist_drive", "Dist Drive", 1.0f, 15.0f, 1.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("eq_bass", "EQ Bass (dB)", -12.0f, 12.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("eq_treble", "EQ Treble (dB)", -12.0f, 12.0f, 0.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("macro_leslie", "Macro Leslie", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("macro_space", "Macro Space", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("macro_spread", "Macro Spread", 0.0f, 1.0f, 0.4f));

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

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MkManSynthAudioProcessor();
}
