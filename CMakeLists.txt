cmake_minimum_required(VERSION 3.22)

# Scarica JUCE automaticamente sul server di GitHub
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12 # Versione stabile di JUCE 7
)
FetchContent_MakeAvailable(JUCE)

project(MkManSynth VERSION 1.0.0 LANGUAGES CXX)

# Attiva i formati VST3 e Standalone
juce_add_plugin(MkManSynth
    VERSION 1.0.0
    COMPANY_NAME "MkMan"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PRODUCT_NAME "MkManSynth"
    FORMATS VST3 Standalone
)

# Dice a CMake dove trovare i tuoi file sorgente
target_sources(MkManSynth PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/SynthVoice.cpp
    Source/SynthVoice.h
)

# Collega i moduli di JUCE necessari al tuo codice (inclusi DSP e GUI)
target_link_libraries(MkManSynth PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
