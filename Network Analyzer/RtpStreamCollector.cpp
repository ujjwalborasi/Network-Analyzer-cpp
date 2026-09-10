#include "RtpStreamCollector.h"
#include "Logger.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

RtpStreamCollector& RtpStreamCollector::GetInstance() {
    static RtpStreamCollector instance;
    return instance;
}

void RtpStreamCollector::CollectFrame(const pcpp::RtpLayer& rtpLayer) {
    uint32_t ssrc = rtpLayer.getSSRC();

    RtpFrame frame;
    frame.seqNum = rtpLayer.getSequenceNumber();
    frame.timestamp = rtpLayer.getTimestamp();
    frame.payloadType = rtpLayer.getRtpHeader()->payloadType;

    const uint8_t* payloadPtr = rtpLayer.getLayerPayload();
    size_t payloadSize = rtpLayer.getLayerPayloadSize();

    // Sanity check: is there actually payload data here?
    if (payloadPtr == nullptr || payloadSize == 0) {
        Logger::GetInstance().Log(LogLevel::WARNING,
            "    RTP payload EMPTY - SSRC: " + std::to_string(ssrc) +
            " Seq: " + std::to_string(frame.seqNum));
        return;
    }

    frame.payload.assign(payloadPtr, payloadPtr + payloadSize);
    CollectFrame(ssrc, std::move(frame));
    ssrcStreams[ssrc].push_back(std::move(frame));

    // Confirm it landed + hex dump for verification against Wireshark
    std::ostringstream hexDump;
    for (size_t i = 0; i < payloadSize; ++i) {
        hexDump << std::hex << std::setw(2) << std::setfill('0') << (int)payloadPtr[i];
    }
    Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO,
        "    RTP payload captured - Size: " + std::to_string(payloadSize) +
        " bytes, PT: " + std::to_string(frame.payloadType) +
        " Seq: " + std::to_string(frame.seqNum));
    Logger::GetInstance().Log(LogLevel::DEBUG,
        "    RTP Payload hex: " + hexDump.str());
}
void RtpStreamCollector::CollectFrame(uint32_t ssrc, RtpFrame frame) {
    std::ostringstream hexDump;
    for (uint8_t b : frame.payload)
        hexDump << std::hex << std::setw(2) << std::setfill('0') << (int)b;

    Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO,
        "    RTP payload captured - Size: " + std::to_string(frame.payload.size()) +
        " bytes, PT: " + std::to_string(frame.payloadType) +
        " Seq: " + std::to_string(frame.seqNum) +
        " SSRC: " + std::to_string(ssrc));
    Logger::GetInstance().Log(LogLevel::DEBUG, "    RTP Payload hex: " + hexDump.str());

    ssrcStreams[ssrc].push_back(std::move(frame));
}


const std::unordered_map<uint32_t, std::vector<RtpFrame>>& RtpStreamCollector::GetStreams() const {
    return ssrcStreams;
}

void RtpStreamCollector::Reset() {
    ssrcStreams.clear();
}