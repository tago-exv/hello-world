#pragma once
#include <string>
#include <vector>

std::string GetConfigString(const std::string& filePath, const char* pSectionName, const char* pKeyName);
std::vector<std::string> split(std::string& input, char delimiter);

