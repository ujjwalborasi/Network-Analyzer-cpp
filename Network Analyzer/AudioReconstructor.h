#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>
#include "RtpStreamCollector.h"

class AudioReconstructor {
public:
    static AudioReconstructor& GetInstance();

    // Reconstructs a WAV file for every SSRC stream in the given map.
    // outputDir: folder to write .wav files into (created if needed by caller)
    void ReconstructAll(const std::unordered_map<uint32_t, std::vector<RtpFrame>>& streams,
        const std::string& outputDir);

private:
    AudioReconstructor() {}
    AudioReconstructor(const AudioReconstructor&) = delete;
    AudioReconstructor& operator=(const AudioReconstructor&) = delete;

    void SortFramesBySequence(std::vector<RtpFrame>& frames);
    int16_t DecodeMuLawSample(uint8_t muLawByte);
    std::vector<int16_t> DecodePcmuFrames(const std::vector<RtpFrame>& frames);
    void WriteWavFile(const std::string& filename, const std::vector<int16_t>& pcmData, int sampleRate);
};