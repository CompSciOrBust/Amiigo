#pragma once
#include <curl/curl.h>
#include <string>
#include <switch.h>

bool retrieveToFile(std::string URL, std::string path);
bool retrieveToString(std::string URL, std::string mimeType, std::string *out);
bool hasNetworkConnection();