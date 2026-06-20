#include "PluginProcessor.h"
#include "PluginEditor.h"

MkManSynthAudioProcessorEditor::MkManSynthAudioProcessorEditor (MkManSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Funzione helper per configurare rapidamente gli slider rotativi (pomelli)
    auto setupRotarySlider = [this] (juce::Slider& slider, juce::Label& label, const juce::String& text) {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
        addAndMakeVisible (slider);
        
        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::Font (12.0f));
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
    };

    // --- OSCILLATORI ---
    addAndMakeVisible (osc1WaveMenu);
    osc1WaveMenu.addItemList (juce::StringArray{"Sine", "Saw", "Square", "Triangle"}, 1);
    osc1WaveAttach = std::make_unique<ComboBoxAttachment> (audioProcessor.apvts, "osc1_wave", osc1WaveMenu);
    osc1WaveLabel.setText ("Osc 1 Wave", juce::dontSendNotification);
    addAndMakeVisible (osc1WaveLabel);

    addAndMakeVisible (osc2WaveMenu);
    osc2WaveMenu.addItemList (juce::StringArray{"Sine", "Saw", "Square", "Triangle"}, 1);
    osc2WaveAttach = std::make_unique<ComboBoxAttachment> (audioProcessor.apvts, "osc2_wave", osc2WaveMenu);
    osc2WaveLabel.setText ("Osc 2 Wave", juce::dontSendNotification);
    addAndMakeVisible (osc2WaveLabel);

    setupRotarySlider (oscMixSlider, oscMixLabel, "Osc Mix");
    oscMixAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "osc_mix", oscMixSlider);

    setupRotarySlider (osc1DetuneSlider, osc1DetuneLabel, "Osc 1 Detune");
    osc1DetuneAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "osc1_detune", osc1DetuneSlider);

    setupRotarySlider (osc2DetuneSlider, osc2DetuneLabel, "Osc 2 Detune");
    osc2DetuneAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "osc2_detune", osc2DetuneSlider);

    // --- INVILUPPO (ADSR) ---
    setupRotarySlider (attackSlider, attackLabel, "Attack");
    attackAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "env_attack", attackSlider);

    setupRotarySlider (decaySlider, decayLabel, "Decay");
    decayAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "env_decay", decaySlider);

    setupRotarySlider (sustainSlider, sustainLabel, "Sustain");
    sustainAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "env_sustain", sustainSlider);

    setupRotarySlider (releaseSlider, releaseLabel, "Release");
    releaseAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "env_release", releaseSlider);

    // --- LFO ---
    setupRotarySlider (lfoRateSlider, lfoRateLabel, "LFO Rate");
    lfoRateAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "lfo_rate", lfoRateSlider);

    setupRotarySlider (lfoDepthSlider, lfoDepthLabel, "LFO Depth");
    lfoDepthAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "lfo_depth", lfoDepthSlider);

    // --- FILTRO ED EFFETTI ---
    setupRotarySlider (cutoffSlider, cutoffLabel, "Cutoff");
    cutoffAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "filter_cutoff", cutoffSlider);

    setupRotarySlider (qSlider, qLabel, "Resonance");
    qAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "filter_q", qSlider);

    setupRotarySlider (distDriveSlider, distDriveLabel, "Dist Drive");
    distDriveAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "dist_drive", distDriveSlider);

    setupRotarySlider (delayTimeSlider, delayTimeLabel, "Delay Time");
    delayTimeAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "delay_time", delayTimeSlider);

    setupRotarySlider (delayFeedbackSlider, delayFeedbackLabel, "Delay FB");
    delayFeedbackAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "delay_feedback", delayFeedbackSlider);

    setupRotarySlider (macroLeslieSlider, macroLeslieLabel, "Leslie Mod");
    macroLeslieAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "macro_leslie", macroLeslieSlider);

    // Dimensioni della finestra dell'interfaccia grafica
    setSize (750, 400);
}

MkManSynthAudioProcessorEditor::~MkManSynthAudioProcessorEditor() {}

void MkManSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Sfondo scuro in stile cyberpunk / moderno
    g.fillAll (juce::Colour (0xFF1A1A24));

    g.setColour (juce::Colours::white.withAlpha (0.1f));
    g.drawRect (10, 10, 730, 110, 2); // Box Oscillatori
    g.drawRect (10, 130, 360, 110, 2); // Box ADSR
    g.drawRect (380, 130, 180, 110, 2); // Box LFO
    g.drawRect (10, 250, 550, 140, 2); // Box FX & Filtro
    g.drawRect (570, 130, 170, 260, 2); // Box Macro Speciale

    // Titoli delle sezioni
    g.setColour (juce::Colours::cyan);
    g.setFont (juce::Font (14.0f, juce::Font::bold));
    g.drawText ("OSCILLATORS (UNISON 5+5)", 20, 15, 300, 20, juce::Justification::left);
    g.drawText ("AMPLITUDE ENVELOPE (ADSR)", 20, 135, 300, 20, juce::Justification::left);
    g.drawText ("GLOBAL LFO", 390, 135, 150, 20, juce::Justification::left);
    g.drawText ("FILTER & EFFECTS CHAIN", 20, 255, 300, 20, juce::Justification::left);
    
    g.setColour (juce::Colours::coral);
    g.drawText ("PERFORMANCE MACRO", 580, 135, 160, 20, juce::Justification::left);
}

void MkManSynthAudioProcessorEditor::resized()
{
    // Posizionamento geometrico preciso dei moduli (Griglia 750x400)
    
    // Riga 1: Oscillatori
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

    // Riga 2: ADSR & LFO
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

    // Riga 3: Filtro ed Effetti
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

    // Colonna Destra: Macro Leslie
    macroLeslieSlider.setBounds (590, 180, 130, 130);
    macroLeslieLabel.setBounds (590, 315, 130, 20);
}
