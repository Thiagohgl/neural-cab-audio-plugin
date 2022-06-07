/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



//==============================================================================
NeuralCabAudioProcessor::NeuralCabAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::mono(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::mono(), true)
                     #endif
                       ),apvts(*this, nullptr, "Parameters", createParameters())
#endif
{
    
    
    // Add listener
    apvts.addParameterListener("l1",this);
    apvts.addParameterListener("l2",this);
    apvts.addParameterListener("l3",this);
    apvts.addParameterListener("l4",this);
    apvts.addParameterListener("normMicDist",this);
    apvts.addParameterListener("makeup",this);
    
}

NeuralCabAudioProcessor::~NeuralCabAudioProcessor()
{
}

//==============================================================================
const juce::String NeuralCabAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NeuralCabAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NeuralCabAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NeuralCabAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NeuralCabAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NeuralCabAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NeuralCabAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NeuralCabAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NeuralCabAudioProcessor::getProgramName (int index)
{
    return {};
}

void NeuralCabAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NeuralCabAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Plot buffers
    bufferToPlot     = std::make_unique<float[]>(FFTSize/2);
    bufferToPlotTemp =std::make_unique<float[]>(FFTSize/2);
    gSamplesPerBlock =samplesPerBlock;
    Fs               = sampleRate;
    
    for(int idx=0;idx<FFTSize/2;idx++)
    {
        bufferToPlot[idx] = 1;
        bufferToPlotTemp[idx] = 1;
    }
        //======================================
    
    NeuralC.setupWOLA (getTotalNumInputChannels());
    
    
    NeuralC.updateParameters(FFTSize,NeuralCab::windowTypeIndex::windowTypeHann);
    NeuralC.setupNeuralCab();
    updateGUITransferFunction();
}

void NeuralCabAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NeuralCabAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() )
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NeuralCabAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();
    
    
    // clean channels
    for (int channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear (channel, 0, numSamples);
    //======================================
    NeuralC.processBlock (buffer);
    //======================================

    if(NeuralC.isTransferFunctionUpdateReady())
        updateGUITransferFunction();
        
    
}

void NeuralCabAudioProcessor::updateGUITransferFunction()
{
    
    for(int idx=0;idx<NeuralC.fftSize*0.5;idx++)
    {
        if (idx == FFTSize/2-1)
        {
            std::copy(&bufferToPlotTemp[0],&bufferToPlotTemp[FFTSize/2-1],&bufferToPlot[0]);
            std::fill (&bufferToPlotTemp[0],&bufferToPlotTemp[FFTSize/2-1], 0.0f);
        }

        bufferToPlotTemp[idx] = std::abs(NeuralC.MainTransferFunctionOld[idx]);
        
    }
    
}

//==============================================================================
bool NeuralCabAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NeuralCabAudioProcessor::createEditor()
{
    isUpdateFromPluginProcessor = true;
    return new NeuralCabAudioProcessorEditor (*this);
}

//==============================================================================
void NeuralCabAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
}

void NeuralCabAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{

    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
         
        if (xmlState.get() != nullptr)
            if (xmlState->hasTagName (apvts.state.getType()))
                apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
    
    parameterChanged("",NULL);
    isUpdateFromPluginProcessor = true;
    
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NeuralCabAudioProcessor();
}


void NeuralCabAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    // How to actually get the paramters from the parameter tree
    auto MakeUp = apvts.getRawParameterValue("makeup");

    auto l1               = apvts.getRawParameterValue("l1");
    auto l2               = apvts.getRawParameterValue("l2");
    auto l3               = apvts.getRawParameterValue("l3");
    auto l4               = apvts.getRawParameterValue("l4");
    auto normMicDist      = apvts.getRawParameterValue("normMicDist");
    //float _latentVariables[4] = {l1->load(),l2->load(),l3->load(),l4->load()};
    latentVariables[0] =l1->load();
    latentVariables[1] =l2->load();
    latentVariables[2] =l3->load();
    latentVariables[3] =l4->load();
    
    normMicDistance =normMicDist->load();
    
    NeuralC.UpdateParameters(latentVariables,normMicDistance ,MakeUp->load(),Fs);
    
    updateGUITransferFunction();
        
}

void NeuralCabAudioProcessor::UpdateLatentSpace(float LatentSpace[4])
{
    juce::Value temp1 = apvts.getParameterAsValue ("l1");
    temp1 = LatentSpace[0];
    juce::Value temp2 = apvts.getParameterAsValue ("l2");
    temp2 = LatentSpace[1];
    juce::Value temp3 = apvts.getParameterAsValue ("l3");
    temp3 = LatentSpace[2];
    juce::Value temp4 = apvts.getParameterAsValue ("l4");
    temp4 = LatentSpace[3];
    parameterChanged("",NULL);
}

void NeuralCabAudioProcessor::UpdateMicNormalizedPosition(float NormalizedPosition)
{
    juce::Value temp1 = apvts.getParameterAsValue ("normMicDist");
    temp1 = NormalizedPosition;
    parameterChanged("",NULL);
}

juce::AudioProcessorValueTreeState::ParameterLayout NeuralCabAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // Latent space
    float LatentMax = 2.0f;
    params.push_back(std::make_unique<juce::AudioParameterFloat>("l1", // parameterID
                                                                 "Latent1", // parameter name
                                                                 -LatentMax,   // minimum value
                                                                 LatentMax,   // maximum value
                                                                 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("l2", // parameterID
                                                                 "Latent2", // parameter name
                                                                 -LatentMax,   // minimum value
                                                                 LatentMax,   // maximum value
                                                                 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("l3", // parameterID
                                                                 "Latent3", // parameter name
                                                                 -LatentMax,   // minimum value
                                                                 LatentMax,   // maximum value
                                                                 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("l4", // parameterID
                                                                 "Latent4", // parameter name
                                                                 -LatentMax,   // minimum value
                                                                 LatentMax,   // maximum value
                                                                 0.0f));
  
    params.push_back(std::make_unique<juce::AudioParameterFloat>("normMicDist", // parameterID
                                                                 "Normalize Microphone Distance", // parameter name
                                                                 0.0f,   // minimum value
                                                                 1.0f,   // maximum value
                                                                 0.32f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>("makeup", //parameterID
                                                                "Make Up gain dB", // parameter name
                                                                -20.0f,   // minimum value
                                                                20.0f,   // maximum value
                                                                0.0f)); // default value
    return {params.begin(),params.end()};
}
