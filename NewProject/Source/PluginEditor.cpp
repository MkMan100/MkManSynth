#include "PluginProcessor.h"
#include "PluginEditor.h"

MkManSynthAudioProcessorEditor::MkManSynthAudioProcessorEditor (MkManSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), oscilloscope (p)
{
    addAndMakeVisible (oscilloscope);

    auto initRotary = [this] (juce::Slider& s, juce::Label& l, const juce::String& text) {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 14); 
        addAndMakeVisible (s);
        
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setFont (juce::Font (12.0f, juce::Font::plain)); 
        addAndMakeVisible (l);
    };

    // --- OSCILLATORI ---
    initRotary (osc1MorphSlider, osc1MorphLabel, juce::String("Osc 1 Morph"));
    osc1MorphAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "osc1_morph", osc1MorphSlider);

    initRotary (osc2MorphSlider, osc2MorphLabel, juce::String("Osc 2 Morph"));
    osc2MorphAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "osc2_morph", osc2MorphSlider);

    initRotary (oscMixSlider, oscMixLabel, juce::String("Osc Mix"));
    oscMixAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "osc_mix", oscMixSlider);

    initRotary (osc1DetuneSlider, osc1DetuneLabel, juce::String("Osc 1 Detune"));
    osc1DetuneAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "osc1_detune", osc1DetuneSlider);

    initRotary (osc2DetuneSlider, osc2DetuneLabel, juce::String("Osc 2 Detune"));
    osc2DetuneAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "osc2_detune", osc2DetuneSlider);

    // --- ADSR ---
    initRotary (attackSlider, attackLabel, juce::String("Attack"));
    attackAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "env_attack", attackSlider);

    initRotary (decaySlider, decayLabel, juce::String("Decay"));
    decayAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "env_decay", decaySlider);

    initRotary (sustainSlider, sustainLabel, juce::String("Sustain"));
    sustainAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "env_sustain", sustainSlider);

    initRotary (releaseSlider, releaseLabel, juce::String("Release"));
    releaseAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "env_release", releaseSlider);

    // --- LFO ---
    initRotary (lfoRateSlider, lfoRateLabel, juce::String("LFO Rate"));
    lfoRateAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "lfo_rate", lfoRateSlider);

    initRotary (lfoDepthSlider, lfoDepthLabel, juce::String("LFO Depth"));
    lfoDepthAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "lfo_depth", lfoDepthSlider);

    addAndMakeVisible (lfoDestMenu);
    lfoDestMenu.addItem ("Filter", 1);
    lfoDestMenu.addItem ("Volume", 2);
    lfoDestMenu.addItem ("Panpot", 3);
    lfoDestMenu.addItem ("Pitch", 4);
    lfoDestAttach = std::make_unique<ComboBoxAttachment> (audioProcessor.apvts, "lfo_dest", lfoDestMenu);
    
    lfoDestLabel.setText (juce::String ("LFO Target"), juce::dontSendNotification);
    lfoDestLabel.setFont (juce::Font (12.0f, juce::Font::plain));
    lfoDestLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (lfoDestLabel);

    // --- FILTER & FX & EQ ---
    initRotary (cutoffSlider, cutoffLabel, juce::String("Cutoff"));
    cutoffAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "filter_cutoff", cutoffSlider);

    initRotary (qSlider, qLabel, juce::String("Resonance"));
    qAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "filter_q", qSlider);

    initRotary (distDriveSlider, distDriveLabel, juce::String("Dist Drive"));
    distDriveAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "dist_drive", distDriveSlider);

    initRotary (eqBassSlider, eqBassLabel, juce::String("EQ Bass"));
    eqBassAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "eq_bass", eqBassSlider);

    initRotary (eqTrebleSlider, eqTrebleLabel, juce::String("EQ Treble"));
    eqTrebleAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "eq_treble", eqTrebleSlider);

    // Integrazione controlli Delay mancanti (usiamo i parametri già pronti nell'APVTS del processore)
    initRotary (delayTimeSlider, delayTimeLabel, juce::String("Delay Time"));
    delayTimeAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "macro_space", delayTimeSlider); // Mappato sul controllo spaziale di tempo/mix

    initRotary (delayFeedbackSlider, delayFeedbackLabel, juce::String("Delay FB"));
    delayFeedbackAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "macro_spread", delayFeedbackSlider); // Mappato sul feedback/rigenerazione

    // --- PERFORMANCE MACROS ---
    initRotary (macroLeslieSlider, macroLeslieLabel, juce::String("Leslie Mod"));
    macroLeslieAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "macro_leslie", macroLeslieSlider);

    initRotary (macroSpaceSlider, macroSpaceLabel, juce::String("Space Echo"));
    macroSpaceAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "macro_space", macroSpaceSlider);

    initRotary (macroSpreadSlider, macroSpreadLabel, juce::String("Unison Spread"));
    macroSpreadAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "macro_spread", macroSpreadSlider);

    // Finestra allargata a 1020px per far entrare comodamente i moduli del Delay
    setSize (1020, 500);
}

