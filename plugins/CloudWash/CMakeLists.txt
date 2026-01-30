# CloudWash - Granular Texture Processor
# WebView-based UI with multi-mode DSP architecture

# Embed web UI files as binary data
juce_add_binary_data(CloudWash_WebUI
    SOURCES
        Source/ui/public/index.html
        Source/ui/public/js/index.js
        Source/ui/public/js/juce/index.js
        Source/ui/public/js/juce/check_native_interop.js
)

# Create the plugin target
juce_add_plugin(CloudWash
    COMPANY_NAME "APC"
    PLUGIN_MANUFACTURER_CODE Apco
    PLUGIN_CODE CldW
    FORMATS VST3 Standalone
    PRODUCT_NAME "CloudWash"
    NEEDS_WEBVIEW2 TRUE
    VST3_CATEGORIES Fx Delay Modulation
)

# Add source files
target_sources(CloudWash
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/dsp/clouds/dsp/granular_processor.cc
        Source/dsp/clouds/resources.cc
        Source/dsp/clouds/dsp/pvoc/phase_vocoder.cc
        Source/dsp/clouds/dsp/pvoc/stft.cc
        Source/dsp/clouds/dsp/pvoc/frame_transformation.cc
        Source/dsp/clouds/dsp/mu_law.cc
        Source/dsp/clouds/dsp/correlator.cc
        Source/dsp/stmlib/dsp/atan.cc
        Source/dsp/stmlib/utils/random.cc
        Source/dsp/stmlib/dsp/units.cc
)

# Include directories for DSP
target_include_directories(CloudWash
    PRIVATE
        Source/dsp
)

# Link JUCE modules
target_link_libraries(CloudWash
    PRIVATE
        CloudWash_WebUI
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# WebView2 compile definitions (CRITICAL for WebView functionality)
target_compile_definitions(CloudWash
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
        JUCE_VST3_CAN_REPLACE_VST2=0
        _USE_MATH_DEFINES=1
)
