#include <iostream>
#include <fstream>
#include <ctime>
#include <string>

/**
* @brief Logger-function. Usage:
* logMessage("INFO", "Programm gestartet");
* logMessage("ERROR", "Fehler aufgetreten");
* ...
*/

void logMessage(const std::string& level, const std::string& msg) {
    std::ofstream file("log.txt", std::ios::app);

    std::time_t t = std::time(nullptr);
    std::string datetime = std::ctime(&t);
    datetime.pop_back();
    std::string output = "[" + datetime +"]" + level + ": "+msg + "\n";
    file << output; // can be found in: \out\build\x64-debug\log.txt
    std::cout << output;
}