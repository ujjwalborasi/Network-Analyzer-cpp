#pragma once
//write a class to handle Logging and log levels like LINK_LAYER_INFO, NETWORK_LAYER_INFO, TRANSPORT_LAYER_INFO, APPLICATION_LAYER_INFO
#include <iostream>
#include <string>


enum class LogLevel {
	LINK_LAYER_INFO,
	NETWORK_LAYER_INFO,
	TRANSPORT_LAYER_INFO,
	APPLICATION_LAYER_INFO,
	ERROR_,
	WARNING,
	DEBUG
};

class Logger {
public:
	static Logger& GetInstance();
	void Log(LogLevel level, const std::string& message);

};