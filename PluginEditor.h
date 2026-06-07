#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class ResiBassAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
   explicit ResiBassAudioProcessorEditor (ResiBassAudioProcessor&);
    ~ResiBassAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ResiBassAudioProcessor& audioProcessor;

    juce::Slider inputSlider, outputSlider, driveSlider, depthSlider;
    juce::Label inputLabel, outputLabel, driveLabel, depthLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> inputAttachment, outputAttachment, driveAttachment, depthAttachment;

    void setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResiBassAudioProcessorEditor)
};
