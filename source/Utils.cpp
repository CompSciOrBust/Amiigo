#include <utils.h>

#include <dirent.h>
#include <unistd.h>
#include <minizip/unzip.h>

#include <cstring>
#include <fstream>
#include <iomanip>

#include <AmiigoSettings.h>
#include <AmiigoUI.h>
#include <Networking.h>
#include <emuiibo.hpp>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize2.h>
#include <stb_image_write.h>

#include <arribaText.h>
#include <codecvt> // TODO: Replace with custom transcoder

static std::string updateURL;

bool checkIfFileExists(const char* path) {
    return !access(path, F_OK);
}

template<typename Str>
static bool caseInsensitiveSort(const Str& a, const Str& b) {
    size_t maxLength = std::min(a.length(), b.length());
    for (size_t i = 0; i < maxLength; i++) {
        auto ca = std::tolower(a[i]);
        auto cb = std::tolower(b[i]);
        if (ca != cb) return ca < cb;
    }
    return false;
}

static std::u32string getAmiiboDisplayName(const std::string& path, bool isCategory) {
    if (isCategory) return Arriba::Text::ASCIIToUnicode(path.substr(path.find_last_of('/') + 1).c_str());
    return Arriba::Text::ASCIIToUnicode(readJsonField<std::string>(path + "/amiibo.json", "name", "Corrupt Amiibo Data").c_str());
}

std::vector<AmiiboEntry> scanForAmiibo(const char* path) {
    // Open path
    DIR* folder = opendir(path);
    std::vector<AmiiboEntry> amiibos;
    if (folder) {
        // List folder entries
        dirent* entry;
        while (entry = readdir(folder)) {
            AmiiboEntry amiibo;
            char flagPath[512] = "";
            strcat(flagPath, path);
            strcat(flagPath, "/");
            strcat(flagPath, entry->d_name);
            amiibo.path = flagPath;
            strcat(flagPath, "/amiibo.flag");
            amiibo.isCategory = !checkIfFileExists(flagPath);
            amiibo.name = getAmiiboDisplayName(amiibo.path, amiibo.isCategory);
            amiibos.push_back(amiibo);
        }
        closedir(folder);
        // Sort alphabetically
        std::sort(amiibos.begin(), amiibos.end(), [](const AmiiboEntry& a, const AmiiboEntry& b) {
            return caseInsensitiveSort(a.name, b.name);
        });
        // Prepend favorites if path is sdmc:/emuiibo/amiibo
        if (!strcmp(path, "sdmc:/emuiibo/amiibo")) {
            amiibos.insert(amiibos.begin(), {U"★Favorites", true, "Favorites"});
        } else {
            // If not in "sdmc:/emuiibo/amiibo" then add back entry
            std::string upDir = path;
            upDir = upDir.substr(0, upDir.find_last_of("/"));
            amiibos.insert(amiibos.begin(), {U"← Back", true, upDir});
        }
    }
    // Check if path is supposed to be favorites
    if (!strcmp(path, "Favorites")) {
        // Add in the back button
        amiibos.insert(amiibos.begin(), {U"← Back", true, "sdmc:/emuiibo/amiibo"});
        // Read each line from the favorites file
        std::string tempLine;
        std::ifstream fileStream("sdmc:/emuiibo/overlay/favorites.txt");
        while (getline(fileStream, tempLine)) {
            // Check if Amiibo or dir
            char flagPath[512] = "";
            strcat(flagPath, tempLine.c_str());
            strcat(flagPath, "/amiibo.flag");
            // Add to amiibo list
            bool isCategory = !checkIfFileExists(flagPath);
            amiibos.push_back({getAmiiboDisplayName(tempLine, isCategory), isCategory, tempLine});
        }
        fileStream.close();
    }
    return amiibos;
}

