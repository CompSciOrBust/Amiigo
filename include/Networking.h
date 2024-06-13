#pragma once
#include <switch.h>
#include <curl/curl.h>
#include <string>

bool retrieveToFile(std::string URL, std::string path);
bool retrieveToString(std::string URL, std::string mimeType, std::string *out);
bool hasNetworkConnection();
