cmake_minimum_required(VERSION 3.22)

# Fix for macOS 15 SDK compatibility with legacy CoreGraphics APIs in JUCE
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum macOS deployment version" FORCE)
    add_compile_options(-Wno-error=unavailable-declarations -Wno-error=deprecated-declarations -Wno-unavailable-declarations -Wno-deprecated-declarations)
endif()

project(BreakStep VERSION 0.3.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

message(STATUS "Fetching JUCE framework...")
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
)
FetchContent_MakeAvailable(JUCE)

juce_add_gui_app(BreakStep
    PRODUCT_NAME "BreakStep"
    VERSION "0.3.0"
    BUNDLE_IDENTIFIER "cl.dgtl.breakstep"
)

target_sources(BreakStep PRIVATE
    Source/Main.cpp
    Source/MainComponent.h
    Source/MainComponent.cpp
    Source/Audio/VintageSamplerDSP.h
    Source/Audio/StepSequencer.h
    Source/Audio/StepSequencer.cpp
    Source/Audio/DrumTrack.h
    Source/Audio/DrumTrack.cpp
    Source/Audio/AudioEngine.h
    Source/Audio/AudioEngine.cpp
    Source/Audio/SongTimeline.h
    Source/Audio/SongTimeline.cpp
    Source/Audio/StyleTemplates.h
    Source/Audio/StyleTemplates.cpp
    Source/Audio/SampleChopper/AudioSlicer.h
    Source/Audio/SampleChopper/AudioSlicer.cpp
    Source/Audio/SampleChopper/SliceSequencer.h
    Source/Audio/SampleChopper/SliceSequencer.cpp
    Source/UI/CustomLookAndFeel.h
    Source/UI/CustomLookAndFeel.cpp
    Source/UI/RotaryKnob.h
    Source/UI/RotaryKnob.cpp
    Source/UI/HeaderComponent.h
    Source/UI/HeaderComponent.cpp
    Source/UI/TrackRowComponent.h
    Source/UI/TrackRowComponent.cpp
    Source/UI/ChopTrackRowComponent.h
    Source/UI/ChopTrackRowComponent.cpp
    Source/UI/ChopperWaveformComponent.h
    Source/UI/ChopperWaveformComponent.cpp
    Source/UI/SongTimelineComponent.h
    Source/UI/SongTimelineComponent.cpp
)

target_compile_definitions(BreakStep PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_SILENCE_XCODE_15_LINKER_WARNING=1
)

target_link_libraries(BreakStep PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

juce_generate_juce_header(BreakStep)