std::vector<std::string> getListOfSeries() {
    std::vector<std::string> series;
    if (checkIfFileExists("sdmc:/config/amiigo/API.json")) {
        JsonDoc APIJson = loadJsonFile("sdmc:/config/amiigo/API.json");
        // Check API cache is valid
        if (APIJson.is_discarded()) {
            printf("API cache is corrupt\n");
            remove("sdmc:/config/amiigo/API.json");
            return {"Error, API cache corrupt!", "Try updating cache in settings!"};
        }
        // Loop over every entry under the Amiibo object
        for (int i = 0; i < APIJson["amiibo"].size(); i++) {
            bool isKnown = false;
            std::string seriesName = APIJson["amiibo"][i]["amiiboSeries"].get<std::string>();
            // Check if series is in list
            for (size_t j = 0; j < series.size(); j++) {
                if (series[j] == seriesName) {
                    isKnown = true;
                    break;
                }
            }
            // If not add it to the list
            if (!isKnown) series.push_back(seriesName);
        }
    } else {
        return {"Error, no API cache!"};
    }

    // Sort alphabetically
    std::sort(series.begin(), series.end(), caseInsensitiveSort<std::string>);
    return series;
}

// Written on 27/01/2021 for Kronos, can't remember how it works but does magic bit shifting
unsigned short shiftAndDec(std::string input) {
    unsigned short value = std::stoi(input, nullptr, 16);
    unsigned short a = value & 0xFF00;
    a = 0x00FF & (a >> 8);
    value = value << 8;
    value = 0xffff & value;
    value = value | a;
    return value;
}

std::vector<AmiiboCreatorData> getAmiibosFromSeries(std::string series) {
    std::vector<AmiiboCreatorData> amiibos;
    if (checkIfFileExists("sdmc:/config/amiigo/API.json")) {
        JsonDoc APIJson = loadJsonFile("sdmc:/config/amiigo/API.json");
        // Loop over every entry under the Amiibo object
        for (int i = 0; i < APIJson["amiibo"].size(); i++) {
            // If series matches add it to the list
            if (APIJson["amiibo"][i]["amiiboSeries"].get<std::string>() == series) {
                // Process the API data the same way Emutool does
                // https://github.com/XorTroll/emuiibo/blob/90cbc54a95c0aa4a9ceb6dd55b633de206763094/emutool/emutool/AmiiboUtils.cs#L144
                AmiiboCreatorData newAmiibo;
                std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> ASCIIToUnicodeConverter;
                newAmiibo.name = ASCIIToUnicodeConverter.from_bytes(APIJson["amiibo"][i]["name"].get<std::string>().c_str());
                std::string fullID = APIJson["amiibo"][i]["head"].get<std::string>() + APIJson["amiibo"][i]["tail"].get<std::string>();
                // Get strings needed to derive IDs
                // Var names taken from emutool
                std::string character_game_id_str = fullID.substr(0, 4);
                std::string character_variant_str = fullID.substr(4, 2);
                std::string figure_type_str = fullID.substr(6, 2);
                std::string model_no_str = fullID.substr(8, 4);
                std::string series_str = fullID.substr(12, 2);
                // Swap endianess for game ID
                newAmiibo.game_character_id = shiftAndDec(character_game_id_str);
                // Get character variant
                newAmiibo.character_variant = static_cast<char>(stoi(character_variant_str, nullptr, 16));
                // Get figure type
                newAmiibo.figure_type = static_cast<char>(stoi(figure_type_str, nullptr, 16));
                // Get model number
                newAmiibo.model_number = (unsigned short)stoi(model_no_str, nullptr, 16);
                // Get series ID
                newAmiibo.series = static_cast<char>(stoi(series_str, nullptr, 16));
                // Get the Game series name (only used for categorization)
                newAmiibo.gameName = ASCIIToUnicodeConverter.from_bytes(APIJson["amiibo"][i]["gameSeries"].get<std::string>().c_str());
                // Get the Amiibo series name (only used for categorization)
                newAmiibo.amiiboSeries = ASCIIToUnicodeConverter.from_bytes(APIJson["amiibo"][i]["amiiboSeries"].get<std::string>().c_str());
                // Get the Amiibo image URL
                newAmiibo.imageURL = APIJson["amiibo"][i]["image"].get<std::string>();
                // Add new amiibo to list
                amiibos.push_back(newAmiibo);
            }
        }
    } else {
        return {{U"Error, API cache vanished?", U"", U"", 0, 0, 0}};
    }

    // Sort alphabetically
    std::sort(amiibos.begin(), amiibos.end(), [](const AmiiboCreatorData& a, const AmiiboCreatorData& b) {
        return caseInsensitiveSort(a.name, b.name);
    });
    return amiibos;
}

