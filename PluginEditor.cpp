#include "PluginEditor.h"

ResiBassAudioProcessorEditor::ResiBassAudioProcessorEditor (ResiBassAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setupSlider (inputSlider, inputLabel, "Input");
    setupSlider (outputSlider, outputLabel, "Output");
    setupSlider (driveSlider, driveLabel, "Drive");
    setupSlider (depthSlider, depthLabel, "Depth");

    inputAttachment  = std::make_unique<SliderAttachment> (audioProcessor.apvts, "input", inputSlider);
    outputAttachment = std::make_unique<SliderAttachment> (audioProcessor.apvts, "output", outputSlider);
    driveAttachment  = std::make_unique<SliderAttachment> (audioProcessor.apvts, "drive", driveSlider);
    depthAttachment  = std::make_unique<SliderAttachment> (audioProcessor.apvts, "depth", depthSlider);

    setSize (520, 300);
}

void ResiBassAudioProcessorEditor::setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 22);
    slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
    slider.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (label);
}

void ResiBassAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff111111));

    auto bounds = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff1d1d1d));
    g.fillRoundedRectangle (bounds.reduced (12.0f), 18.0f);

    g.setColour (juce::Colours::orange);
    g.setFont (juce::Font (32.0f, juce::Font::bold));
    g.drawFittedText ("ResiBass", getLocalBounds().removeFromTop (70), juce::Justification::centred, 1);

    g.setColour (juce::Colours::white.withAlpha (0.65f));
    g.setFont (14.0f);
    g.drawFittedText ("Residual low-end weight from difference layers", 0, 52, getWidth(), 24, juce::Justification::centred, 1);
}

void ResiBassAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (24);
    area.removeFromTop (80);

    const int knobW = area.getWidth() / 4;
    auto row = area.removeFromTop (170);

    auto place = [&] (juce::Slider& slider, juce::Label& label, juce::Rectangle<int> r)
    {
        label.setBounds (r.removeFromTop (24));
        slider.setBounds (r.reduced (8));
    };

    place (inputSlider,  inputLabel,  row.removeFromLeft (knobW));
    place (driveSlider,  driveLabel,  row.removeFromLeft (knobW));
    place (depthSlider,  depthLabel,  row.removeFromLeft (knobW));
    place (outputSlider, outputLabel, row.removeFromLeft (knobW));
}
