# Neural Cab
Neural Cab is a FIR guitar cabinet simulator which generates its transfer functions by means of a Variational Auto-Encoder (VAE) trained with an additional adversarial loss and a very simple Boundary Element Method (BEM) simulation to consider the microphone position. It is coded using JUCE and Onnx for the Machine Learning part. You can see a video with the usage of the plugin below: 



https://user-images.githubusercontent.com/23222254/172481889-20ae6fe3-b458-47e0-97df-bfee7e8fc56e.mov



## Installation 
After cloning the repository, you can just start the Projucer project and compile it to your platform. 
The current project is set for the MacOS arm architecture, but you can easily modify it to run in other platforms since the only OS-dependent parts are the Onnx dynamic library and the binaries output paths. Pre-compiled binaries for Onnx can be downloaded from here: https://github.com/microsoft/onnxruntime/releases, be sure to download the correct one for your system. 

## Motivation
Often when we want to simulate a guitar cabinet, we’re faced with a list full of entries and the only cues that we have to guide our choice is our previous experience, which limits the possibility of exploring new sounds or match the cabinet specifically for the current track. 
The idea of Neural Cab is to replace the hurdle of testing individual and independent cabinets in an endless list by a very simple click-and-drag approach that can generate a huge amount of transfer functions with context cohesion. Thus, if you want a cabinet that sounds only slightly sharper than the current one, you can get it very easily with this tool, while this would be a close to impossible task in the traditional list-selection approach. 

## Disclaimer 
This is an old project of mine that I did mostly for educational purposes. Thus be aware that the code may not be fully optimised for speed and may need some changes if you want to use it in a project with a very heavy DSP load. 
