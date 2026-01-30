# Gnarly2 Plugin CMakeLists.txt

# Embed web UI files into binary
juce_add_binary_data(gnarly2_WebUI
    SOURCES
        Source/ui/public/index.html
    NAMESPACE gnarly2_BinaryData
)

# Plugin definition
juce_add_plugin(gnarly2
    COMPANY_NAME "NPS"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE NPS0
    PLUGIN_CODE Gnr2
    FORMATS VST3 Standalone
    PRODUCT_NAME "Gnarly2"
    NEEDS_WEBVIEW2 TRUE
)

# Source files
target_sources(gnarly2
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

# Compile definitions
target_compile_definitions(gnarly2
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link libraries
target_link_libraries(gnarly2
    PRIVATE
        gnarly2_WebUI
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
