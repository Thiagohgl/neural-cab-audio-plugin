/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SpectrumComponent.h"
//==============================================================================
NeuralCabAudioProcessorEditor::NeuralCabAudioProcessorEditor (NeuralCabAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),Spec(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (720*2, 400*2);
    startTimerHz (30);
    setOpaque(true);
    // Variable initialization
    auto width = getWidth();
    auto heigh = getHeight();
    
    int LatentStart[2] = {(int)(width*0.2)+pad,(int)(heigh*0.6)+pad};
    int LatentLength[2] = {(int)(width*0.8)-pad*2, (int)(heigh*0.4)-pad*2};
    Latent12[0] = LatentStart[0]+LatentLength[0]/2;
    Latent34[0] = LatentStart[0]+LatentLength[0]/4;

    Latent12[1] = LatentStart[1]+LatentLength[1]/2;
    Latent34[1] = LatentStart[1]+LatentLength[1]/4;
    
    Latentxi =LatentStart[0]; Latentxe = LatentStart[0]+LatentLength[0];
    Latentyi =LatentStart[1]; Latentye = LatentStart[1]+LatentLength[1];
    
    
    int LoudspeakerStart[2]  = {pad,pad};
    int LoudspeakerLength[2] = {(int)(width*0.2)-pad*2, (int)(heigh*0.6)-pad*2};
    PointMicPosition[0]      = LoudspeakerStart[0]+LoudspeakerLength[0]/2;

    PointMicPosition[1]      = LoudspeakerStart[1]+LoudspeakerLength[1]/2;
    Micxi =LoudspeakerStart[0]; Micxe = LoudspeakerStart[0]+LoudspeakerLength[0];
    Micyi =LoudspeakerStart[1]; Micye = LoudspeakerStart[1]+LoudspeakerLength[1];
    
    
    // add objects
    
    sliderL1.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    sliderL1.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    labelL1.setText("L1", juce::NotificationType::dontSendNotification);
    labelL1.setJustificationType(juce::Justification::centredTop);
       
    sliderL2.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    sliderL2.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    labelL2.setText("L2", juce::NotificationType::dontSendNotification);
    labelL2.setJustificationType(juce::Justification::centredTop);
    
    sliderL3.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    sliderL3.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    labelL3.setText("L3", juce::NotificationType::dontSendNotification);
    labelL3.setJustificationType(juce::Justification::centredTop);
    
    sliderL4.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    sliderL4.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    labelL4.setText("L4", juce::NotificationType::dontSendNotification);
    labelL4.setJustificationType(juce::Justification::centredTop);
        
        
    sliderMakeUp.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    sliderMakeUp.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    labelMakeUp.setText("MakeUp", juce::NotificationType::dontSendNotification);
    labelMakeUp.setJustificationType(juce::Justification::centredTop);
    
    // Make it visible
    addAndMakeVisible(Spec);
    /*
    addAndMakeVisible(sliderL1); addAndMakeVisible(labelL1);
    addAndMakeVisible(sliderL2); addAndMakeVisible(labelL2);
    addAndMakeVisible(sliderL3); addAndMakeVisible(labelL3);
    addAndMakeVisible(sliderL4); addAndMakeVisible(labelL4);*/
    addAndMakeVisible(sliderMakeUp); addAndMakeVisible(labelMakeUp);
    
    // Add atachments
    sliderL1Attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"l1",sliderL1);
    sliderL2Attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"l2",sliderL2);
    sliderL3Attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"l3",sliderL3);
    sliderL4Attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"l4",sliderL4);
    sliderMakeUpAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"makeup",sliderMakeUp);
    
}

NeuralCabAudioProcessorEditor::~NeuralCabAudioProcessorEditor()
{
}

