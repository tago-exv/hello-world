#include <Windows.h>  // GetPrivateProfileString
#include <string>
#include <sstream>
#include <array>
#include <vector>

using namespace std;

string GetConfigString(const std::string& filePath, const char* pSectionName, const char* pKeyName)
{
    if (filePath.empty()) {
        return "";
    }
    std::array<char, MAX_PATH> buf = {};
    GetPrivateProfileStringA(pSectionName, pKeyName, "", &buf.front(), static_cast<DWORD>(buf.size()), filePath.c_str());
    return &buf.front();
}

vector<string> split(string& input, char delimiter)
{
    istringstream stream(input);
    string field;
    vector<string> result;
    while (getline(stream, field, delimiter)) {
        result.push_back(field);
    }
    return result;
}
