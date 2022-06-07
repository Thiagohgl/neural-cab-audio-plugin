/*
  ==============================================================================

    NeuralCabinet.cpp
    Created: 17 Dec 2020 6:31:40pm
    Author:  Thiago Lobato

  ==============================================================================
*/

#include "NeuralCabinet.h"


NeuralCab::NeuralCab()
{


};

void NeuralCab::setupNeuralCab()
{
    CrossFadeLengthInSamples  = fftSize;
    NewTransferFunctionReady = false;
   
   AmplitudeOfMainTransferFunctionInDB  = std::make_unique<float[]>(fftSize) ;
   TransferFunctionFromMicrophonePositionInDB = std::make_unique<float[]>(fftSize) ;
   
   MainTransferFunction.realloc (fftSize);
   MainTransferFunction.clear (fftSize);
   
   MainTransferFunctionOld.realloc (fftSize);
   MainTransferFunctionOld.clear (fftSize);
   
   timeDomainBufferCrossFade.realloc (fftSize);
   timeDomainBufferCrossFade.clear (fftSize);
   
   frequencyDomainBufferCrossFade.realloc (fftSize);
   frequencyDomainBufferCrossFade.clear (fftSize);
   

   FilterPhaseArray = std::make_unique< std::complex<float>[]>(fftSize) ;
   
   df    = SamplingRate/fftSize;
   float numtaps = 2;
   
   for(int idx=0;idx<fftSize;idx++)
   {
       float arg = -(numtaps - 1) / 2.  * PI * ((float)idx) / (fftSize*0.5-1) ;
       FilterPhaseArray[idx] = {  std::cos( arg ), std::sin(arg)   };
       
       AmplitudeOfMainTransferFunctionInDB[idx] = 0;
       MainTransferFunction[idx] = FilterPhaseArray[idx] ;
       MainTransferFunctionOld[idx]       = {1,0};
       frequencyDomainBufferCrossFade[idx] ={0,0};
       timeDomainBufferCrossFade[idx]={0,0};
       TransferFunctionFromMicrophonePositionInDB[idx] = 0;
       
   }
  
   UpdateCabinetTransferFunction();
   UpdateMicrophoneTransferFunction();
   MergeTransferFunctions();
   ResampleTransferFunction();
   NewTransferFunctionReady = true;
}

void NeuralCab::modification()
{
    
    fft->perform (timeDomainBuffer, frequencyDomainBuffer, false);
    
    
// ####### Filter the signal
    std::complex<float> FIRfilterElement = {1,0};
    if(NewTransferFunctionReady)
    {
        for(int idx = 1; idx < fftSize*0.5-1; idx++)
        {
 
            // Older filter for cross-fade
            FIRfilterElement= MainTransferFunctionOld[idx];
            frequencyDomainBufferCrossFade[idx]           = FIRfilterElement*frequencyDomainBuffer[idx];
            frequencyDomainBufferCrossFade[idx]           *= makeUp;
            frequencyDomainBufferCrossFade[fftSize - idx] = std::conj(frequencyDomainBufferCrossFade[idx]);;
            
            // Current filter
            FIRfilterElement= MainTransferFunction[idx];
            frequencyDomainBuffer[idx]                  = FIRfilterElement*frequencyDomainBuffer[idx];
            frequencyDomainBuffer[idx]                  *= makeUp;
            frequencyDomainBuffer[fftSize - idx]        = std::conj(frequencyDomainBuffer[idx]);
            

            MainTransferFunctionOld[idx] = FIRfilterElement;
            

        }
    }
    else
    {
        for(int idx = 1; idx < fftSize*0.5-1; idx++)
        {
            // Actual filter
            FIRfilterElement= MainTransferFunction[idx];
            frequencyDomainBuffer[idx]           = FIRfilterElement*frequencyDomainBuffer[idx];
            frequencyDomainBuffer[idx]           *= makeUp;
            frequencyDomainBuffer[fftSize - idx] =  std::conj(frequencyDomainBuffer[idx]);
            

            MainTransferFunctionOld[idx] = FIRfilterElement;

        }
    }
    
 // ########## Get back the time signal ################
    fft->perform (frequencyDomainBuffer, timeDomainBuffer, true);
   
    if(NewTransferFunctionReady)
    {
        CrossFadeOutput();
        NewTransferFunctionReady = false;

    }
}

void NeuralCab::CrossFadeOutput()
{
    fft->perform (frequencyDomainBufferCrossFade, timeDomainBufferCrossFade, true);
    
    for(int idx=0;idx<CrossFadeLengthInSamples;idx++)
    {
        float alpha = std::cos(2*PI*((float)idx)/((CrossFadeLengthInSamples-1)*4)    );//((float)idx)/(L-1);
        timeDomainBuffer[idx] = timeDomainBuffer[idx]*alpha+timeDomainBufferCrossFade[idx]*(1-alpha);
    }
}

