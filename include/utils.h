#pragma once

#include <string>
#include <vector>
#include <WorkerQueue.h>

struct AmiiboEntry {
    std::u32string name;
    bool isCategory;
    std::string path;
};

struct AmiiboCreatorData {
    std::u32string name;
    std::u32string gameName;
    std::u32string amiiboSeries;
    std::string imageURL;
    unsigned short game_character_id;
    char character_variant;
    char figure_type;
    unsigned short model_number;
    char series;
};

bool checkIfFileExists(const char* path);
unsigned char* scaleImageToFit(unsigned char* src, int w, int h, int channels, int maxSize, int& outW, int& outH);
std::vector<AmiiboEntry> scanForAmiibo(const char* path);
std::vector<std::string> getListOfSeries();
std::vector<AmiiboCreatorData> getAmiibosFromSeries(std::string series);
void saveAmiiboImage(std::string pathBase, AmiiboCreatorData amiibo);
void createVirtualAmiibo(AmiiboCreatorData amiibo);
void firstTimeSetup();
bool checkForUpdates();
