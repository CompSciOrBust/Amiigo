#pragma once

#include <switch.h>
#include <curl/curl.h>
#include <string>

bool downloadToRAM(std::string URL, char* &buffer, int &dataSize);
bool retrieveToFile(std::string URL, std::string path);
bool downloadToString(std::string URL, std::string *out);
bool hasNetworkConnection();