void NeuralCab::UpdateParameters(float _latentVariables[4], float normMicDist,float _makeUpDB, float _SamplingRate=48000)
{

    makeUp             = std::pow(10,_makeUpDB/20.);
    SamplingRate       =  _SamplingRate;
    df    = SamplingRate/fftSize;
    NormalizedPosition = normMicDist;
    for(int idx=0;idx<SIZE_OF_LATENT_SPACE;idx++)
        latentVariables[idx]    = _latentVariables[idx];

    if (!NewTransferFunctionReady && UpdateIsReady)
    {
        UpdateIsReady = false;
        UpdateMicrophoneTransferFunction();
        UpdateCabinetTransferFunction();
        MergeTransferFunctions();
        ResampleTransferFunction();
        NewTransferFunctionReady = true;
        
        UpdateIsReady = true;
    }
}

void NeuralCab::MergeTransferFunctions()
{
    
    float spreadFactorOfMicrophoneAdjustment = 2; // The higher, the stronger the simulated mic influence
    float OffsetInDBForLatentSpaceTF = 0;
    for(int idx=0;idx<SIZE_OF_HALF_TRANSFER_FUNCTION;idx++)
    {
        AmplitudeOfMainTransferFunctionInDB[idx] = TransferFunctionFromLatentSpaceIndB[idx]+OffsetInDBForLatentSpaceTF;
        AmplitudeOfMainTransferFunctionInDB[idx] += TransferFunctionFromMicrophonePositionInDB[idx]*spreadFactorOfMicrophoneAdjustment;
    }
    
}

void NeuralCab::ResampleTransferFunction()
{

    float Tol = 0.1;
    bool SameDF = Tol>std::abs(df-DELTA_FREQ_ORIGINAL );
    float dfRatio = df/DELTA_FREQ_ORIGINAL;
    
    if (SameDF)
    {
        for(int idx = 1; idx < fftSize*0.5-1; idx++)
        {
            MainTransferFunction[idx] =  std::exp( (AmplitudeOfMainTransferFunctionInDB[idx])/20)*FilterPhaseArray[idx] ;
            MainTransferFunction[fftSize - idx] = std::conj(MainTransferFunction[idx]);
        }
    }
    else // Linear interpolation with the frequency bins
    {
        for(int idx = 1; idx < fftSize*0.5-1; idx++)
        {
            float EquivalentOriginalIdx = idx*dfRatio;
            
            if(EquivalentOriginalIdx>=SIZE_OF_HALF_TRANSFER_FUNCTION-1)
            {
                MainTransferFunction[idx] =  FilterPhaseArray[idx] ;
                MainTransferFunction[fftSize - idx] = std::conj(MainTransferFunction[idx]);
                continue;
            }
            
            int LowerIdx = std::floor(EquivalentOriginalIdx);
            int UpperIdx = std::ceil(EquivalentOriginalIdx);
            float RationBetweenCalculatedPoints = EquivalentOriginalIdx-LowerIdx;

            float TFtemp = (AmplitudeOfMainTransferFunctionInDB[UpperIdx]*RationBetweenCalculatedPoints+
                      AmplitudeOfMainTransferFunctionInDB[LowerIdx]*(1-RationBetweenCalculatedPoints));
        
           
            MainTransferFunction[idx] =  std::exp((TFtemp)/20)*FilterPhaseArray[idx] ;
            MainTransferFunction[fftSize - idx] = std::conj(MainTransferFunction[idx]);
            
        }
    }
}


void NeuralCab::UpdateMicrophoneTransferFunction()
{
    int TableLength            = 10;
    float NormalizedFloatIndex = NormalizedPosition*TableLength;
    float RatioBetweenIndexes  = NormalizedFloatIndex-(int)NormalizedFloatIndex;
    
    int idxLow = (int)NormalizedFloatIndex;
    int idxHigh = std::fmin(TableLength,(int)NormalizedFloatIndex+1);
    
    for(int idx = 1; idx < SIZE_OF_HALF_TRANSFER_FUNCTION; idx++)
    {
        TransferFunctionFromMicrophonePositionInDB[idx] = PreCalculatedMicTransferFunction[idxLow][idx]*(1-RatioBetweenIndexes)+
                                                          PreCalculatedMicTransferFunction[idxHigh][idx]*RatioBetweenIndexes;
        TransferFunctionFromMicrophonePositionInDB[idx] = 20*std::log10(TransferFunctionFromMicrophonePositionInDB[idx]);
    }
}

void NeuralCab::UpdateCabinetTransferFunction()
{
    // Create vector to be read by onnx
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    
    input_tensor_values = {latentVariables[0],latentVariables[1],latentVariables[2],latentVariables[3]};
    
    Ort::Value input_tensor_ = Ort::Value::CreateTensor<float>(memory_info, input_tensor_values.data(), input_tensor_values.size(), input_shape_.data(), input_shape_.size());
    Ort::Value output_tensor_ = Ort::Value::CreateTensor<float>(memory_info, AmplitudeTransferFunctionFromOnnxInDB.data(), AmplitudeTransferFunctionFromOnnxInDB.size(), output_shape_.data(), output_shape_.size());

    const char* input_names[] = {"input"};
    const char* output_names[] = {"output"};
    
    size_t input_count = 1;
    size_t output_count = 1;
    session.Run(Ort::RunOptions{nullptr}, input_names, &input_tensor_, input_count, output_names, &output_tensor_, output_count);
    
    for(int idx=0;idx<SIZE_OF_HALF_TRANSFER_FUNCTION;idx++)
    {
        TransferFunctionFromLatentSpaceIndB[idx] = AmplitudeTransferFunctionFromOnnxInDB[idx];
    }
}


