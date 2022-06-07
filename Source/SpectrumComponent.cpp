/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "SpectrumComponent.h"

//==============================================================================
SpectrumComponent::SpectrumComponent (NeuralCabAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    startTimerHz (30);
    setSize (700, 400);
    mouseXPosition = -1;
    mouseYPosition = -1;



}

SpectrumComponent::~SpectrumComponent()
{
}

//==============================================================================
void SpectrumComponent::paint (juce::Graphics& g)
{
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colour::fromFloatRGBA (c2[0],c2[1],c2[2], 1.0f));


                
    
    int scopeSize = audioProcessor.NeuralC.fftSize/2;
    auto width    = getLocalBounds().getWidth();
    auto height   = getLocalBounds().getHeight();
    
    // Create the relative index of the plot for a logarithmic image
    float PotStart   = std::log(20);
    float PotEnd     = std::log(20000);
    float LogStep    = (PotEnd-PotStart)/width;
    float df         = audioProcessor.getSampleRate()/(audioProcessor.NeuralC.fftSize);
    
    // Generate the graph background
    // Frequency Labels
    float FrequencyPosLabels[9] = {50,100,200,500,1000,2000,5000,8000,15000};
    std::string FrequencyLabels[9] = {"50","100","200","500","1k","2k","5k","8k","15k"};

    std::string AmplitudeLabels[9] = {"40 dB","30 dB","20 dB","10 dB","0 dB","-10 dB","-20 dB","-30 dB","-40 dB"};
    
    g.setFont (12.0f);
    
    for (int idx=0;idx<9;idx++)
    {
        // Frequency
        float ExactXpos = (std::log(FrequencyPosLabels[idx])-PotStart)/(LogStep);
        int xPos =(int) ExactXpos;
        g.setColour (juce::Colour::fromFloatRGBA (c5[0],c5[1],c5[2], 1.0f));
        g.drawText(FrequencyLabels[idx], xPos+2, 0, 40, 20, juce::Justification::left);
        
        // Level
        int yPos = (int)((float)height/10.0*idx);
        g.drawText(AmplitudeLabels[idx], 10, yPos, 40, 20, juce::Justification::left);
        g.setColour (juce::Colour::fromFloatRGBA (c5[0],c5[1],c5[2], 1.0f));
        g.drawHorizontalLine(yPos, 0, width);
        
    }
    
    float FrequencyLines[27] = {20,30,40,50,60,80,90,100,200,300,400,500,600,700,800,900,1000,2000,3000,
                                4000,5000,6000,7000,8000,9000,10000,20000};
    g.setColour (juce::Colour::fromFloatRGBA (c5[0],c5[1],c5[2], 1.0f));
    for (int idx=0;idx<27;idx++)
    {
        float ExactXpos = (std::log(FrequencyLines[idx])-PotStart)/(LogStep);
        int xPos =(int) ExactXpos;
        
        g.drawVerticalLine(xPos, 0, height);                          //width      // height
        
    }

    // Start plotting actual spectrum
    float MinPlot = -50.0f;
    float MaxPlot = 40.0f;
    juce::Path path;
    float eta = 0.00000001f;
    path.startNewSubPath (juce::Point<float> ((float) juce::jmap (1, 0, scopeSize - 1, 0, width),
                          juce::jmap ( 20*std::log10(audioProcessor.bufferToPlot[1]+eta), MinPlot, MaxPlot, (float) height, 0.0f) ) );
    for (int i = 2; i < audioProcessor.NeuralC.fftSize/2; i++)
    {
        float factual          = i*df;
        // Freq to pixel
        float XPixelValue      =  std::fmin(width, (std::log(factual)-PotStart)/(LogStep) ) ;
        
        path.lineTo (juce::Point<float> (XPixelValue,
                     juce::jmap (20*std::log10(audioProcessor.bufferToPlot[i]+eta), MinPlot, MaxPlot, (float) height, 0.0f) ));
        
        if((width-XPixelValue) <0) // In case frequency above wished max frequency
            break;
    }
    // Close path completely (Path is always automatically closed, so add lower right and lower left points at the end so it can close properly)
    path.lineTo (juce::Point<float> ((float) juce::jmap (scopeSize+5, 0, scopeSize - 1, 0, width),
                 juce::jmap (MinPlot-20, MinPlot, MaxPlot ,(float) height, 0.0f) ));
    path.lineTo (juce::Point<float> ((float) juce::jmap (-5, 0, scopeSize - 1, 0, width),
                 juce::jmap (MinPlot-20, MinPlot, MaxPlot, (float) height, 0.0f) ));
    
    path.closeSubPath();
    
    float lineThickness = 4.0f;
    g.setColour (juce::Colour::fromFloatRGBA (c4[0],c4[1],c4[2], 1.0f));
    g.strokePath (path, juce::PathStrokeType(lineThickness));
    g.setColour (juce::Colour::fromFloatRGBA (c5[0],c5[1],c5[2], 0.7f));

    

}

void SpectrumComponent::mouseMove(const juce::MouseEvent &event)
{
    mouseXPosition = event.getPosition().getX();
    mouseYPosition =event.getPosition().getY();
}

void SpectrumComponent::mouseDown(const juce::MouseEvent &event)
{

}

void SpectrumComponent::resized()
{
   
}

void SpectrumComponent::timerCallback()
{
        repaint();
}

void SpectrumComponent::repaintCallback()
{
        repaint();
}
