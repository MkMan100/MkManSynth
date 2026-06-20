#include "PluginProcessor.h"
#include "PluginEditor.h"

MkManSynthAudioProcessorEditor::MkManSynthAudioProcessorEditor (MkManSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // --- OSCILLATORI ---
    addAndMakeVisible (osc1WaveMenu);
    osc1WaveMenu.addItem ("Sine", 1);
    osc1WaveMenu.addItem ("Saw", 2);
    osc1WaveMenu.addItem ("Square", 3);
    osc1WaveMenu.addItem ("Triangle", 4);
    osc1WaveAttach = std::make_unique<ComboBoxAttachment> (audioProcessor.apvts, "osc1_wave", osc1WaveMenu);
    osc1WaveLabel.setText (juce::String ("Osc 1 Wave"), juce::dontSendNotification);
    addAndMakeVisible (osc1WaveLabel);

    addAndMakeVisible (osc2WaveMenu);
    osc2WaveMenu.addItem ("Sine", 1);
    osc2WaveMenu.addItem ("Saw", 2);
    osc2WaveMenu.addItem ("Square", 3);
    osc2WaveMenu.addItem ("Triangle", 4);
    osc2WaveAttach = std::make_unique<ComboBoxAttachment> (audioProcessor.apvts, "osc2_wave", osc2WaveMenu);
    osc2WaveLabel.setText (juce::String ("Osc 2 Wave"), juce::dontSendNotification);
    addAndMakeVisible (osc2WaveLabel);

    oscMixSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    oscMixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (oscMixSlider);
    oscMixLabel.setText (juce::String ("Osc Mix"), juce::dontSendNotification);
    oscMixLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (oscMixLabel);
    oscMixAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "osc_mix", oscMixSlider);

    osc1DetuneSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    osc1DetuneSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (osc1DetuneSlider);
    osc1DetuneLabel.setText (juce::String ("Osc 1 Detune"), juce::dontSendNotification);
    osc1DetuneLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (osc1DetuneLabel);
    osc1DetuneAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "osc1_detune", osc1DetuneSlider);

    osc2DetuneSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    osc2DetuneSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (osc2DetuneSlider);
    osc2DetuneLabel.setText (juce::String ("Osc 2 Detune"), juce::dontSendNotification);
    osc2DetuneLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (osc2DetuneLabel);
    osc2DetuneAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "osc2_detune", osc2DetuneSlider);

    // --- ADSR ---
    attackSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    attackSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (attackSlider);
    attackLabel.setText (juce::String ("Attack"), juce::dontSendNotification);
    attackLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (attackLabel);
    attackAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "env_attack", attackSlider);

    decaySlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    decaySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (decaySlider);
    decayLabel.setText (juce::String ("Decay"), juce::dontSendNotification);
    decayLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (decayLabel);
    decayAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "env_decay", decaySlider);

    sustainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sustainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (sustainSlider);
    sustainLabel.setText (juce::String ("Sustain"), juce::dontSendNotification);
    sustainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (sustainLabel);
    sustainAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "env_sustain", sustainSlider);

    releaseSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    releaseSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (releaseSlider);
    releaseLabel.setText (juce::String ("Release"), juce::dontSendNotification);
    releaseLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (releaseLabel);
    releaseAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "env_release", releaseSlider);

    // --- LFO ---
    lfoRateSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    lfoRateSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (lfoRateSlider);
    lfoRateLabel.setText (juce::String ("LFO Rate"), juce::dontSendNotification);
    lfoRateLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (lfoRateLabel);
    lfoRateAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "lfo_rate", lfoRateSlider);

    lfoDepthSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    lfoDepthSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (lfoDepthSlider);
    lfoDepthLabel.setText (juce::String ("LFO Depth"), juce::dontSendNotification);
    lfoDepthLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (lfoDepthLabel);
    lfoDepthAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "lfo_depth", lfoDepthSlider);

    // --- FX ---
    cutoffSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    cutoffSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (cutoffSlider);
    cutoffLabel.setText (juce::String ("Cutoff"), juce::dontSendNotification);
    cutoffLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (cutoffLabel);
    cutoffAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "filter_cutoff", cutoffSlider);

    qSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    qSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (qSlider);
    qLabel.setText (juce::String ("Resonance"), juce::dontSendNotification);
    qLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (qLabel);
    qAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "filter_q", qSlider);

    distDriveSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    distDriveSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (distDriveSlider);
    distDriveLabel.setText (juce::String ("Dist Drive"), juce::dontSendNotification);
    distDriveLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (distDriveLabel);
    distDriveAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "dist_drive", distDriveSlider);

    delayTimeSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    delayTimeSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (delayTimeSlider);
    delayTimeLabel.setText (juce::String ("Delay Time"), juce::dontSendNotification);
    delayTimeLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (delayTimeLabel);
    delayTimeAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "delay_time", delayTimeSlider);

    delayFeedbackSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    delayFeedbackSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (delayFeedbackSlider);
    delayFeedbackLabel.setText (juce::String ("Delay FB"), juce::dontSendNotification);
    delayFeedbackLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (delayFeedbackLabel);
    delayFeedbackAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "delay_feedback", delayFeedbackSlider);

    macroLeslieSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    macroLeslieSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (macroLeslieSlider);
    macroLeslieLabel.setText (juce::String ("Leslie Mod"), juce::dontSendNotification);
    macroLeslieLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (macroLeslieLabel);
    macroLeslieAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "macro_leslie", macroLeslieSlider);

    setSize (750, 400);
}

