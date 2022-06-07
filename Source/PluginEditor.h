/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SpectrumComponent.h"

//==============================================================================

/**
*/
class NeuralCabAudioProcessorEditor  : public juce::AudioProcessorEditor,
private juce::Timer
{
public:
    NeuralCabAudioProcessorEditor (NeuralCabAudioProcessor&);
    ~NeuralCabAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    SpectrumComponent Spec;
    void mouseUp(const juce::MouseEvent &event) override;
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void updateModelLatentSpace();
    void timerCallback() override;
    void updatePlotFromGivenLatentSpace(float latentSpace[4]);
    void updatePlotFromGivenRadius(float normMicDistance);

    
private:
    
    // Latent circles positions
    int Latent12[2];
    int Latent34[2];
    
    int PointMicPosition[2];
    
    int Latentxi;
    int Latentxe;
    int Latentyi;
    int Latentye;
    
    int Micxi;
    int Micxe;
    int Micyi;
    int Micye;
    
    int pad = 20;
    int padBoarder = 5;
    float ScalingLatentSpace = 3.0f;
    int EllipseSize  = 30;
    
    enum ActualMouseSelection{None,FirstLatent,SecondLatent,Microphone};
    ActualMouseSelection ActualSelected;
    
    // Colors
    float c1[3] = {0.23137255, 0.23137255, 0.24313725}; // Black
    float c2[3] = {0.32941176, 0.3254902 , 0.34117647}; // Gray
    float c3[3] = {0.2745098 , 0.44705882, 0.52941176}; // dark blue
    float c4[3] = {0.48627451, 0.63137255, 0.67843137}; // llight blue
    float c5[3] = {0.81176471, 0.84705882, 0.86666667}; // kind white
    float c6[3] = {0.55686275, 0.45490196, 0.83137255}; // purple
    
    // Sliders/Parameters
    juce::Slider sliderL1;
    juce::Slider sliderL2;
    juce::Slider sliderL3;
    juce::Slider sliderL4;
    juce::Slider sliderMakeUp;
    
    juce::Label labelL1;
    juce::Label labelL2;
    juce::Label labelL3;
    juce::Label labelL4;
    juce::Label labelMakeUp;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderL1Attachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderL2Attachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderL3Attachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderL4Attachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderMakeUpAttachment;
    
    
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NeuralCabAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeuralCabAudioProcessorEditor)
};
