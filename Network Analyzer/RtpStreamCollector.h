#pragma once
// RtpStreamCollector.h

#include <vector>
#include <unordered_map>
#include <cstdint>
#include "RtpLayer.h"

struct RtpFrame {
    uint16_t seqNum;
    uint32_t timestamp;
    uint8_t  payloadType;
    std::vector<uint8_t> payload;
};

class RtpStreamCollector {
public:
    static RtpStreamCollector& GetInstance();

    void CollectFrame(const pcpp::RtpLayer& rtpLayer);
    void CollectFrame(uint32_t ssrc, RtpFrame frame);
    const std::unordered_map<uint32_t, std::vector<RtpFrame>>& GetStreams() const;
    void Reset(); // clear state between file runs

private:
    RtpStreamCollector() {}
    RtpStreamCollector(const RtpStreamCollector&) = delete;
    RtpStreamCollector& operator=(const RtpStreamCollector&) = delete;

    std::unordered_map<uint32_t, std::vector<RtpFrame>> ssrcStreams;
};