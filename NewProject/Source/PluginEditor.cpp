#include "PluginProcessor.h"
#include "PluginEditor.h"

MkManSynthAudioProcessorEditor::MkManSynthAudioProcessorEditor (MkManSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Allarghiamo la finestra per far stare comodamente tutti i controlli
    setSize (750, 400);

    // Funzione helper lambda per configurare rapidamente le manopole
    auto setupKnob = [this] (juce::Slider& slider, std::unique_ptr<Attachment>& attach, juce::String paramID)
    {
        slider.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (slider);
        attach = std::make_unique<Attachment> (audioProcessor.apvts, paramID, slider);
    };

    // Inizializzazione di tutti i controlli tramite gli ID definiti nel processore
    setupKnob (oscMixSlider, oscMixAttachment, "osc_mix");
    setupKnob (filterCutoffSlider, filterCutoffAttachment, "filter_cutoff");
    setupKnob (filterQSlider, filterQAttachment, "filter_q");
    setupKnob (distDriveSlider, distDriveAttachment, "dist_drive");
    setupKnob (delayTimeSlider, delayTimeAttachment, "delay_time");
    setupKnob (delayFeedbackSlider, delayFeedbackAttachment, "delay_feedback");
    setupKnob (macro1Slider, macro1Attachment, "macro_dark_reverb");
    setupKnob (macro2Slider, macro2Attachment, "macro_cyber_punch");
    setupKnob (macro3Slider, macro3Attachment, "macro_leslie");
}

MkManSynthAudioProcessorEditor::~MkManSynthAudioProcessorEditor() {}

void MkManSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Sfondo scuro cyberpunk
    g.fillAll (juce::Colour (0xFF151515));

    // Intestazione
    g.setColour (juce::Colours::cyan);
    g.setFont (24.0f);
    g.drawFittedText ("MkMan Synth - Master Engine", getLocalBounds().removeFromTop (40), juce::Justification::centred, 1);

    // Disegniamo delle linee di divisione estetiche per le sezioni
    g.setColour (juce::Colour (0xFF333333));
    g.drawRect (20, 60, 210, 310, 1);  // Sezione SYNTH
    g.drawRect (250, 60, 230, 310, 1); // Sezione FX
    g.drawRect (500, 60, 230, 310, 1); // Sezione MACROS

    // Etichette delle Sezioni
    g.setColour (juce::Colours::cyan);
    g.setFont (16.0f);
    g.drawText ("SYNTH ENGINE", 20, 70, 210, 20, juce::Justification::centred);
    g.drawText ("FX RACK", 250, 70, 230, 20, juce::Justification::centred);
    g.drawText ("PERFORMANCE MACROS", 500, 70, 230, 20, juce::Justification::centred);

    // Etichette dei singoli controlli
    g.setColour (juce::Colours::white);
    g.setFont (13.0f);
    
    // Riga Synth
    g.drawText ("Osc Mix", 75, 200, 100, 20, juce::Justification::centred);
    g.drawText ("Cutoff", 25, 330, 100, 20, juce::Justification::centred);
    g.drawText ("Resonance", 125, 330, 100, 20, juce::Justification::centred);

    // Riga FX
    g.drawText ("Dist Drive", 315, 200, 100, 20, juce::Justification::centred);
    g.drawText ("Delay Time", 265, 330, 100, 20, juce::Justification::centred);
    g.drawText ("Feedback", 365, 330, 100, 20, juce::Justification::centred);

    // Riga Macro
    g.drawText ("Dark Reverb", 515, 200, 100, 20, juce::Justification::centred);
    g.drawText ("Cyber Punch", 615, 200, 100, 20, juce::Justification::centred);
    g.drawText ("Leslie Speed", 565, 330, 100, 20, juce::Justification::centred);
}

void MkManSynthAudioProcessorEditor::resized()
{
    // Organizzazione spaziale delle manopole (X, Y, Larghezza, Altezza)
    
    // --- COLONNA SYNTH ---
    oscMixSlider.setBounds (75, 110, 100, 100);
    filterCutoffSlider.setBounds (25, 240, 100, 100);
    filterQSlider.setBounds (125, 240, 100, 100);

    // --- COLONNA FX ---
    distDriveSlider.setBounds (315, 110, 100, 100);
    delayTimeSlider.setBounds (265, 240, 100, 100);
    delayFeedbackSlider.setBounds (365, 240, 100, 100);

    // --- COLONNA MACROS ---
    macro1Slider.setBounds (515, 110, 100, 100);
    macro2Slider.setBounds (615, 110, 100, 100);
    macro3Slider.setBounds (565, 240, 100, 100);
}