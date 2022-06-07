/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "NeuralCabinet.h"
//==============================================================================
/**
*/
class NeuralCabAudioProcessor  : public juce::AudioProcessor,public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    NeuralCabAudioProcessor();
    ~NeuralCabAudioProcessor() override;

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
    
    
 
    
    // Sound buffer for plot
     int FFTSize = 2048;
     std::unique_ptr<float[]> bufferToPlot; // Buffer is average of all channels
     std::unique_ptr<float[]> bufferToPlotTemp; // Buffer used to temporary save values
     int gSamplesPerBlock;
     double Fs = 48000; // Default sampling rate
    
     
     // Neural Cabinet
     NeuralCab NeuralC;
     float latentVariables[4] = {0.2,-0.4,-0.5,0.7};
     float normMicDistance = 0.8f;

     // GUI parameters
     juce::AudioProcessorValueTreeState apvts;
     bool isUpdateFromPluginProcessor = false;
     void UpdateLatentSpace(float LatentSpace[4]);
     void UpdateMicNormalizedPosition(float normalizedPosition);
     void parameterChanged(const juce::String &parameterID, float newValue) override;
     void updateGUITransferFunction();

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeuralCabAudioProcessor)
};