std::string sanitizeAmiiboName(std::u32string path) {
    std::string output;
    for (char32_t c: path) {
        switch (c)
        {
        case U'é':
        output += 'e';
            break;
        case U'ō':
        output += (char)'o';
            break;
        case U'É':
        output += 'E';
            break;
        case U'“':
        output += '\'';
            break;
        case U'”':
        output += '\'';
            break;
        default:
        output += (char)c;
            break;
        }
    }
    return output;
}

std::vector<unsigned char> scaleImageToFit(unsigned char* src, int w, int h, int channels, int maxSize, int& outW, int& outH) {
    int largest = (w > h) ? w : h;
    outW = (maxSize * w) / largest;
    outH = (maxSize * h) / largest;
    std::vector<unsigned char> out(outW * outH * channels);
    stbir_resize_uint8_linear(src, w, h, 0, out.data(), outW, outH, 0, (stbir_pixel_layout)channels);
    return out;
}

void saveAmiiboImage(std::string pathBase, AmiiboCreatorData amiibo) {
    int maxImageSize = 512;
    std::string imagePath = pathBase + "/amiibo.png";
    auto imageData = downloadToRAM(amiibo.imageURL);
    if (!imageData) {
        MainThread::dispatch([]() { Amiigo::UI::updateStatusError(U"Failed to save Amiibo image"); });
        return;
    }
    int width, height, channels;
    unsigned char* input = stbi_load_from_memory((const stbi_uc*)imageData->data(), imageData->size(), &width, &height, &channels, 0);
    int outWidth = width, outHeight = height;
    const unsigned char* outImg = input;
    std::vector<unsigned char> scaled;
    if (width > maxImageSize || height > maxImageSize) {
        scaled = scaleImageToFit(input, width, height, channels, maxImageSize, outWidth, outHeight);
        outImg = scaled.data();
    }
    stbi_write_png(imagePath.c_str(), outWidth, outHeight, channels, outImg, outWidth * channels);
    stbi_image_free(input);
}

void createVirtualAmiibo(AmiiboCreatorData amiibo) {
    std::string pathBase = "sdmc:/emuiibo/amiibo/";
    switch (Amiigo::Settings::categoryMode) {
        case Amiigo::Settings::saveByGameName:
        pathBase += sanitizeAmiiboName(amiibo.gameName) + "/";
        mkdir(pathBase.c_str(), 0);
        break;
    
        case Amiigo::Settings::saveByAmiiboSeries:
        pathBase += sanitizeAmiiboName(amiibo.amiiboSeries) + "/";
        mkdir(pathBase.c_str(), 0);
        break;

        case Amiigo::Settings::saveByCurrentFolder:
        if (Amiigo::UI::getSelectorPath() != "Favorites") pathBase = Amiigo::UI::getSelectorPath() + "/";
        break;
    }
    pathBase += sanitizeAmiiboName(amiibo.name);
    mkdir(pathBase.c_str(), 0);
    std::ofstream fileStream(pathBase + "/amiibo.flag");
    fileStream.close();
    JsonDoc amiiboJson;
    amiiboJson["name"] = std::string(sanitizeAmiiboName(amiibo.name));
    amiiboJson["write_counter"] = 0;
    amiiboJson["version"] = 0;
    amiiboJson["mii_charinfo_file"] = "mii-charinfo.bin";
    amiiboJson["first_write_date"]["y"] = 2019;
    amiiboJson["first_write_date"]["m"] = 1;
    amiiboJson["first_write_date"]["d"] = 1;
    amiiboJson["last_write_date"]["y"] = 2019;
    amiiboJson["last_write_date"]["m"] = 1;
    amiiboJson["last_write_date"]["d"] = 1;
    amiiboJson["id"]["game_character_id"] = amiibo.game_character_id;
    amiiboJson["id"]["character_variant"] = amiibo.character_variant;
    amiiboJson["id"]["figure_type"] = amiibo.figure_type;
    amiiboJson["id"]["series"] = amiibo.series;
    amiiboJson["id"]["model_number"] = amiibo.model_number;
    if (!Amiigo::Settings::useRandomisedUUID)
        amiiboJson["uuid"] = {rand() % 256, rand() % 256, rand() % 256, rand() % 256, rand() % 256, rand() % 256, rand() % 256, 0, 0, 0};
    else
        amiiboJson["use_random_uuid"] = true;
    writeJsonFile(pathBase + "/amiibo.json", amiiboJson);

    if (Amiigo::Settings::saveAmiiboImages) workerQueue.enqueue(std::bind(saveAmiiboImage, pathBase, amiibo));
}

