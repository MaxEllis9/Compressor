/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicCompressorAudioProcessorEditor::BasicCompressorAudioProcessorEditor (BasicCompressorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
//inputMeterL([&](){}),
//    inputMeterR([&](){}),
//    outputMeterL([&](){}),
//    outputMeterR([&](){})

attack(1, "ms", 5, 500, false),
release(1, "ms", 5, 500, false),
threshold(1, "dB", -60, 20, false),
ratio(1, "", 0, 10, true)

//attackAttach(audioProcessor.apvts, "attack", attack),
//releaseAttach(audioProcessor.apvts, "release", release),
//threshAttach(audioProcessor.apvts, "threshold", threshold),
//ratioAttach(audioProcessor.apvts, "ratio", ratio)

{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    for(auto* slider : getSliders()){
//        slider->setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
//        slider->setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 20);
//        slider->
        addAndMakeVisible(slider);
    }
    
    addAndMakeVisible(audioProcessor.waveViewer);
    audioProcessor.waveViewer.setColours(Colours::darkgrey, Colours::black);
    
    addAndMakeVisible(waveZoom);
    waveZoom.setSliderStyle(Slider::SliderStyle::LinearVertical);
    waveZoom.setRange(128, 1024);
    waveZoom.onValueChange = [this]()
    {
        audioProcessor.waveViewer.setBufferSize(waveZoom.getValue());
    };
    setSize (600, 500);

}

BasicCompressorAudioProcessorEditor::~BasicCompressorAudioProcessorEditor()
{
}

//==============================================================================
void BasicCompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::black);
    auto bounds = getLocalBounds();
    auto topArea = bounds.removeFromTop(bounds.getHeight() * 0.7).reduced(5.f);
    auto dials = bounds.reduced(5.f);
    
    g.setColour(Colours::skyblue);
    g.fillRect(topArea.toFloat());
    g.fillRect(dials.toFloat());
    
}

void BasicCompressorAudioProcessorEditor::paintOverChildren(Graphics& g)
{
    auto bounds = getLocalBounds();
    auto topArea = bounds.removeFromTop(bounds.getHeight() * 0.7).reduced(5.f);
    auto graphArea = topArea.removeFromLeft(topArea.getWidth() * 0.8).reduced(5.f);
    
    auto left = graphArea.getX();
    auto right = graphArea.getRight();
    auto top = graphArea.getY();
    auto midline = graphArea.getHeight()/2;
    auto bottom = graphArea.getBottom();
    
//    g.drawHorizontalLine(, left, right);
    
    auto yTop = jmap(float(threshold.getValue()), -60.f, 20.f, float(midline), float(top));
    auto yBottom = jmap(float(threshold.getValue()), -60.f, 20.f, float(midline), float(bottom));
    
    auto topHorizontalLine = Rectangle<float>(left, yTop, right-left, 3.f);
    auto bottomHorizontalLine = Rectangle<float>(left, yBottom, right-left, 3.f);
    
    g.setColour(Colours::lightgrey.withAlpha(0.25f));
    g.fillRect(Rectangle<float>(topHorizontalLine.getBottomLeft().getX(), yTop, right-left, bottomHorizontalLine.getY()-topHorizontalLine.getY()));
    
    g.setColour(Colours::black);
    g.fillRect(topHorizontalLine);
    g.fillRect(bottomHorizontalLine);
}

void BasicCompressorAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    auto topArea = bounds.removeFromTop(bounds.getHeight() * 0.7).reduced(5.f);
    audioProcessor.waveViewer.setBounds(topArea.removeFromLeft(topArea.getWidth() * 0.8).reduced(5.f));
    waveZoom.setBounds(topArea);
    bounds.reduced(10.f);
    attack.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.25).reduced(10.f));
    release.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.33).reduced(10.f));
    threshold.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.5).reduced(10.f));
    ratio.setBounds(bounds.reduced(10.f));
}

std::vector<juce::Slider*> BasicCompressorAudioProcessorEditor::getSliders()
{
    return
    {
        &attack,
        &release,
        &threshold,
        &ratio
    };
}