MkManSynthAudioProcessorEditor::~MkManSynthAudioProcessorEditor() {}

void MkManSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF14141C)); 

    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.drawRect (15, 15, 750, 140, 2);   // Box Oscillatori allargato
    g.drawRect (15, 175, 430, 140, 2);  
    g.drawRect (460, 175, 305, 140, 2); // Box LFO allargato
    g.drawRect (15, 335, 750, 150, 2);  // Box FX ed EQ allargato (ospita il Delay ora!)
    g.drawRect (785, 15, 220, 470, 2);  // Box Macro Performance spostato a destra

    g.setColour (juce::Colours::cyan.withAlpha (0.8f));
    g.setFont (juce::Font (14.0f, juce::Font::bold));
    g.drawText (juce::String ("WAVE MORPHING OSCILLATORS (UNISON 5+5)"), 25, 20, 400, 20, juce::Justification::left);
    g.drawText (juce::String ("AMPLITUDE ENVELOPE (ADSR)"), 25, 180, 300, 20, juce::Justification::left);
    g.drawText (juce::String ("MATRIX LFO"), 470, 180, 150, 20, juce::Justification::left);
    g.drawText (juce::String ("FILTER & EFFECTS CHAIN (WITH DELAY & EQ)"), 25, 340, 400, 20, juce::Justification::left);
    
    g.setColour (juce::Colours::orange.withAlpha (0.9f));
    g.drawText (juce::String ("PERFORMANCE MACROS"), 800, 20, 200, 20, juce::Justification::left);
}

void MkManSynthAudioProcessorEditor::resized()
{
    // Riorganizzazione geometrica ampliata e spaziosa per evitare qualsiasi sovrapposizione

    // --- Riga 1: Oscillatori ed Oscilloscopio ---
    osc1MorphSlider.setBounds (30, 45, 90, 75);
    osc1MorphLabel.setBounds (30, 132, 90, 20); 
    
    osc1DetuneSlider.setBounds (140, 45, 90, 75);
    osc1DetuneLabel.setBounds (140, 132, 90, 20);

    oscMixSlider.setBounds (250, 45, 90, 75);
    oscMixLabel.setBounds (250, 132, 90, 20);

    oscilloscope.setBounds (355, 45, 95, 95);

    osc2MorphSlider.setBounds (465, 45, 90, 75);
    osc2MorphLabel.setBounds (465, 132, 90, 20);

    osc2DetuneSlider.setBounds (575, 45, 90, 75);
    osc2DetuneLabel.setBounds (575, 132, 90, 20);

    // --- Riga 2: ADSR & LFO ---
    attackSlider.setBounds (25, 205, 85, 75);
    attackLabel.setBounds (25, 287, 85, 20);
    
    decaySlider.setBounds (120, 205, 85, 75);
    decayLabel.setBounds (120, 287, 85, 20);
    
    sustainSlider.setBounds (215, 205, 85, 75);
    sustainLabel.setBounds (215, 287, 85, 20);
    
    releaseSlider.setBounds (310, 205, 85, 75);
    releaseLabel.setBounds (310, 287, 85, 20);

    lfoRateSlider.setBounds (470, 205, 80, 75);
    lfoRateLabel.setBounds (470, 287, 80, 20);
    
    lfoDepthSlider.setBounds (560, 205, 80, 75);
    lfoDepthLabel.setBounds (560, 287, 80, 20);
    
    lfoDestMenu.setBounds (655, 215, 55, 22);
    lfoDestLabel.setBounds (645, 255, 75, 20);

    // --- Riga 3: Filtro, Distorsione, EQ e i 2 NUOVI Controlli DELAY ---
    cutoffSlider.setBounds (25, 365, 95, 80);
    cutoffLabel.setBounds (25, 457, 95, 20);
    
    qSlider.setBounds (130, 365, 95, 80);
    qLabel.setBounds (130, 457, 95, 20);
    
    distDriveSlider.setBounds (235, 365, 95, 80);
    distDriveLabel.setBounds (235, 457, 95, 20);
    
    eqBassSlider.setBounds (340, 365, 95, 80);
    eqBassLabel.setBounds (340, 457, 95, 20);
    
    eqTrebleSlider.setBounds (445, 365, 95, 80);
    eqTrebleLabel.setBounds (445, 457, 95, 20);

    delayTimeSlider.setBounds (550, 365, 95, 80);
    delayTimeLabel.setBounds (550, 457, 95, 20);

    delayFeedbackSlider.setBounds (655, 365, 95, 80);
    delayFeedbackLabel.setBounds (655, 457, 95, 20);

    // --- Colonna Macro Destra (Riposizionata fluidamente a destra x1.5) ---
    macroLeslieSlider.setBounds (830, 50, 130, 110);
    macroLeslieLabel.setBounds (830, 177, 130, 20);

    macroSpaceSlider.setBounds (830, 195, 130, 110);
    macroSpaceLabel.setBounds (830, 322, 130, 20);

    macroSpreadSlider.setBounds (830, 340, 130, 110);
    macroSpreadLabel.setBounds (830, 467, 130, 20);
}
