#pragma once
#include <string>

/**
* @brief Logger function
* @param[in] level  The error level/type, which occures.
* @param[in] msg    The corresponding message, that discribes the error in detail.
*/
void logMessage(const std::string &level, const std::string &msg);
