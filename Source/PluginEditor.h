/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Meter.h"

//==============================================================================
/**
*/
class BasicCompressorAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    BasicCompressorAudioProcessorEditor (BasicCompressorAudioProcessor&);
    ~BasicCompressorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void paintOverChildren (Graphics&) override;
    void timerCallback() override;
    
    void prepTextButton(TextButton* button, String text);

    std::vector<juce::Slider*> getSliders();

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BasicCompressorAudioProcessor& audioProcessor;
    Slider waveZoom, attack, release, threshold, ratio, inputGain, outputGain;
    juce::AudioProcessorValueTreeState::SliderAttachment attackAttach, releaseAttach, threshAttach, ratioAttach, inputGainAttach, outputGainAttach;
    TextButton bypass;
    AudioProcessorValueTreeState::ButtonAttachment bypassAttach;
//    viator_gui::FilmStripKnob attack, release, threshold, ratio;
    
    
    Meter inputMeterL, inputMeterR, outputMeterL, outputMeterR;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicCompressorAudioProcessorEditor)
};
