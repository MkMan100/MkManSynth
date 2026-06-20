#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MkManSynthAudioProcessorEditor::MkManSynthAudioProcessorEditor (MkManSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // 1. Definiamo le dimensioni iniziali della finestra del VST (Larghezza, Altezza)
    setSize (600, 300);

    // --- CONFIGURAZIONE MACRO 1 (Dark Reverb) ---
    macro1Slider.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    macro1Slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0); // Nascondiamo il box di testo standard
    addAndMakeVisible (macro1Slider); // Rende visibile il componente a schermo
    
    // Colleghiamo la manopola alla stringa "macro_dark_reverb" definita nell'APVTS
    macro1Attachment = std::make_unique<Attachment> (audioProcessor.apvts, "macro_dark_reverb", macro1Slider);

    // --- CONFIGURAZIONE MACRO 2 (Cyber Punch) ---
    macro2Slider.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    macro2Slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (macro2Slider);
    macro2Attachment = std::make_unique<Attachment> (audioProcessor.apvts, "macro_cyber_punch", macro2Slider);

    // --- CONFIGURAZIONE MACRO 3 (Leslie Simulator) ---
    macro3Slider.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    macro3Slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (macro3Slider);
    macro3Attachment = std::make_unique<Attachment> (audioProcessor.apvts, "macro_leslie", macro3Slider);
}

MkManSynthAudioProcessorEditor::~MkManSynthAudioProcessorEditor()
{
}

//==============================================================================
void MkManSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Sfondo scuro in stile Synth Cyberpunk/Moderno
    g.fillAll (juce::Colour (0xFF1A1A1A));

    // Scritta del titolo
    g.setColour (juce::Colours::cyan);
    g.setFont (24.0f);
    g.drawFittedText ("MkMan Synth - Master Engine", getLocalBounds().removeFromTop (50), juce::Justification::centred, 1);

    // Etichette sotto le manopole
    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    
    // Dividiamo lo spazio per scrivere i testi sotto ogni manopola
    g.drawText ("Dark Reverb",  100, 190, 100, 20, juce::Justification::centred);
    g.drawText ("Cyber Punch",  250, 190, 100, 20, juce::Justification::centred);
    g.drawText ("Leslie Speed", 400, 190, 100, 20, juce::Justification::centred);
}

void MkManSynthAudioProcessorEditor::cooledResized()
{
    // Posizioniamo fisicamente le 3 manopole a schermo (X, Y, Larghezza, Altezza)
    // Le allineiamo in orizzontale distanziate uniformemente
    macro1Slider.setBounds (100, 80, 100, 100);
    macro2Slider.setBounds (250, 80, 100, 100);
    macro3Slider.setBounds (400, 80, 100, 100);
}