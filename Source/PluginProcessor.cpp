/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicCompressorAudioProcessor::BasicCompressorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
waveViewer(1)
#endif
{
    ratioPtr = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("ratio"));
    attackPtr = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("attack"));
    releasePtr = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("release"));
    thresholdPtr = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("threshold"));
    bypassPtr = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter("bypass"));
    inputGainPtr = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("inputGain"));
    outputGainPtr = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("outputGain"));

    waveViewer.setRepaintRate(30);
    waveViewer.setBufferSize(256);
}

BasicCompressorAudioProcessor::~BasicCompressorAudioProcessor()
{
}

//==============================================================================
const juce::String BasicCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BasicCompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BasicCompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BasicCompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BasicCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicCompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BasicCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicCompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BasicCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;
    
    rmsInLevelL.reset(sampleRate, 0.75);
    rmsInLevelR.reset(sampleRate, 0.75);
    rmsOutLevelL.reset(sampleRate, 0.75);
    rmsOutLevelR.reset(sampleRate, 0.75);

    rmsInLevelL.setCurrentAndTargetValue(-1000.f);
    rmsInLevelR.setCurrentAndTargetValue(-1000.f);
    rmsOutLevelL.setCurrentAndTargetValue(-1000.f);
    rmsOutLevelR.setCurrentAndTargetValue(-1000.f);
    
    compressor.prepare(spec);
    inputGain.prepare(spec);
    outputGain.prepare(spec);
    
    inputGain.setRampDurationSeconds(0.05);
    outputGain.setRampDurationSeconds(0.05);
}

void BasicCompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void BasicCompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
//
//    // In case we have more outputs than inputs, this code clears any output
//    // channels that didn't contain input data, (because these aren't
//    // guaranteed to be empty - they may contain garbage).
//    // This is here to avoid people getting screaming feedback
//    // when they first compile a plugin, but obviously you don't need to keep
//    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    
    compressor.setRatio(ratioPtr->getCurrentChoiceName().getFloatValue());
    compressor.setAttack(attackPtr->get());
    compressor.setRelease(releasePtr->get());
    compressor.setThreshold(thresholdPtr->get());
    
    inputGain.setGainDecibels(inputGainPtr->get());
    outputGain.setGainDecibels(outputGainPtr->get());
    
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    
    if(bypassPtr->get() != true)
    {
        inputGain.process(context);
        storeRmsValue(rmsInLevelL, buffer, 0);
        storeRmsValue(rmsInLevelR, buffer, 1);
        
        compressor.process(context);
        
        waveViewer.pushBuffer(buffer);
        
        outputGain.process(context);
        storeRmsValue(rmsOutLevelL, buffer, 0);
        storeRmsValue(rmsOutLevelR, buffer, 1);
    }
    else
    {
        waveViewer.pushBuffer(buffer);
        
        rmsInLevelL.setCurrentAndTargetValue(-1000.f);
        rmsInLevelR.setCurrentAndTargetValue(-1000.f);
        rmsOutLevelL.setCurrentAndTargetValue(-1000.f);
        rmsOutLevelR.setCurrentAndTargetValue(-1000.f);

    }
}

//==============================================================================
bool BasicCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BasicCompressorAudioProcessor::createEditor()
{
    return new BasicCompressorAudioProcessorEditor (*this);
//    return new GenericAudioProcessorEditor(*this);
}

//==============================================================================
void BasicCompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BasicCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}


AudioProcessorValueTreeState::ParameterLayout BasicCompressorAudioProcessor::createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;
    
    auto ratioChoices = std::vector<double> {1, 1.5, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 25, 50, 100};
    StringArray ratioChoicesArray;
    for(auto choice : ratioChoices)
    {
        ratioChoicesArray.add(juce::String(choice, 1));
    }
    
    auto attackReleaseRange = NormalisableRange<float>(5.f, 500.f, 1.f, 1.f);
    auto gainRange = NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f);

        
    layout.add(std::make_unique<AudioParameterChoice>("ratio",
                                                      "ratio",
                                                      ratioChoicesArray,
                                                      0));
    layout.add(std::make_unique<AudioParameterFloat>("attack",
                                                     "attack",
                                                     attackReleaseRange,
                                                     50.f));
    layout.add(std::make_unique<AudioParameterFloat>("release",
                                                     "release",
                                                     attackReleaseRange,
                                                     250.f));
    layout.add(std::make_unique<AudioParameterFloat>("threshold",
                                                     "threshold",
                                                     NormalisableRange<float>(-60.f, 20.f, 1.f, 1.f),
                                                     0.f));
    layout.add(std::make_unique<AudioParameterBool>("bypass",
                                                    "bypass",
                                                    false));
    layout.add(std::make_unique<AudioParameterFloat>("inputGain",
                                                     "inputGain",
                                                     gainRange,
                                                     0));
    layout.add(std::make_unique<AudioParameterFloat>("outputGain",
                                                     "outputGain",
                                                     gainRange,
                                                     0));
    
    return layout;
    
}

float BasicCompressorAudioProcessor::getRmsLevel(bool inOut, const int channel)
{
    if(inOut)
    {
        if(channel == 0)
        {
            return rmsInLevelL.getCurrentValue();
        }
        if(channel == 1)
        {
            return rmsInLevelR.getCurrentValue();
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if(channel == 0)
        {
            return rmsOutLevelL.getCurrentValue();
        }
        if(channel == 1)
        {
            return rmsOutLevelR.getCurrentValue();
        }
        else
        {
            return 0;
        }
    }
}

void BasicCompressorAudioProcessor::storeRmsValue(LinearSmoothedValue<float>& rmsMember, juce::AudioBuffer<float>& buffer, const int channel)
{
    rmsMember.skip(buffer.getNumSamples());
    
    auto value = Decibels::gainToDecibels(buffer.getRMSLevel(channel, 0, buffer.getNumSamples()));
    if (value < rmsMember.getCurrentValue())
    {
        rmsMember.setTargetValue(value);
    }
    else
    {
        rmsMember.setCurrentAndTargetValue(value);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicCompressorAudioProcessor();
}
