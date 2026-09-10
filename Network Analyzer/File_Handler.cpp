#include "File_Handler.h"
#include "Packet_Dissector.h"
#include "DataHandler.h"
#include "Logger.h"
#include "RtpStreamCollector.h"
#include "AudioReconstructor.h"  

FileHandler& FileHandler::GetInstance() {
    static FileHandler instance;
    return instance;
}
void FileHandler::HandleFile(const std::string& filePath) {
    // Check if the file path is empty
    if (filePath.empty()) {
        Logger::GetInstance().Log(LogLevel::ERROR_, "Error: File path is empty");
        return;
    }
    // Check the file extension and call the appropriate handler
    if (filePath.find(".pcap") != std::string::npos) {
        HandlePcapFile(filePath);
    }
    else if (filePath.find(".txt") != std::string::npos) {
        HandleTxtFile(filePath);
    }
    else if (filePath.find(".bin") != std::string::npos) {
        HandleBinFile(filePath);
    }
    else {
        Logger::GetInstance().Log(LogLevel::ERROR_, "Error: Unsupported file type");
    }
}
void FileHandler::HandlePcapFile(const std::string& filePath) {

    Logger::GetInstance().Log(LogLevel::DEBUG, "Handling pcap file: " + filePath);
    
    // Write code to read pcap file and print packet size for each packet
    pcpp::IFileReaderDevice* reader = pcpp::IFileReaderDevice::getReader(filePath);
    if (!reader->open()) {
        Logger::GetInstance().Log(LogLevel::ERROR_, "Error: Cannot open pcap file");
        delete reader;
        return;
    }
	Logger::GetInstance().Log(LogLevel::DEBUG, "Successfully opened pcap file: " + filePath);
    
    pcpp::RawPacket rawPacket;
    int num = 1;
    // Blue color    \e[0;34m
    // Pink color    \e[0;35m
    // Cyan color    \e[0;36m
    // Green color   \e[0;32m
    // Orange color  \e[0;33m
    // Yellow color  \e[0;33m
    // Red color     \e[0;31m
    // White color   \e[0;37m
    // Grey color    \e[0;90m
    // Reset color   \e[0m
    // Magenta color \e[0;35m

    while (reader->getNextPacket(rawPacket)) {
        Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    Packet Number: " + std::to_string(num));
        Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    Packet length: " + std::to_string(rawPacket.getRawDataLen()));
        pcpp::Packet parsedPacket(&rawPacket);
        DataHandler::GetInstance().printHexDump(parsedPacket);
        DataHandler::GetInstance().printASCII(parsedPacket);
        PacketDissector::GetInstance().DissectPacket(parsedPacket);
        num++;
    }
    reader->close();
    delete reader;
    //ReconstructAllRtpStreams();

    Logger::GetInstance().Log(LogLevel::DEBUG, "    ---- RTP Stream Summary ----");
    for (const auto& streamPair : RtpStreamCollector::GetInstance().GetStreams()) {
        uint32_t ssrc = streamPair.first;
        const auto& frames = streamPair.second;
        size_t totalBytes = 0;
        for (const auto& f : frames) totalBytes += f.payload.size();

        Logger::GetInstance().Log(LogLevel::DEBUG,
            "    SSRC: " + std::to_string(ssrc) +
            " - Frames: " + std::to_string(frames.size()) +
            " - Total payload bytes: " + std::to_string(totalBytes));

    }
    // ADD FROM HERE:
    Logger::GetInstance().Log(LogLevel::DEBUG, "    ---- Reconstructing Audio ----");
    AudioReconstructor::GetInstance().ReconstructAll(
        RtpStreamCollector::GetInstance().GetStreams(),
        "."  // writes WAVs into current working directory - change if you want a specific folder
    );
    // TO HERE
}
void FileHandler::HandleTxtFile(const std::string& filePath) {
    Logger::GetInstance().Log(LogLevel::DEBUG, "Handling txt file: " + filePath);
    // Add code to handle txt file
}
void FileHandler::HandleBinFile(const std::string& filePath) {
    Logger::GetInstance().Log(LogLevel::DEBUG, "Handling bin file: " + filePath);
    // Add code to handle bin file
}
