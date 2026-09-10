#include "Logger.h"

Logger& Logger::GetInstance() {
	static Logger instance;
	return instance;
}

void Logger::Log(LogLevel level, const std::string& message)
{
	// implement the log function
	std::string logLevel;
	switch (level)
	{
	case LogLevel::LINK_LAYER_INFO:
		logLevel = "LINK_LAYER_INFO";
		std::cout << "\033[0;32m" << " [" << logLevel << "] " << message << "\033[0m" << "\n";
		break;
	case LogLevel::NETWORK_LAYER_INFO:
		logLevel = "NETWORK_LAYER_INFO";
		std::cout << "\033[0;34m" << " [" << logLevel << "] " << message << "\033[0m" << "\n";
		break;
	case LogLevel::TRANSPORT_LAYER_INFO:
		logLevel = "TRANSPORT_LAYER_INFO";
		std::cout << "\033[0;33m" << " [" << logLevel << "] " << message << "\033[0m" << "\n";
		break;
	case LogLevel::APPLICATION_LAYER_INFO:
		logLevel = "APPLICATION_LAYER_INFO";
		std::cout << "\033[0;35m" << " [" << logLevel << "] " << message << "\033[0m" << "\n";
		break;
	case LogLevel::ERROR_:
		logLevel = "ERROR";
		std::cerr << "\033[0;31m" << " [" << logLevel << "] " << message << "\033[0m" << "\n";
		break;
	case LogLevel::WARNING:
		logLevel = "WARNING";
		std::cerr << "\033[0;36m" << " [" << logLevel << "] " << message << "\033[0m" << "\n";
		break;
	case LogLevel::DEBUG:
		logLevel = "DEBUG";
		std::cout << "\033[0;37m" << " [" << logLevel << "] " << message << "\033[0m" << "\n";
		break;
	}
}