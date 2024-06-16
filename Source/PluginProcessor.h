/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48,

};

// Data structure for all parameter values
struct ChainSettings
{
    float peakFreq{ 0 }, peakGainInDecibels{ 0 }, peakQuality{ 1.f };
    float lowCutFreq{ 0 }, highCutFreq{ 0 };
    Slope lowCutSlope{Slope::Slope_12}, highCutSlope{Slope::Slope_12};
};

// Helper function to get the parameter values in the data struct
ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class SimpleEQAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    // Cannot glitch when in this process block 
    // can't interupt this process block

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

    // Added Audio Processor value tree state
    static juce::AudioProcessorValueTreeState::ParameterLayout
        createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr,
        "Parameters", createParameterLayout()};

private:

    // 'using x = juce::dsp::IIR::Filter<float>' creates an alias. Instead of writing the whole thing out again  
    using Filter = juce::dsp::IIR::Filter<float>; // using allows you to specify a namespace
    // An important concept in the DSP framework is to have a processor chain and pass in a processorContext 

    //IIR has -12dB / Octave. So for -48dB we need 4 filters 

    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

    // Can configure filters to be different types of filters
    // One filter can represent parametric filters 
    // define a chain for the whole signal path
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

    //Adjust cutoff, gain, slope
    // Need 2 instances for stereo 

    MonoChain leftChain, rightChain; 

    enum ChainPositions
    {
        LowCut,
        Peak,
        HighCut
    };

    void updatePeakFilter(const ChainSettings& chainSettings);

    // create an alias
    using Coefficients = Filter::CoefficientsPtr;
    // doesn't use any variables so static
    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);

    template<typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& leftLowCut,
        const CoefficientType& cutCoefficients,
//        const ChainSettings& chainSettings
        const Slope& lowCutSlope)
    {
        //auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, sampleRate, 2 * (chainSettings.lowCutSlope + 1));

        //auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();

        // 4 positions so bypass all 4
        leftLowCut.template setBypassed<0>(true);
        leftLowCut.template setBypassed<1>(true);
        leftLowCut.template setBypassed<2>(true);
        leftLowCut.template setBypassed<3>(true);

        // want to switch based on slope setting
        switch (lowCutSlope)
        {
        case Slope_12:
        {
            *leftLowCut.template get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.template setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *leftLowCut.template get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.template setBypassed<0>(false);
            *leftLowCut.template get<1>().coefficients = *cutCoefficients[0];
            leftLowCut.template  setBypassed<1>(false);
            break;
        }
        case Slope_36:
        {
            *leftLowCut.template get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.template setBypassed<0>(false);
            *leftLowCut.template get<1>().coefficients = *cutCoefficients[0];
            leftLowCut.template setBypassed<1>(false);
            *leftLowCut.template get<2>().coefficients = *cutCoefficients[0];
            leftLowCut.template setBypassed<2>(false);
            break;
        }
        case Slope_48:
        {
            *leftLowCut.template get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.template setBypassed<0>(false);
            *leftLowCut.template get<1>().coefficients = *cutCoefficients[0];
            leftLowCut.template setBypassed<1>(false);
            *leftLowCut.template get<2>().coefficients = *cutCoefficients[0];
            leftLowCut.template setBypassed<2>(false);
            *leftLowCut.template get<3>().coefficients = *cutCoefficients[0];
            leftLowCut.template setBypassed<3>(false);
            break;
        }
        }
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};