void firstTimeSetup() {
    // Get the API cache
    if (!checkIfFileExists("sdmc:/config/amiigo/API.json")) {
        std::string APIData;
        std::ofstream out("sdmc:/config/amiigo/API.json");
        bool downloadSuccess = downloadToString("https://amiiboapi.org/api/amiibo/", &APIData);
        if (downloadSuccess) {
            out << APIData;
        } else {
            // Extract local copy of the Amiibo API data
            unzFile zipFile = unzOpen("romfs:/API.cache");
            unz_file_info fileInfo;
            unzOpenCurrentFile(zipFile);
            unzGetCurrentFileInfo(zipFile, &fileInfo, nullptr, 0, nullptr, 0, nullptr, 0);
            unsigned long dataSize = fileInfo.uncompressed_size;
            printf("Extracting local API cache. Size is %ld bytes\n", dataSize);
            char* buffer = (char*)malloc(dataSize);
            unzReadCurrentFile(zipFile, buffer, dataSize);
            out.write(buffer, dataSize);
            free(buffer);
            unzCloseCurrentFile(zipFile);
            unzClose(zipFile);
        }
        out.close();
    }
    
    // Install emuiibo
    if (!checkIfFileExists("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp")) {
        bool hasValidEmuiiboJSON = false;
        JsonDoc emuiiboInfo;
        while (!hasValidEmuiiboJSON) {
            printf("Downloading Emuiibo zip\n");
            while (!hasNetworkConnection()) continue;
            if (checkIfFileExists("sdmc:/config/amiigo/emuiibo.tmp")) remove("sdmc:/config/amiigo/emuiibo.tmp");
            // We should probably do this in a more robust way
            std::string emuiiboReleaseInfo;
            while (!downloadToString("https://api.github.com/repos/XorTroll/Emuiibo/releases", &emuiiboReleaseInfo)) continue;
            emuiiboInfo = parseJsonString(emuiiboReleaseInfo);
            hasValidEmuiiboJSON = !emuiiboInfo.is_discarded();
        }
        printf("hasValidEmuiiboJSON: %d\n", hasValidEmuiiboJSON);
        while (!retrieveToFile(emuiiboInfo[0]["assets"][0]["browser_download_url"].get<std::string>().c_str(), "sdmc:/config/amiigo/emuiibo.tmp")) continue;
        printf("Unzipping\n");
        // Extract the files from the emuiibo zip
        mkdir("sdmc:/atmosphere/contents/", 0);
        mkdir("sdmc:/atmosphere/contents/0100000000000352/", 0);
        mkdir("sdmc:/atmosphere/contents/0100000000000352/flags/", 0);
        std::ofstream fileStream("sdmc:/atmosphere/contents/0100000000000352/flags/boot2.flag");
        fileStream.close();
        unzFile zipFile = unzOpen("sdmc:/config/amiigo/emuiibo.tmp");
        unz_global_info zipInfo;
        unzGetGlobalInfo(zipFile, &zipInfo);
        for (int i = 0; i < zipInfo.number_entry; i++) {
            char fileName[256];
            unz_file_info fileInfo;
            unzGetCurrentFileInfo(zipFile, &fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0);
            printf("Zip index:%d is %s\n", i, fileName);
            if (strcmp("SdOut/atmosphere/contents/0100000000000352/exefs.nsp", fileName) == 0) {
                unzOpenCurrentFile(zipFile);
                void* buffer = malloc(500000);
                FILE* outfile = fopen("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp", "wb");
                for (int j = unzReadCurrentFile(zipFile, buffer, 500000); j > 0; j = unzReadCurrentFile(zipFile, buffer, 500000)) {
                    fwrite(buffer, 1, j, outfile);
                }
                fclose(outfile);
                free(buffer);
                unzCloseCurrentFile(zipFile);
                break;
            }
            unzGoToNextFile(zipFile);
        }
        printf("Unzip done\n");
        unzClose(zipFile);
        remove("sdmc:/config/amiigo/emuiibo.tmp");
        // Launch the sysmodule
        pmshellInitialize();
        NcmProgramLocation emuiiboLoc = {0x0100000000000352, NcmStorageId_None};
        pmshellLaunchProgram(0, &emuiiboLoc, nullptr);
        pmshellExit();
        while(!emu::IsAvailable()) continue;
    }
    // If flag exists download update
    if (checkIfFileExists("sdmc:/config/amiigo/update.flag")) {
        while (!hasNetworkConnection()) continue;
        bool DLSuccess = retrieveToFile(updateURL, "sdmc:/switch/Failed_Amiigo_Update.nro");
        if (checkIfFileExists("sdmc:/switch/Failed_Amiigo_Update.nro") && DLSuccess) {
            romfsExit();
            if (checkIfFileExists(Amiigo::Settings::amiigoPath)) remove(Amiigo::Settings::amiigoPath);
            rename("sdmc:/switch/Failed_Amiigo_Update.nro", Amiigo::Settings::amiigoPath);
        }
        if (Amiigo::Settings::amiigoPath[0] != '\0') envSetNextLoad(Amiigo::Settings::amiigoPath, Amiigo::Settings::amiigoPath);
        Amiigo::UI::isRunning = 0;
        remove("sdmc:/config/amiigo/update.flag");
    }
}

