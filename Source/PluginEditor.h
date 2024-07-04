/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// Put slider in the constructor 
struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox)
    {

    }
};

//==============================================================================
/**
*/
class SimpleEQAudioProcessorEditor  : public juce::AudioProcessorEditor,
    // inherit from audio processor listener 
juce::AudioProcessorParameter::Listener,  // Listener needs to be threadsafe and fast 
// inherit timer for atomic counting 
juce::Timer
{
public:
    SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // From the Listener class
     void parameterValueChanged(int parameterIndex, float newValue) override;

    virtual void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}

    // From Timer class
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEQAudioProcessor& audioProcessor;

    juce::Atomic<bool> parametersChanged{ false };

    CustomRotarySlider peakFreqSlider,
        peakGainSlider,
        peakQualitySlider,
        lowCutFreqSlider,
        highCutFreqSlider,
        lowCutSlopeSlider,
        highCutSlopeSlider;


    // aptvs has a helper class to attach sliders but it's got a long name 
    using APTVS = juce::AudioProcessorValueTreeState;
    using Attachment = APTVS::SliderAttachment;

    // Declare attachment for all sliders
    Attachment peakFreqSliderAttachment,
        peakGainSliderAttachment,
        peakQualitySliderAttachment,
        lowCutFreqSliderAttachment,
        highCutFreqSliderAttachment,
        lowCutSlopeSliderAttachment,
        highCutSlopeSliderAttachment;

    std::vector<juce::Component*> getComps();

    // Monochain for response curve
    MonoChain MonoChain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessorEditor)
};
