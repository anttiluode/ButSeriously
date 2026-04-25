#pragma once
#include <JuceHeader.h>
#include <vector>
#include <array>
#include <atomic>

// ─────────────────────────────────────────────────────────────────────
// Simple Interpolating Delay Line
// ─────────────────────────────────────────────────────────────────────
class SimpleDelay {
public:
    void prepare(double sampleRate, double maxDelayMs) {
        fs = sampleRate;
        buffer.assign((size_t)(sampleRate * maxDelayMs / 1000.0) + 10, 0.0f);
        writePtr = 0;
    }
    void write(float sample) {
        buffer[writePtr] = sample;
        writePtr = (writePtr + 1) % buffer.size();
    }
    float read(float delayMs) {
        float delaySamples = delayMs * (float)fs / 1000.0f;
        float readPos = (float)writePtr - delaySamples;
        while (readPos < 0.0f) readPos += (float)buffer.size();
        
        int idx1 = (int)readPos;
        int idx2 = (idx1 + 1) % buffer.size();
        float frac = readPos - (float)idx1;
        return buffer[idx1] * (1.0f - frac) + buffer[idx2] * frac;
    }
private:
    std::vector<float> buffer;
    int writePtr = 0;
    double fs = 44100.0;
};

// ─────────────────────────────────────────────────────────────────────
// "But Seriously" Processor
// ─────────────────────────────────────────────────────────────────────
class ButSeriouslyProcessor : public juce::AudioProcessor, 
                              public juce::AudioProcessorValueTreeState::Listener {
public:
    ButSeriouslyProcessor();
    ~ButSeriouslyProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "But Seriously"; }
    
    // IMPORTANT: Tells the DAW we want to consume MIDI!
    bool acceptsMidi() const override { return true; } 
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock& dest) override;
    void setStateInformation(const void* data, int size) override;

    void parameterChanged(const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float> scopeX {0.0f};
    std::atomic<float> scopeY {0.0f};

    // MIDI Tracking
    std::atomic<float> midiDelayTarget { 15.0f };
    std::atomic<bool>  isMidiDriven { false };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    static constexpr int NUM_ARMS = 6;
    std::array<float, NUM_ARMS> primes    = {2.0f, 3.0f, 5.0f, 7.0f, 11.0f, 13.0f};
    std::array<float, NUM_ARMS> harmonics = {2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f};

    SimpleDelay takensDelay[2];
    SimpleDelay armDelays[2][NUM_ARMS];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ButSeriouslyProcessor)
};