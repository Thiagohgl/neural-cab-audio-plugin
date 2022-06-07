/*
  ==============================================================================

    SpectrumComponent.h
    Created: 17 Dec 2020 6:32:55pm
    Author:  Thiago Lobato

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
//==============================================================================
/**
*/
class SpectrumComponent  : public juce::AudioProcessorEditor,
private juce::Timer
{
public:
    SpectrumComponent (NeuralCabAudioProcessor&);
    ~SpectrumComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;
    
    // #####################
    juce::Image SpectogramImageProcessor;
    void mouseMove (const juce::MouseEvent &event) override;
    void mouseDown(const juce::MouseEvent &event) override;
    void repaintCallback();
private:
    
    NeuralCabAudioProcessor& audioProcessor;
    int mouseXPosition;
    int mouseYPosition;

    float c1[3] = {0.23137255, 0.23137255, 0.24313725}; // Black
    float c2[3] = {0.32941176, 0.3254902 , 0.34117647}; // Gray
    float c3[3] = {0.2745098 , 0.44705882, 0.52941176}; // dark blue
    float c4[3] = {0.48627451, 0.63137255, 0.67843137}; // llight blue
    float c5[3] = {0.81176471, 0.84705882, 0.86666667}; // kind white
    float c6[3] = {0.55686275, 0.45490196, 0.83137255}; // purple

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumComponent)
};

