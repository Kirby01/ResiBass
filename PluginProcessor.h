#pragma once

#include <JuceHeader.h>

class ResiBassAudioProcessor  : public juce::AudioProcessor
{
public:
    ResiBassAudioProcessor();
    ~ResiBassAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    static constexpr int maxLayers = 128;
    static constexpr int bufferSize = 4096;

    struct ChannelState
    {
        std::array<float, bufferSize> buffer{};
        float hpLp1 = 0.0f, hpLp2 = 0.0f;
        float lp1 = 0.0f, lp2 = 0.0f, lp3 = 0.0f, lp4 = 0.0f;
        float lp5 = 0.0f, lp6 = 0.0f, lp7 = 0.0f, lp8 = 0.0f;

        void reset()
        {
            buffer.fill (0.0f);
            hpLp1 = hpLp2 = 0.0f;
            lp1 = lp2 = lp3 = lp4 = lp5 = lp6 = lp7 = lp8 = 0.0f;
        }
    };

    std::array<ChannelState, 2> states;
    int writePos = 0;
    double currentSampleRate = 44100.0;

    float processSample (float x, ChannelState& st, float drive, int layers);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResiBassAudioProcessor)
};
