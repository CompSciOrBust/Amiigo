#pragma once
#include <curl/curl.h>
#include <string>
#include <switch.h>

std::size_t curlWriteFile(const char* in, std::size_t size, std::size_t num, FILE* out);
bool retrieveToFile(std::string URL, std::string path);
std::string retrieveToString(std::string URL, std::string mimeType);
bool hasNetworkConnection();