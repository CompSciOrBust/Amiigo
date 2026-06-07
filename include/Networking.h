#pragma once

#include <switch.h>
#include <curl/curl.h>
#include <optional>
#include <string>
#include <vector>

std::optional<std::vector<char>> downloadToRAM(const std::string& url);
bool retrieveToFile(const std::string& URL, const std::string& path);
std::optional<std::string> downloadToString(const std::string& URL);
bool hasNetworkConnection();
