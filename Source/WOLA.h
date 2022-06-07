/*
  ==============================================================================

    OLA.h
    Created: 22 Nov 2020 4:41:10pm
    Author:  Thiago Lobato

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
class WOLA
{
public:
    
    enum windowTypeIndex {
        windowTypeHann=0,
        windowTypeHannRoot=1,
    };

    //======================================

    WOLA() : numChannels (1)
    {
    }

    virtual ~WOLA()
    {
    }

    //======================================

    void setupWOLA (const int numInputChannels)
    {
        numChannels = (numInputChannels > 0) ? numInputChannels : 1;
    }

    void updateParameters (const int newFftSize, const int newWindowType)
    {
        updateFftSize (newFftSize);
        updateWindow (newWindowType);
    }

    //======================================

    void processBlock (juce::AudioSampleBuffer& block)
    {
        numSamples = block.getNumSamples();

        for (int channel = 0; channel < numChannels; ++channel) {
            float* channelData = block.getWritePointer (channel);

            currentInputBufferWritePosition = inputBufferWritePosition;
            currentOutputBufferWritePosition = outputBufferWritePosition;
            currentOutputBufferReadPosition = outputBufferReadPosition;
            currentSamplesSinceLastFFT = samplesSinceLastFFT;

            for (int sample = 0; sample < numSamples; ++sample) {
                const float inputSample = channelData[sample];
                inputBuffer.setSample (channel, currentInputBufferWritePosition, inputSample);
                if (++currentInputBufferWritePosition >= inputBufferLength)
                    currentInputBufferWritePosition = 0;

                channelData[sample] = outputBuffer.getSample (channel, currentOutputBufferReadPosition);
                outputBuffer.setSample (channel, currentOutputBufferReadPosition, 0.0f);
                if (++currentOutputBufferReadPosition >= outputBufferLength)
                    currentOutputBufferReadPosition = 0;

                if (++currentSamplesSinceLastFFT >= hopSize) {
                    currentSamplesSinceLastFFT = 0;

                    analysis (channel);
                    modification();
                    synthesis (channel);
                }
            }
        }

        inputBufferWritePosition = currentInputBufferWritePosition;
        outputBufferWritePosition = currentOutputBufferWritePosition;
        outputBufferReadPosition = currentOutputBufferReadPosition;
        samplesSinceLastFFT = currentSamplesSinceLastFFT;
    }

private:
    //======================================

    void updateFftSize (const int newFftSize)
    {
        fftSize = newFftSize;
        fft = std::make_unique<juce::dsp::FFT>(log2 (fftSize));

        inputBufferLength = fftSize;
        inputBuffer.clear();
        inputBuffer.setSize (numChannels, inputBufferLength);

        outputBufferLength = fftSize;
        outputBuffer.clear();
        outputBuffer.setSize (numChannels, outputBufferLength);

        fftWindow.realloc (fftSize);
        fftWindow.clear (fftSize);

        timeDomainBuffer.realloc (fftSize);
        timeDomainBuffer.clear (fftSize);

        frequencyDomainBuffer.realloc (fftSize);
        frequencyDomainBuffer.clear (fftSize);

        inputBufferWritePosition = 0;
        outputBufferWritePosition = 0;
        outputBufferReadPosition = 0;
        samplesSinceLastFFT = 0;
    }

    void updateHopSize (const int newOverlap)
    {
        overlap = newOverlap;
        if (overlap != 0) {
            hopSize = fftSize / overlap;
            outputBufferWritePosition = hopSize % outputBufferLength;
        }
    }

    void updateWindow (const int newWindowType)
    {
        int NormalizationPotency = 1;
        
        // Parameters were choosen to fulfill COLA constrain
        switch (newWindowType) {
            case windowTypeHann: {
                for (int sample = 0; sample < fftSize; ++sample)
                    fftWindow[sample] = 0.5f - 0.5f * cosf (2.0f * M_PI * (float)sample / (float)(fftSize - 1));
                windowScaleFactor = 1.0f /1.5f;
                updateHopSize(4);
                break;
            }
            case windowTypeHannRoot:
            {
                NormalizationPotency=2;
                for (int sample = 0; sample < fftSize; ++sample)
                    fftWindow[sample] = std::sqrt( 0.5f - 0.5f * cosf (2.0f * M_PI * (float)sample / (float)(fftSize - 1)) );
                windowScaleFactor = 1.0f;
                updateHopSize(2);
                break;
            }
        }

    }

    //======================================

    void analysis (const int channel)
    {
        int inputBufferIndex = currentInputBufferWritePosition;
        for (int index = 0; index < fftSize; ++index) {
            timeDomainBuffer[index].real (fftWindow[index] * inputBuffer.getSample (channel, inputBufferIndex));
            timeDomainBuffer[index].imag (0.0f);

            if (++inputBufferIndex >= inputBufferLength)
                inputBufferIndex = 0;
        }
    }

    virtual void modification() // Only a template
    {
        fft->perform (timeDomainBuffer, frequencyDomainBuffer, false);

        for (int index = 0; index < fftSize / 2 + 1; ++index) {
            float magnitude = abs (frequencyDomainBuffer[index]);
            float phase = arg (frequencyDomainBuffer[index]);

            frequencyDomainBuffer[index].real (magnitude * cosf (phase));
            frequencyDomainBuffer[index].imag (magnitude * sinf (phase));
            if (index > 0 && index < fftSize / 2) {
                frequencyDomainBuffer[fftSize - index].real (magnitude * cosf (phase));
                frequencyDomainBuffer[fftSize - index].imag (magnitude * sinf (-phase));
            }
        }

        fft->perform (frequencyDomainBuffer, timeDomainBuffer, true);
    }

    void synthesis (const int channel)
    {
        int outputBufferIndex = currentOutputBufferWritePosition;
        for (int index = 0; index < fftSize; ++index) {
            float outputSample = outputBuffer.getSample (channel, outputBufferIndex);
            outputSample += fftWindow[index] * timeDomainBuffer[index].real() * windowScaleFactor;
            outputBuffer.setSample (channel, outputBufferIndex, outputSample);

            if (++outputBufferIndex >= outputBufferLength)
                outputBufferIndex = 0;
        }

        currentOutputBufferWritePosition += hopSize;
        if (currentOutputBufferWritePosition >= outputBufferLength)
            currentOutputBufferWritePosition = 0;
    }
public:
    int fftSize;
    juce::HeapBlock<float> fftWindow;
protected:
    //======================================
    int numChannels;
    int numSamples;

    
    std::unique_ptr<juce::dsp::FFT> fft;

    int inputBufferLength;
    juce::AudioSampleBuffer inputBuffer;

    int outputBufferLength;
    juce::AudioSampleBuffer outputBuffer;

    
    juce::HeapBlock<juce::dsp::Complex<float>> timeDomainBuffer;
    juce::HeapBlock<juce::dsp::Complex<float>> frequencyDomainBuffer;

    int overlap;
    int hopSize;
    float windowScaleFactor;

    int inputBufferWritePosition;
    int outputBufferWritePosition;
    int outputBufferReadPosition;
    int samplesSinceLastFFT;

    int currentInputBufferWritePosition;
    int currentOutputBufferWritePosition;
    int currentOutputBufferReadPosition;
    int currentSamplesSinceLastFFT;
};
