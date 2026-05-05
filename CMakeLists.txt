cmake_minimum_required(VERSION 3.22)

project(BinauralJungleForge
    VERSION 0.1.0
    DESCRIPTION "Binaural Jungle Forge — binaural pad/string synth"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS deployment target")
    # Native arch by default during dev. CI overrides with -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64".
endif()

# ───────────────────────────────────────────────────────────────────────────────
# JUCE — pulled at configure time via FetchContent, pinned to a release tag.
# ───────────────────────────────────────────────────────────────────────────────
include(FetchContent)

set(JUCE_VERSION 8.0.12)

FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        ${JUCE_VERSION}
    GIT_SHALLOW    TRUE
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/third_party/JUCE)

FetchContent_MakeAvailable(JUCE)

# ───────────────────────────────────────────────────────────────────────────────
# libmysofa — SOFA file reader for HRTFs. BSD-3-Clause.
# Static-only build, tests off, shared off so we link a single .a into our plugin.
# ───────────────────────────────────────────────────────────────────────────────
set(LIBMYSOFA_VERSION v1.3.4)

set(BUILD_TESTS         OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS   OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS   ON  CACHE BOOL "" FORCE)

FetchContent_Declare(libmysofa
    GIT_REPOSITORY https://github.com/hoene/libmysofa.git
    GIT_TAG        ${LIBMYSOFA_VERSION}
    GIT_SHALLOW    TRUE
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/third_party/libmysofa)

FetchContent_MakeAvailable(libmysofa)

# libmysofa's static target doesn't add its own build dir (where the
# generated mysofa_export.h lives) to its public include path. Patch it
# so dependents can resolve <mysofa.h>'s transitive include.
get_target_property(_mysofa_bin_dir mysofa-static BINARY_DIR)
target_include_directories(mysofa-static
    PUBLIC
        $<BUILD_INTERFACE:${_mysofa_bin_dir}>)

# ───────────────────────────────────────────────────────────────────────────────
# Plugin target
# ───────────────────────────────────────────────────────────────────────────────
juce_add_plugin(BinauralJungleForge
    PRODUCT_NAME                "Binaural Jungle Forge"
    COMPANY_NAME                "Binaural Jungle"
    BUNDLE_ID                   "com.binauraljungle.forge"
    PLUGIN_MANUFACTURER_CODE    BJun
    PLUGIN_CODE                 BjF1
    FORMATS                     VST3 AU
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             "Instrument" "Synth"
    AU_MAIN_TYPE                "kAudioUnitType_MusicDevice")

target_sources(BinauralJungleForge PRIVATE
    src/app/PluginProcessor.cpp
    src/app/PluginEditor.cpp
    src/dsp/spatial/BinauralLSDecoder.cpp
    src/dsp/spatial/DecoderMatrix.cpp
    src/dsp/spatial/HOACoefficients.cpp
    src/dsp/spatial/HOAEncoder.cpp
    src/dsp/spatial/SimpleStereoDecoder.cpp
    src/dsp/spatial/SOFALoader.cpp
    src/dsp/voice/Filter.cpp
    src/dsp/voice/Oscillator.cpp
    src/dsp/voice/OscillatorStack.cpp
    src/dsp/voice/Voice.cpp
    src/dsp/voice/VoiceManager.cpp)

target_include_directories(BinauralJungleForge PRIVATE
    ${CMAKE_SOURCE_DIR}/src)

target_compile_definitions(BinauralJungleForge PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_REPORT_APP_USAGE=0
    BJF_DEFAULT_SOFA_PATH="${CMAKE_SOURCE_DIR}/third_party/libmysofa/share/MIT_KEMAR_normal_pinna.sofa")

target_link_libraries(BinauralJungleForge
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        mysofa-static
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# ───────────────────────────────────────────────────────────────────────────────
# Smoke tests — run after every build to verify the audio path.
# ───────────────────────────────────────────────────────────────────────────────
add_executable(VoiceSmokeTest
    tests/voice_smoke_test.cpp
    src/dsp/voice/Filter.cpp
    src/dsp/voice/Oscillator.cpp
    src/dsp/voice/OscillatorStack.cpp
    src/dsp/voice/Voice.cpp)

target_link_libraries(VoiceSmokeTest PRIVATE
    juce::juce_audio_basics
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

add_executable(VoiceManagerSmokeTest
    tests/voice_manager_smoke_test.cpp
    src/dsp/voice/Filter.cpp
    src/dsp/voice/Oscillator.cpp
    src/dsp/voice/OscillatorStack.cpp
    src/dsp/voice/Voice.cpp
    src/dsp/voice/VoiceManager.cpp)

target_link_libraries(VoiceManagerSmokeTest PRIVATE
    juce::juce_audio_basics
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

add_executable(OscillatorStackSmokeTest
    tests/oscillator_stack_smoke_test.cpp
    src/dsp/voice/Oscillator.cpp
    src/dsp/voice/OscillatorStack.cpp)

target_link_libraries(OscillatorStackSmokeTest PRIVATE
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

add_executable(FilterSmokeTest
    tests/filter_smoke_test.cpp
    src/dsp/voice/Filter.cpp)

target_link_libraries(FilterSmokeTest PRIVATE
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

add_executable(HOACoefficientsSmokeTest
    tests/hoa_coefficients_smoke_test.cpp
    src/dsp/spatial/HOACoefficients.cpp)

target_link_libraries(HOACoefficientsSmokeTest PRIVATE
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

add_executable(SpatialPipelineSmokeTest
    tests/spatial_pipeline_smoke_test.cpp
    src/dsp/spatial/HOACoefficients.cpp
    src/dsp/spatial/HOAEncoder.cpp
    src/dsp/spatial/SimpleStereoDecoder.cpp)

target_link_libraries(SpatialPipelineSmokeTest PRIVATE
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

add_executable(SOFALoaderSmokeTest
    tests/sofa_loader_smoke_test.cpp
    src/dsp/spatial/SOFALoader.cpp)

target_compile_definitions(SOFALoaderSmokeTest PRIVATE
    BJF_DEFAULT_SOFA_PATH="${CMAKE_SOURCE_DIR}/third_party/libmysofa/share/MIT_KEMAR_normal_pinna.sofa")

target_link_libraries(SOFALoaderSmokeTest PRIVATE
    mysofa-static
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

add_executable(BinauralLSDecoderSmokeTest
    tests/binaural_ls_decoder_smoke_test.cpp
    src/dsp/spatial/BinauralLSDecoder.cpp
    src/dsp/spatial/DecoderMatrix.cpp
    src/dsp/spatial/HOACoefficients.cpp
    src/dsp/spatial/HOAEncoder.cpp
    src/dsp/spatial/SOFALoader.cpp)

target_compile_definitions(BinauralLSDecoderSmokeTest PRIVATE
    BJF_DEFAULT_SOFA_PATH="${CMAKE_SOURCE_DIR}/third_party/libmysofa/share/MIT_KEMAR_normal_pinna.sofa")

target_link_libraries(BinauralLSDecoderSmokeTest PRIVATE
    mysofa-static
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