void NeuralCabAudioProcessorEditor::mouseDrag(const juce::MouseEvent &event)
{
    int Xpos = event.getPosition().getX();
    int Ypos = event.getPosition().getY();
    
    
    switch(ActualSelected)
    {
        case(ActualMouseSelection::None):{
            
            break;
        }
        case(ActualMouseSelection::FirstLatent):{
            if(Xpos>Latentxi  && Xpos<Latentxe)
                Latent12[0] = Xpos;
            if(Ypos>Latentyi  && Ypos<Latentye)
                Latent12[1] = Ypos;
            
            
            updateModelLatentSpace();
            break;
        }
        case(ActualMouseSelection::SecondLatent):{
            if(Xpos>Latentxi  && Xpos<Latentxe)
                Latent34[0] = Xpos;
            if(Ypos>Latentyi  && Ypos<Latentye)
                Latent34[1] = Ypos;
            
            
            updateModelLatentSpace();
            break;
        }
        case (ActualMouseSelection::Microphone):{
            
            if(Xpos>Micxi  && Xpos<Micxe)
                PointMicPosition[0] = Xpos;
            if(Ypos>Micyi  && Ypos<Micye)
                PointMicPosition[1] = Ypos;
            
            //float radiusFromCenter = std::sqrt( std::pow( (PointMicPosition[0]-Micxi)-((Micxe-Micxi)*0.5+Micxi  ),2) +
            //                           std::pow( (PointMicPosition[1]-Micyi)-((Micye-Micyi)*0.5+Micyi  ),2) );
            float radiusFromCenter = std::sqrt( std::pow( (PointMicPosition[0])-((Micxe-Micxi)*0.5+Micxi  ),2) +
                                       std::pow( (PointMicPosition[1])-((Micye-Micyi)*0.5+Micyi  ),2) );
            
            float NormalizedPosition = radiusFromCenter/((Micye-Micyi)*0.5);
            NormalizedPosition       = 1.0f-std::fmax(0,std::fmin(1.0f,NormalizedPosition));
            audioProcessor.UpdateMicNormalizedPosition(NormalizedPosition);
            
            
            break;
        }
    }
}

void NeuralCabAudioProcessorEditor::updateModelLatentSpace()
{
    int width = getWidth();
    int heigh = getHeight();
    int LatentStart[2] = {(int)(width*0.2)+pad,(int)(heigh*0.6)+pad};
    int LatentLength[2] = {(int)(width*0.8)-pad*2, (int)(heigh*0.4)-pad*2};
    
    
    float LatentSpace[4] = {(2*(float)(Latent12[0]-LatentStart[0])-LatentLength[0])/LatentLength[0]*ScalingLatentSpace,
                            (2*(float)(Latent12[1]-LatentStart[1])-LatentLength[1])/LatentLength[1]*ScalingLatentSpace,
                            (2*(float)(Latent34[0]-LatentStart[0])-LatentLength[0])/LatentLength[0]*ScalingLatentSpace,
                            (2*(float)(Latent34[1]-LatentStart[1])-LatentLength[1])/LatentLength[1]*ScalingLatentSpace};
    audioProcessor.UpdateLatentSpace(LatentSpace);
}


void NeuralCabAudioProcessorEditor::mouseDown(const juce::MouseEvent &event)
{
    int Xpos = event.getPosition().getX();
    int Ypos = event.getPosition().getY();
    
    ActualSelected = ActualMouseSelection::None;
    int radius = 30;
    if( std::sqrt( std::pow(Xpos-Latent12[0],2)+std::pow(Ypos-Latent12[1],2)    )<=radius  )
        ActualSelected = ActualMouseSelection::FirstLatent;
    if( std::sqrt( std::pow(Xpos-Latent34[0],2)+std::pow(Ypos-Latent34[1],2)    )<=radius  )
        ActualSelected = ActualMouseSelection::SecondLatent;
    if( std::sqrt( std::pow(Xpos-PointMicPosition[0],2)+std::pow(Ypos-PointMicPosition[1],2)    )<=radius  )
        ActualSelected = ActualMouseSelection::Microphone;
}

