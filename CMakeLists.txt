cmake_minimum_required(VERSION 3.22)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
)
FetchContent_MakeAvailable(JUCE)

project(MkManSynth VERSION 1.0.0 LANGUAGES CXX)

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

# COMANDO INTELLIGENTE: Cerca tutti i file .cpp e .h nel progetto in automatico
file(GLOB_RECURSE SOUND_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h")

# Passiamo a CMake la lista dei file trovati automaticamente
target_sources(MkManSynth PRIVATE ${SOUND_SOURCES})

target_link_libraries(MkManSynth PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
