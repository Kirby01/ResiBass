#include "PluginProcessor.h"
#include "PluginEditor.h"

ResiBassAudioProcessor::ResiBassAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
        #if ! JucePlugin_IsMidiEffect
         #if ! JucePlugin_IsSynth
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
         #endif
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
        #endif
      ),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ResiBassAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "input", 1 }, "Input",
        juce::NormalisableRange<float> (0.0f, 4.0f, 0.01f), 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "output", 1 }, "Output",
        juce::NormalisableRange<float> (0.0f, 4.0f, 0.01f), 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "drive", 1 }, "Drive",
        juce::NormalisableRange<float> (0.0f, 150.0f, 0.01f), 20.0f));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { "depth", 1 }, "Depth", 8, maxLayers, 48));

    return { params.begin(), params.end() };
}

void ResiBassAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    writePos = 0;
    for (auto& s : states)
        s.reset();
}

void ResiBassAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ResiBassAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (mainOut != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
}
#endif

float ResiBassAudioProcessor::processSample (float x, ChannelState& st, float drive, int layers)
{
    constexpr int step = 6;
    constexpr float lpCut = 260.0f;
    constexpr float hpCut = 45.0f;
    constexpr float sumAmount = 0.25f;
    constexpr float oddLayerBalance = -0.35f;
    constexpr float wetScale = 0.18f;

    st.buffer[(size_t) writePos] = x;

    float wet = 0.0f;
    float wsum = 0.0f;

    layers = juce::jlimit (8, maxLayers, layers);

    for (int k = 0; k < layers; ++k)
    {
        const int delay = k * step;
        const int idx0 = (writePos - delay + bufferSize) & (bufferSize - 1);
        const int idx1 = (writePos - delay - 1 + bufferSize) & (bufferSize - 1);

        const float a = st.buffer[(size_t) idx0];
        const float p = st.buffer[(size_t) idx1];

        const float invDiff = - (a - p);
        const float invSum  = - (a + p) * sumAmount;

        const float w = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * ((float) k + 0.5f) / (float) layers);
        const float sign = (k % 2 == 0) ? 1.0f : oddLayerBalance;

        wet += w * sign * (invDiff + invSum);
        wsum += w;
    }

    wet /= juce::jmax (wsum, 0.000001f);
    wet *= std::sqrt (wsum) * wetScale;

    const float hpA = std::exp (-juce::MathConstants<float>::twoPi * hpCut / (float) currentSampleRate);
    const float hpC = 1.0f - hpA;

    st.hpLp1 = hpC * wet + hpA * st.hpLp1;
    wet -= st.hpLp1;
    st.hpLp2 = hpC * wet + hpA * st.hpLp2;
    wet -= st.hpLp2;

    const float lpA = std::exp (-juce::MathConstants<float>::twoPi * lpCut / (float) currentSampleRate);
    const float c = 1.0f - lpA;

    st.lp1 = c * wet + lpA * st.lp1;
    st.lp2 = c * st.lp1 + lpA * st.lp2;
    st.lp3 = c * st.lp2 + lpA * st.lp3;
    st.lp4 = c * st.lp3 + lpA * st.lp4;
    st.lp5 = c * st.lp4 + lpA * st.lp5;
    st.lp6 = c * st.lp5 + lpA * st.lp6;
    st.lp7 = c * st.lp6 + lpA * st.lp7;
    st.lp8 = c * st.lp7 + lpA * st.lp8;

    return x + drive * st.lp8;
}

void ResiBassAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const float input  = apvts.getRawParameterValue ("input")->load();
    const float output = apvts.getRawParameterValue ("output")->load();
    const float drive  = apvts.getRawParameterValue ("drive")->load();
    const int depth    = (int) apvts.getRawParameterValue ("depth")->load();

    const int numChannels = juce::jmin (2, totalNumOutputChannels);

    for (int n = 0; n < buffer.getNumSamples(); ++n)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float x = buffer.getSample (ch, n) * input;
            const float y = processSample (x, states[(size_t) ch], drive, depth) * output;
            buffer.setSample (ch, n, y);
        }

        if (totalNumOutputChannels == 1 && totalNumInputChannels > 0)
        {
            // mono already handled above
        }

        writePos = (writePos + 1) & (bufferSize - 1);
    }
}

bool ResiBassAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* ResiBassAudioProcessor::createEditor()
{
    return new ResiBassAudioProcessorEditor (*this);
}

void ResiBassAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void ResiBassAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ResiBassAudioProcessor();
}
