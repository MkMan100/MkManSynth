cmake_minimum_required(VERSION 3.22)

# Disattiva gli esempi e i test di JUCE
set(JUCE_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(JUCE_BUILD_EXTRAS OFF CACHE INTERNAL "")

project(MkManSynth VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
)
FetchContent_MakeAvailable(JUCE)

# La configurazione VST3 si imposta direttamente qui dentro in JUCE 7
juce_add_plugin(MkManSynth
    VERSION 1.0.0
    COMPANY_NAME "MkMan"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PRODUCT_NAME "MkManSynth"
    FORMATS VST3 Standalone
    VST3_CATEGORIES Synth
    JUCE_VST3_CAN_REPLACE_VST2 FALSE # Rimuove la dipendenza dai file VST2 mancanti
)

# Cerca i file SOLO dentro la cartella NewProject/Source
file(GLOB_RECURSE SOUND_SOURCES CONFIGURE_DEPENDS 
    "NewProject/Source/*.cpp" 
    "NewProject/Source/*.h"
)

target_sources(MkManSynth PRIVATE ${SOUND_SOURCES})

target_link_libraries(MkManSynth PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
