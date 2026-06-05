#pragma once

#include <switch.h>
#include <curl/curl.h>
#include <optional>
#include <string>
#include <vector>

std::optional<std::vector<char>> downloadToRAM(const std::string& url);
bool retrieveToFile(std::string URL, std::string path);
bool downloadToString(std::string URL, std::string *out);
bool hasNetworkConnection();
