#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct AmiiboEntry
{
    std::string name;
    bool isCategory;
};

struct AmiiboCreatorData
{
    std::string name;
    std::string gameName;
    std::string amiiboSeries;
    unsigned short game_character_id;
    char character_variant;
    char figure_type;
    unsigned short model_number;
    char series;
};

bool checkIfFileExists(char* path);
bool read_DB(nlohmann::json& base,std::string path);
bool write_DB(nlohmann::json base,std::string path);
std::vector<AmiiboEntry> scanForAmiibo(const char* path);
std::vector<std::string> getListOfSeries();
std::vector<AmiiboCreatorData> getAmiibosFromSeries(std::string series);
void createVirtualAmiibo(AmiiboCreatorData amiibo);
void firstTimeSetup();
bool checkForUpdates();