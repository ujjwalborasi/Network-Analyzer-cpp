#include "DataHandler.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include "RawPacket.h"
#include "Packet.h"

DataHandler& DataHandler::GetInstance() {
	static DataHandler instance;
	return instance;
}

void DataHandler::printHexDump(const pcpp::Packet& packet)
{
    const uint8_t* rawData = packet.getRawPacket()->getRawData();
    size_t dataLen = packet.getRawPacket()->getRawDataLen();

    std::ostringstream oss;
    const size_t bytesPerLine = 16;

    for (size_t i = 0; i < dataLen; i += bytesPerLine) {
        oss << std::setw(4) << std::setfill('0') << std::hex << i << "  ";

        for (size_t j = 0; j < bytesPerLine; ++j) {
            if (i + j < dataLen) {
                oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(rawData[i + j]) << " ";
            }
            else {
                oss << "   ";
            }
        }

        oss << " ";

        for (size_t j = 0; j < bytesPerLine; ++j) {
            if (i + j < dataLen) {
                unsigned char byte = rawData[i + j];
                if (std::isprint(byte)) {
                    oss << byte;
                }
                else {
                    oss << '.';
                }
            }
        }

        oss << std::endl;
    }

    Logger::GetInstance().Log(LogLevel::DEBUG, "    Hex Dump:\n" + oss.str());

}

void DataHandler::printASCII(const pcpp::Packet& packet)
{
    const uint8_t* rawData = packet.getRawPacket()->getRawData();
    size_t dataLen = packet.getRawPacket()->getRawDataLen();

    std::string ascii;
    for (size_t i = 0; i < dataLen; ++i) {
        if (std::isprint(rawData[i])) {
            ascii += rawData[i];
        }
        else {
            ascii += '.';
        }
    }

    Logger::GetInstance().Log(LogLevel::DEBUG, "    ASCII:\n" + ascii);
}
// Returned to this project after an year 
