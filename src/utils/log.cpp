#include <ctime>
#include <fstream>
#include <string>

void logMessage(const std::string &level, const std::string &msg)
{
	std::ofstream file("log.txt", std::ios::app);

	std::time_t t = std::time(nullptr);
	std::string datetime = std::ctime(&t);
	datetime.pop_back();
	std::string output = "[" + datetime + "]" + level + ": " + msg + "\n";
	file << output;
}