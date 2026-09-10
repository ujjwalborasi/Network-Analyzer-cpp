#include "AudioReconstructor.h"
#include "Logger.h"
#include <algorithm>
#include <fstream>

AudioReconstructor& AudioReconstructor::GetInstance() {
    static AudioReconstructor instance;
    return instance;
}

// Sorts by RTP sequence number, correctly handling 16-bit wraparound (65535 -> 0)
void AudioReconstructor::SortFramesBySequence(std::vector<RtpFrame>& frames) {
    std::sort(frames.begin(), frames.end(), [](const RtpFrame& a, const RtpFrame& b) {
        uint16_t diff = static_cast<uint16_t>(a.seqNum - b.seqNum);
        return diff > 32768; // true if 'a' logically comes before 'b'
        });
}

// Standard ITU-T G.711 mu-law -> linear PCM16 decode
int16_t AudioReconstructor::DecodeMuLawSample(uint8_t muLawByte) {
    muLawByte = ~muLawByte;
    int sign = (muLawByte & 0x80) ? -1 : 1;
    int exponent = (muLawByte >> 4) & 0x07;
    int mantissa = muLawByte & 0x0F;
    int sample = ((mantissa << 3) + 0x84) << exponent;
    sample -= 0x84;
    return static_cast<int16_t>(sign * sample);
}

std::vector<int16_t> AudioReconstructor::DecodePcmuFrames(const std::vector<RtpFrame>& frames) {
    std::vector<int16_t> pcm;
    for (const auto& frame : frames) {
        // PT 0 = PCMU. Skip anything else (e.g. PT 101 = telephone-event/DTMF,
        // which is not audio and would decode to garbage noise if treated as PCMU).
        if (frame.payloadType != 0) {
            Logger::GetInstance().Log(LogLevel::DEBUG,
                "    Skipping non-PCMU frame - PT: " + std::to_string(frame.payloadType) +
                " Seq: " + std::to_string(frame.seqNum));
            continue;
        }
        for (uint8_t b : frame.payload) {
            pcm.push_back(DecodeMuLawSample(b));
        }
    }
    return pcm;
}

void AudioReconstructor::WriteWavFile(const std::string& filename, const std::vector<int16_t>& pcmData, int sampleRate) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        Logger::GetInstance().Log(LogLevel::ERROR_, "    Cannot open WAV output file: " + filename);
        return;
    }

    int32_t dataSize = static_cast<int32_t>(pcmData.size() * sizeof(int16_t));
    int32_t chunkSize = 36 + dataSize;
    int16_t numChannels = 1;
    int16_t bitsPerSample = 16;
    int32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
    int16_t blockAlign = numChannels * bitsPerSample / 8;
    int32_t subchunk1Size = 16;
    int16_t audioFormat = 1; // PCM

    out.write("RIFF", 4);
    out.write(reinterpret_cast<char*>(&chunkSize), 4);
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    out.write(reinterpret_cast<char*>(&subchunk1Size), 4);
    out.write(reinterpret_cast<char*>(&audioFormat), 2);
    out.write(reinterpret_cast<char*>(&numChannels), 2);
    out.write(reinterpret_cast<char*>(&sampleRate), 4);
    out.write(reinterpret_cast<char*>(&byteRate), 4);
    out.write(reinterpret_cast<char*>(&blockAlign), 2);
    out.write(reinterpret_cast<char*>(&bitsPerSample), 2);
    out.write("data", 4);
    out.write(reinterpret_cast<char*>(&dataSize), 4);
    out.write(reinterpret_cast<const char*>(pcmData.data()), dataSize);

    out.close();
    Logger::GetInstance().Log(LogLevel::DEBUG,
        "    WAV written: " + filename + " (" + std::to_string(pcmData.size()) + " samples, " +
        std::to_string(dataSize) + " bytes)");
}

void AudioReconstructor::ReconstructAll(const std::unordered_map<uint32_t, std::vector<RtpFrame>>& streams,
    const std::string& outputDir) {
    for (const auto& streamPair : streams) {
        uint32_t ssrc = streamPair.first;
        std::vector<RtpFrame> frames = streamPair.second; // copy - we sort locally

        SortFramesBySequence(frames);

        std::vector<int16_t> pcm = DecodePcmuFrames(frames);
        if (pcm.empty()) {
            Logger::GetInstance().Log(LogLevel::WARNING,
                "    SSRC " + std::to_string(ssrc) + " - no PCMU audio decoded, skipping WAV write");
            continue;
        }

        std::string filename = outputDir + "\\stream_" + std::to_string(ssrc) + ".wav";
        WriteWavFile(filename, pcm, 8000); // PCMU is fixed at 8000 Hz per RFC 3551
    }
}