#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <mutex>
#include <atomic>

#include "clouds/dsp/granular_processor.h"
#include "clouds/dsp/frame.h"
#include "clouds/dsp/sample_rate_converter.h"
#include "clouds/resources.h"

//==============================================================================
/**
 * CloudWash - Granular Texture Processor
 *
 * Authentic port of Mutable Instruments Clouds DSP.
 */
class CloudWashAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    CloudWashAudioProcessor();
    ~CloudWashAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameter Value Tree State (APVTS)
    juce::AudioProcessorValueTreeState apvts;

    //==============================================================================
    // AUDIO METERING & VISUALIZATION DATA
    //==============================================================================
    std::atomic<float> inputPeakLevel { 0.0f };
    std::atomic<float> outputPeakLevel { 0.0f };
    float inputPeakHold { 0.0f };    // Peak hold with decay
    float outputPeakHold { 0.0f };   // Peak hold with decay

    std::atomic<int> activeGrainCount { 0 };
    std::atomic<float> grainDensityViz { 0.0f };
    std::atomic<float> grainTextureViz { 0.0f };

    // Mode and Quality mapping helper
    static int getNumQualityModes() { return 5; }
    static juce::String getQualityModeName(int index);

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    // CLOUDS DSP
    //==============================================================================

    // Memory blocks for the processor (use heap allocation like VCV Rack)
    uint8_t* block_mem = nullptr;
    uint8_t* block_ccm = nullptr;

    // Use pointer and heap allocation (matches VCV Rack pattern)
    clouds::GranularProcessor* processor = nullptr;
    
    // Resampling state (Host SR -> 32kHz -> Host SR)
    juce::AudioBuffer<float> resampledInputBuffer;
    juce::AudioBuffer<float> resampledOutputBuffer;
    
    // Use original Clouds SampleRateConverter for authentic sound
    // instead of JUCE's LagrangeInterpolator
    clouds::SampleRateConverter<-2, 45, clouds::src_filter_1x_2_45> inputResamplers[2];
    clouds::SampleRateConverter<+2, 45, clouds::src_filter_1x_2_45> outputResamplers[2];

    // Internal buffers for Clouds (ShortFrame)
    std::vector<clouds::ShortFrame> inputFrames;
    std::vector<clouds::ShortFrame> outputFrames;

    bool isFrozen { false };

    double hostSampleRate = 44100.0;
    double internalSampleRate = 32000.0;

    // High fidelity mixing buffer
    juce::AudioBuffer<float> dryBuffer;

    // Thread safety for DSP re-initialization
    std::mutex processorMutex;

    // Quality/Mode change handling (to prevent audio thread blocking)
    // All atomic for thread safety (parameters can change from message thread)
    std::atomic<int> pendingMode { -1 };
    std::atomic<int> pendingQuality { -1 };
    std::atomic<int> silenceBlocksRemaining { 0 };
    std::atomic<int> currentMode { 0 };
    std::atomic<int> currentQuality { 0 };
    std::atomic<bool> cloudsInitialized { false };  // Track if Clouds processor is initialized

    // Preset management
    struct PresetData {
        juce::String name;
        std::map<juce::String, float> parameters;
    };
    std::vector<PresetData> presets;
    int currentPresetIndex { 0 };
    void initializePresets();
    void loadPreset(int index);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CloudWashAudioProcessor)
};
