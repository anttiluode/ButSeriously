#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

juce::AudioProcessorValueTreeState::ParameterLayout ButSeriouslyProcessor::createLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("beta", "Cooling (Beta)", 0.0f, 2.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("eml_drive", "EML Gate", 0.0f, 10.0f, 2.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("time", "Base Delay (ms)", 1.0f, 100.0f, 15.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("feedback", "Resonance", 0.0f, 0.98f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 0.5f));
    return layout;
}

ButSeriouslyProcessor::ButSeriouslyProcessor()
    : AudioProcessor(BusesProperties().withInput("In", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Out", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createLayout()) 
{
    // Listen for manual knob turns to disable MIDI override
    apvts.addParameterListener("time", this);
}

ButSeriouslyProcessor::~ButSeriouslyProcessor() {
    apvts.removeParameterListener("time", this);
}

juce::AudioProcessorEditor* ButSeriouslyProcessor::createEditor() {
    return new ButSeriouslyEditor(*this);
}

void ButSeriouslyProcessor::parameterChanged(const juce::String& parameterID, float) {
    // If the user manually turns the delay knob, disable MIDI tracking
    if (parameterID == "time") {
        isMidiDriven.store(false);
    }
}

void ButSeriouslyProcessor::prepareToPlay(double sampleRate, int) {
    for (int ch = 0; ch < 2; ++ch) {
        takensDelay[ch].prepare(sampleRate, 50.0);
        for (int k = 0; k < NUM_ARMS; ++k) {
            armDelays[ch][k].prepare(sampleRate, 1000.0);
        }
    }
}

void ButSeriouslyProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    
    // --- Parse incoming MIDI notes to tune the Resonator ---
    for (const auto metadata : midiMessages) {
        auto message = metadata.getMessage();
        if (message.isNoteOn()) {
            float hz = (float)juce::MidiMessage::getMidiNoteInHertz(message.getNoteNumber());
            // Formula: 1 cycle in Hz = 1000 / Hz in milliseconds
            midiDelayTarget.store(1000.0f / hz);
            isMidiDriven.store(true);
        }
    }

    float beta  = apvts.getRawParameterValue("beta")->load();
    float drive = apvts.getRawParameterValue("eml_drive")->load();
    float fb    = apvts.getRawParameterValue("feedback")->load();
    float mix   = apvts.getRawParameterValue("mix")->load();

    // Use either the MIDI note delay or the UI knob delay
    float baseTime = isMidiDriven.load() ? midiDelayTarget.load() : apvts.getRawParameterValue("time")->load();
    float lockPhase = juce::jlimit(0.0f, 1.0f, beta);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        float* channelData = buffer.getWritePointer(ch);

        for (int s = 0; s < buffer.getNumSamples(); ++s) {
            float x_t = channelData[s];
            
            float x_td = takensDelay[ch].read(15.0f);
            takensDelay[ch].write(x_t);

            if (ch == 0 && s % 8 == 0) {
                scopeX.store(x_t);
                scopeY.store(x_td);
            }

            float mag = std::sqrt(x_t * x_t + x_td * x_td) * 5.0f; 
            float phase = std::abs(std::atan2(x_td, x_t)) / juce::MathConstants<float>::pi;
            
            float frustration = std::abs(mag - phase);
            float gate = std::exp(-drive * frustration);
            float x_eml = std::tanh(x_t * gate * (1.0f + drive));

            float armSum = 0.0f;
            for (int k = 0; k < NUM_ARMS; ++k) {
                float targetLength = (primes[k] + lockPhase * (harmonics[k] - primes[k])) * baseTime;
                armSum += armDelays[ch][k].read(targetLength);
            }
            armSum /= (float)NUM_ARMS;

            for (int k = 0; k < NUM_ARMS; ++k) {
                armDelays[ch][k].write(x_eml + armSum * fb);
            }

            float wet = std::tanh(armSum);
            channelData[s] = x_t + mix * (wet - x_t);
        }
    }
}

void ButSeriouslyProcessor::getStateInformation(juce::MemoryBlock& dest) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void ButSeriouslyProcessor::setStateInformation(const void* data, int size) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ButSeriouslyProcessor(); }