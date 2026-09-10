#include <iostream>
#include "File_Handler.h"
#include "Logger.h"
int main(int argc, char* argv[]) {
    // Check if a file path was provided
    if (argc < 2) {
        
        Logger::GetInstance().Log(LogLevel::ERROR_, "Usage: " + std::string(argv[0]) + " <file_path>");  
        return 1;
    }
    // The first command line argument after the program name is the file path
    std::string filePath = argv[1];
    // Get the single instance of FileHandler
    FileHandler& fileHandler = FileHandler::GetInstance();

	// print the file name and path to the console

	Logger::GetInstance().Log(LogLevel::DEBUG, "File path: " + filePath);

    // Handle the file based on its type
    fileHandler.HandleFile(filePath);
    return 0;
    // Logger::GetInstance().Log(LogLevel::DEBUG, "Program started successfully.");
    
    

}

