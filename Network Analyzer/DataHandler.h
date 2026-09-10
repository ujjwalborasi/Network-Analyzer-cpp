#pragma once
#include "Logger.h"
#include "File_Handler.h"

class DataHandler
{
	public:
		static DataHandler& GetInstance();
		void printHexDump(const pcpp::Packet& packet);
		void printASCII(const pcpp::Packet& packet);
};