bool checkForUpdates() {
    // Return false if no internet
    printf("Checking for updates\n");
    if (!hasNetworkConnection()) {
        printf("No connection\b");
        return false;
    }

    printf("Getting network time\n");
    timeInitialize();
    long unsigned int time = 0;
    if (R_FAILED(timeGetCurrentTime(TimeType_NetworkSystemClock, &time))) time = Amiigo::Settings::updateTime;
    timeExit();

    // Only check for updates once every 24 hours, unless an update is found.
    if (Amiigo::Settings::updateTime > time) {
        printf("Last check less than 24 hours ago\n");
        return false;
    }
    printf("Getting API data\n");
    std::string amiigoReleaseInfo;
    bool releaseInfoSuccess = downloadToString("https://api.github.com/repos/CompSciOrBust/Amiigo/releases", &amiigoReleaseInfo);
    // User is probably being rate limited
    if (amiigoReleaseInfo.size() < 300 || !releaseInfoSuccess) {
        printf("%s\n", amiigoReleaseInfo.c_str());
        printf("Error, getting Amiigo update info failed\n");
        return false;
    }
    JsonDoc amiigoInfoParsed = parseJsonString(amiigoReleaseInfo);

    // If data is corrupt do nothing
    if (amiigoInfoParsed.is_discarded()) {
        printf("%s\n", amiigoReleaseInfo.c_str());
        printf("Error, Amiigo update info corrupt\n");
        return false;
    }

    // If on the latest update wait another 24 hours before checking again
    if (amiigoInfoParsed[0]["tag_name"].get<std::string>() == VERSION) {
        Amiigo::Settings::updateTime = time + 86400;
        Amiigo::Settings::saveSettings();
        return false;
    } else {updateURL = amiigoInfoParsed[0]["assets"][0]["browser_download_url"].get<std::string>();}
    return true;
}
