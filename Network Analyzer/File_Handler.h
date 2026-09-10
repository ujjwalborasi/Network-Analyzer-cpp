#pragma once
#include <string>
#include <iostream>
#include "Packet.h"
#include "PcapFileDevice.h"
/**
 * @class FileHandler
 * @brief A singleton class to handle different types of files.
 */
class FileHandler {
public:
    /**
     * @brief Get the single instance of FileHandler.
     * @return Reference to the single instance of FileHandler.
     */
    static FileHandler& GetInstance();

    /**
     * @brief Handle a file based on its type.
     * @param filePath The path of the file to handle.
     */
    void HandleFile(const std::string& filePath);

private:
    // Private constructor to prevent instantiation.
    FileHandler() {}
    // Delete copy constructor and assignment operator to prevent copying.
    FileHandler(const FileHandler&) = delete;
    FileHandler& operator=(const FileHandler&) = delete;

    // Private methods to handle specific file types.
    void HandlePcapFile(const std::string& filePath);
    void HandleTxtFile(const std::string& filePath);
    void HandleBinFile(const std::string& filePath);
};