void NeuralCabAudioProcessorEditor::mouseUp(const juce::MouseEvent &event)
{
    
    ActualSelected = ActualMouseSelection::None;
    
}

void NeuralCabAudioProcessorEditor::updatePlotFromGivenLatentSpace(float latentSpace[4])
{
    int width = getWidth();
    int heigh = getHeight();
    int LatentStart[2] = {(int)(width*0.2)+pad,(int)(heigh*0.6)+pad};
    int LatentLength[2] = {(int)(width*0.8)-pad*2, (int)(heigh*0.4)-pad*2};
    
    Latent12[0] = LatentStart[0] + 0.5*( LatentLength[0]*latentSpace[0]/ScalingLatentSpace + LatentLength[0] );
    Latent12[1] = LatentStart[1] + 0.5*( LatentLength[1]*latentSpace[1]/ScalingLatentSpace + LatentLength[1] );
    Latent34[0] = LatentStart[0] + 0.5*( LatentLength[0]*latentSpace[2]/ScalingLatentSpace + LatentLength[0] );
    Latent34[1] = LatentStart[1] + 0.5*( LatentLength[1]*latentSpace[3]/ScalingLatentSpace + LatentLength[1] );
}

void NeuralCabAudioProcessorEditor::updatePlotFromGivenRadius(float normMicDistance)
{
    // Since radial symmetry, place in a position with equivalent radius
    normMicDistance = 1-normMicDistance;
    float radiusFromCenter = normMicDistance*((Micye-Micyi)*0.5);
    PointMicPosition[0] = ((Micxe-Micxi)*0.5+Micxi  );
    PointMicPosition[1] = radiusFromCenter+Micyi+(Micye-Micyi)*0.5;
    
    
}

//==============================================================================
void NeuralCabAudioProcessorEditor::paint (juce::Graphics& g)
{
    
    if (audioProcessor.isUpdateFromPluginProcessor)
    {
        updatePlotFromGivenLatentSpace(audioProcessor.latentVariables);
        updatePlotFromGivenRadius(audioProcessor.normMicDistance);
        audioProcessor.isUpdateFromPluginProcessor = false;
    }
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colour::fromFloatRGBA (c1[0],c1[1],c1[2], 1.0f));

    g.setColour (juce::Colour::fromFloatRGBA (c5[0],c5[1],c5[2], 1.0f));
    auto width = getWidth();
    auto heigh = getHeight();
    
    // ###################### Latent Space region ###################    
    int LatentStart[2] = {(int)(width*0.2)+pad,(int)(heigh*0.6)+pad};
    int LatentLength[2] = {(int)(width*0.8)-pad*2, (int)(heigh*0.4)-pad*2};
    g.fillRect(LatentStart[0],LatentStart[1] , LatentLength[0],LatentLength[1]);
    
    g.setColour (juce::Colour::fromFloatRGBA (c2[0],c2[1],c2[2], 1.0f));
    g.drawRect((int)(width*0.2)+pad-padBoarder, (int)(heigh*0.6)+pad-padBoarder, (int)(width*0.8)-pad*2+2*padBoarder, (int)(heigh*0.4)-pad*2+2*padBoarder);
    
    // Circles to select latent space
    g.setColour (juce::Colour::fromFloatRGBA (c4[0],c4[1],c4[2], 1.0f));
    int Lx[2] = {Latent12[0],Latent34[0]};
    int Ly[2] = {Latent12[1],Latent34[1]};
    

    g.fillEllipse( Lx[0]-EllipseSize/2, Ly[0]-EllipseSize/2, EllipseSize, EllipseSize);
    g.setColour (juce::Colour::fromFloatRGBA (c3[0],c3[1],c3[2], 1.0f));
    g.fillEllipse( Lx[1]-EllipseSize/2, Ly[1]-EllipseSize/2, EllipseSize, EllipseSize);
    
    
    // ############# Loudspeaker region ###############
    g.setColour (juce::Colour::fromFloatRGBA (c2[0],c2[1],c2[2], 1.0f));
    int LoudWidth = (int)(width*0.2)-pad*2;
    int LoudHeight =(int)(heigh*0.6)-pad*2;
    g.fillRect(pad, pad, LoudWidth, LoudHeight);
    
    // Loudspeaker coils
    g.setColour (juce::Colour::fromFloatRGBA (c4[0],c4[1],c4[2], 1.0f));
    int Diameter1 = (int)(LoudWidth*0.8);
    g.fillEllipse( LoudWidth/2+pad-Diameter1/2, (int)(LoudHeight*0.5)-Diameter1/2, Diameter1, Diameter1);
    
    g.setColour (juce::Colour::fromFloatRGBA (c5[0],c5[1],c5[2], 1.0f));
    int Diameter2 = (int)(LoudWidth*0.5);
    g.fillEllipse( LoudWidth/2+pad-Diameter2/2, (int)(LoudHeight*0.5)-Diameter2/2, Diameter2, Diameter2);
    
    // Microphone
    g.setColour (juce::Colour::fromFloatRGBA (c6[0],c6[1],c6[2], 1.0f));
    int MicX =PointMicPosition[0]; int MicY = PointMicPosition[1];
    g.fillEllipse( MicX-EllipseSize/2, MicY-EllipseSize/2, EllipseSize, EllipseSize);
    
    
    // #### Boarder region #############
    // Loudspeaker
    g.setColour (juce::Colour::fromFloatRGBA (c2[0],c2[1],c2[2], 1.0f));
    g.drawRect(pad-padBoarder, pad-padBoarder, (int)(width*0.2)-pad*2+2*padBoarder, (int)(heigh*0.6)-pad*2+2*padBoarder);
    // TF
    g.drawRect((int)(width*0.2)+pad-padBoarder,  pad-padBoarder, (int)(width*0.8)-pad*2+2*padBoarder, (int)(heigh*0.6)-pad*2+2*padBoarder);
    // Sliders
    g.drawRect(pad-padBoarder,  (int)(heigh*0.6)+pad-padBoarder, (int)(width*0.2)-pad*2+2*padBoarder, LatentLength[1]);
    
    
}

void NeuralCabAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto SliderSections = getLocalBounds();
    auto width = getWidth();
    auto heigh = getHeight();
    int sideItemMargin = 20;
    float plotWidthProportion = 0.8;
    float plotHeightProportion = 0.6;
    // Spectrum sub-component
    auto PlotRegion = SliderSections.removeFromRight( (int)(width*plotWidthProportion));
    Spec.setBounds(PlotRegion.removeFromTop((int)(heigh*plotHeightProportion)).reduced (sideItemMargin));
    
    // Slides
    SliderSections = SliderSections.removeFromBottom((int)(heigh*0.35));
    
    int leftBlock = width*(1-plotWidthProportion)*0.5;
    auto temp = SliderSections.removeFromLeft((int)(leftBlock));
    sliderMakeUp.setBounds(temp.removeFromTop((int)(heigh*0.1)));
    labelMakeUp.setBounds(temp.removeFromTop((int)(heigh*0.05))) ;
    /* 
    sliderL1.setBounds(temp.removeFromTop((int)(heigh*0.1)));
    labelL1.setBounds(temp.removeFromTop((int)(heigh*0.05))) ;
    
    sliderL3.setBounds(temp.removeFromTop((int)(heigh*0.1)));
    labelL3.setBounds(temp.removeFromTop((int)(heigh*0.05))) ;
    
    sliderL2.setBounds(SliderSections.removeFromTop((int)(heigh*0.1)));
    labelL2.setBounds(SliderSections.removeFromTop((int)(heigh*0.05))) ;
    
    sliderL4.setBounds(SliderSections.removeFromTop((int)(heigh*0.1)));
    labelL4.setBounds(SliderSections.removeFromTop((int)(heigh*0.05))) ;*/
    
   
}

void NeuralCabAudioProcessorEditor::timerCallback()
{
        repaint();
}