MkManSynthAudioProcessorEditor::~MkManSynthAudioProcessorEditor() {}

void MkManSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF1A1A24));

    g.setColour (juce::Colours::white.withAlpha (0.1f));
    g.drawRect (10, 10, 730, 110, 2); 
    g.drawRect (10, 130, 360, 110, 2); 
    g.drawRect (380, 130, 180, 110, 2); 
    g.drawRect (10, 250, 550, 140, 2); 
    g.drawRect (570, 130, 170, 260, 2); 

    g.setColour (juce::Colours::cyan);
    g.setFont (14.0f);
    g.drawText (juce::String ("OSCILLATORS (UNISON 5+5)"), 20, 15, 300, 20, juce::Justification::left);
    g.drawText (juce::String ("AMPLITUDE ENVELOPE (ADSR)"), 20, 135, 300, 20, juce::Justification::left);
    g.drawText (juce::String ("GLOBAL LFO"), 390, 135, 150, 20, juce::Justification::left);
    g.drawText (juce::String ("FILTER & EFFECTS CHAIN"), 20, 255, 300, 20, juce::Justification::left);
    
    g.setColour (juce::Colours::coral);
    g.drawText (juce::String ("PERFORMANCE MACRO"), 580, 135, 160, 20, juce::Justification::left);
}

void MkManSynthAudioProcessorEditor::resized()
{
    osc1WaveMenu.setBounds (20, 45, 100, 25);
    osc1WaveLabel.setBounds (20, 75, 100, 20);
    
    osc1DetuneSlider.setBounds (130, 35, 80, 80);
    osc1DetuneLabel.setBounds (130, 5, 80, 20);

    oscMixSlider.setBounds (230, 35, 80, 80);
    oscMixLabel.setBounds (230, 5, 80, 20);

    osc2WaveMenu.setBounds (340, 45, 100, 25);
    osc2WaveLabel.setBounds (340, 75, 100, 20);

    osc2DetuneSlider.setBounds (450, 35, 80, 80);
    osc2DetuneLabel.setBounds (450, 5, 80, 20);

    attackSlider.setBounds (20, 155, 80, 80);
    attackLabel.setBounds (20, 135, 80, 20);
    decaySlider.setBounds (105, 155, 80, 80);
    decayLabel.setBounds (105, 135, 80, 20);
    sustainSlider.setBounds (190, 155, 80, 80);
    sustainLabel.setBounds (190, 135, 80, 20);
    releaseSlider.setBounds (275, 155, 80, 80);
    releaseLabel.setBounds (275, 135, 80, 20);

    lfoRateSlider.setBounds (390, 155, 80, 80);
    lfoRateLabel.setBounds (390, 135, 80, 20);
    lfoDepthSlider.setBounds (475, 155, 80, 80);
    lfoDepthLabel.setBounds (475, 135, 80, 20);

    cutoffSlider.setBounds (20, 290, 95, 95);
    cutoffLabel.setBounds (20, 270, 95, 20);
    qSlider.setBounds (125, 290, 95, 95);
    qLabel.setBounds (125, 270, 95, 20);
    distDriveSlider.setBounds (235, 290, 95, 95);
    distDriveLabel.setBounds (235, 270, 95, 20);
    delayTimeSlider.setBounds (345, 290, 95, 95);
    delayTimeLabel.setBounds (345, 270, 95, 20);
    delayFeedbackSlider.setBounds (455, 290, 95, 95);
    delayFeedbackLabel.setBounds (455, 270, 95, 20);

    macroLeslieSlider.setBounds (590, 180, 130, 130);
    macroLeslieLabel.setBounds (590, 315, 130, 20);
}
