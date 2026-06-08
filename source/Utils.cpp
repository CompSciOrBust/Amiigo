#include <utils.h>

#include <dirent.h>
#include <unistd.h>
#include <minizip/unzip.h>

#include <cstring>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>

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
    DIR* folder = opendir(path);
    std::vector<AmiiboEntry> amiibos;
    if (folder) {
        dirent* entry;
        while ((entry = readdir(folder)) != nullptr) {
            AmiiboEntry amiibo;
            amiibo.path = std::string(path) + "/" + entry->d_name;
            std::string flagCheckPath = amiibo.path + "/amiibo.flag";
            amiibo.isCategory = !checkIfFileExists(flagCheckPath.c_str());
            amiibo.name = getAmiiboDisplayName(amiibo.path, amiibo.isCategory);
            amiibos.push_back(amiibo);
        }
        closedir(folder);

        std::sort(amiibos.begin(), amiibos.end(), [](const AmiiboEntry& a, const AmiiboEntry& b) {
            return caseInsensitiveSort(a.name, b.name);
        });

        if (!strcmp(path, "sdmc:/emuiibo/amiibo")) {
            amiibos.insert(amiibos.begin(), {U"★Favorites", true, "Favorites"});
        } else {
            std::string cleanPath(path);
            amiibos.insert(amiibos.begin(), {U"← Back", true, cleanPath.substr(0, cleanPath.find_last_of("/"))});
        }
    }

    if (!strcmp(path, "Favorites")) {
        amiibos.insert(amiibos.begin(), {U"← Back", true, "sdmc:/emuiibo/amiibo"});
        std::string tempLine;
        std::ifstream fileStream("sdmc:/emuiibo/overlay/favorites.txt");
        while (getline(fileStream, tempLine)) {
            std::string flagCheckPath = tempLine + "/amiibo.flag";
            bool isCategory = !checkIfFileExists(flagCheckPath.c_str());
            amiibos.push_back({getAmiiboDisplayName(tempLine, isCategory), isCategory, tempLine});
        }
        fileStream.close();
    }
    return amiibos;
}

std::vector<std::string> getListOfSeries() {
    if (!checkIfFileExists("sdmc:/config/amiigo/API.json")) return {"Error, no API cache!"};

    JsonDoc APIJson = loadJsonFile("sdmc:/config/amiigo/API.json");
    if (APIJson.is_discarded()) {
        printf("API cache is corrupt\n");
        remove("sdmc:/config/amiigo/API.json");
        return {"Error, API cache corrupt!", "Try updating cache in settings!"};
    }

    std::unordered_set<std::string> seen;
    std::vector<std::string> series;
    for (int i = 0; i < APIJson["amiibo"].size(); i++) {
        std::string seriesName = APIJson["amiibo"][i]["amiiboSeries"].get<std::string>();
        if (seen.insert(seriesName).second) series.push_back(seriesName);
    }

    std::sort(series.begin(), series.end(), caseInsensitiveSort<std::string>);
    return series;
}

// Written on 27/01/2021 for Kronos, can't remember how it works but does magic bit shifting
unsigned short shiftAndDec(const std::string& input) {
    unsigned short value = std::stoi(input, nullptr, 16);
    unsigned short a = value & 0xFF00;
    a = 0x00FF & (a >> 8);
    value = value << 8;
    value = 0xffff & value;
    value = value | a;
    return value;
}

std::vector<AmiiboCreatorData> getAmiibosFromSeries(const std::string& series) {
    if (!checkIfFileExists("sdmc:/config/amiigo/API.json")) return {{U"Error, API cache vanished?", U"", U"", 0, 0, 0}};
    JsonDoc APIJson = loadJsonFile("sdmc:/config/amiigo/API.json");

    std::vector<AmiiboCreatorData> amiibos;
    for (int i = 0; i < APIJson["amiibo"].size(); i++) {
        if (APIJson["amiibo"][i]["amiiboSeries"].get<std::string>() != series) continue;
        // Process the API data the same way Emutool does
        // https://github.com/XorTroll/emuiibo/blob/90cbc54a95c0aa4a9ceb6dd55b633de206763094/emutool/emutool/AmiiboUtils.cs#L144
        AmiiboCreatorData newAmiibo;
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> ASCIIToUnicodeConverter;
        newAmiibo.name = ASCIIToUnicodeConverter.from_bytes(APIJson["amiibo"][i]["name"].get<std::string>().c_str());
        std::string fullID = APIJson["amiibo"][i]["head"].get<std::string>() + APIJson["amiibo"][i]["tail"].get<std::string>();
        // Var names taken from emutool
        std::string character_game_id_str = fullID.substr(0, 4);
        std::string character_variant_str = fullID.substr(4, 2);
        std::string figure_type_str = fullID.substr(6, 2);
        std::string model_no_str = fullID.substr(8, 4);
        std::string series_str = fullID.substr(12, 2);
        // Swap endianess for game ID
        newAmiibo.game_character_id = shiftAndDec(character_game_id_str);
        newAmiibo.character_variant = static_cast<char>(stoi(character_variant_str, nullptr, 16));
        newAmiibo.figure_type = static_cast<char>(stoi(figure_type_str, nullptr, 16));
        newAmiibo.model_number = (unsigned short)stoi(model_no_str, nullptr, 16);
        newAmiibo.series = static_cast<char>(stoi(series_str, nullptr, 16));
        newAmiibo.gameName = ASCIIToUnicodeConverter.from_bytes(APIJson["amiibo"][i]["gameSeries"].get<std::string>().c_str()); // only used for categorization
        newAmiibo.amiiboSeries = ASCIIToUnicodeConverter.from_bytes(APIJson["amiibo"][i]["amiiboSeries"].get<std::string>().c_str()); // only used for categorization
        newAmiibo.imageURL = APIJson["amiibo"][i]["image"].get<std::string>();

        amiibos.push_back(newAmiibo);
    }

    std::sort(amiibos.begin(), amiibos.end(), [](const AmiiboCreatorData& a, const AmiiboCreatorData& b) {
        return caseInsensitiveSort(a.name, b.name);
    });
    return amiibos;
}

std::string sanitizeAmiiboName(const std::u32string& name) {
    static const std::unordered_map<char32_t, char> replacements = {
        {U'é', 'e'}, {U'ō', 'o'}, {U'É', 'E'}, {U'“', '\''}, {U'”', '\''}
    };
    std::string output;
    output.reserve(name.size());
    for (char32_t c : name) {
        auto it = replacements.find(c);
        output += (it != replacements.end()) ? it->second : static_cast<char>(c);
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

void saveAmiiboImage(const std::string& pathBase, const AmiiboCreatorData& amiibo) {
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

void createVirtualAmiibo(const AmiiboCreatorData& amiibo) {
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

static std::vector<char> extractSingleFileZip(const char* zipPath) {
    unzFile zipFile = unzOpen(zipPath);
    unz_file_info fileInfo;
    unzOpenCurrentFile(zipFile);
    unzGetCurrentFileInfo(zipFile, &fileInfo, nullptr, 0, nullptr, 0, nullptr, 0);
    printf("Extracting %s. Uncompressed size: %ld bytes\n", zipPath, fileInfo.uncompressed_size);
    std::vector<char> buffer(fileInfo.uncompressed_size);
    unzReadCurrentFile(zipFile, buffer.data(), buffer.size());
    unzCloseCurrentFile(zipFile);
    unzClose(zipFile);
    return buffer;
}

static bool extractFileFromZip(const char* zipPath, const char* entryName, const char* destPath) {
    unzFile zipFile = unzOpen(zipPath);
    unz_global_info zipInfo;
    unzGetGlobalInfo(zipFile, &zipInfo);
    for (int i = 0; i < zipInfo.number_entry; i++) {
        char fileName[FS_MAX_PATH];
        unz_file_info fileInfo;
        unzGetCurrentFileInfo(zipFile, &fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0);
        printf("Zip index:%d is %s\n", i, fileName);
        if (strcmp(entryName, fileName) == 0) {
            unzOpenCurrentFile(zipFile);
            void* buffer = malloc(500000);
            FILE* outFile = fopen(destPath, "wb");
            for (int j = unzReadCurrentFile(zipFile, buffer, 500000); j > 0; j = unzReadCurrentFile(zipFile, buffer, 500000))
                fwrite(buffer, 1, j, outFile);
            fclose(outFile);
            free(buffer);
            unzCloseCurrentFile(zipFile);
            unzClose(zipFile);
            return true;
        }
        unzGoToNextFile(zipFile);
    }
    unzClose(zipFile);
    return false;
}

static void setupAPICache() {
    if (checkIfFileExists("sdmc:/config/amiigo/API.json")) return;
    std::ofstream out("sdmc:/config/amiigo/API.json");
    auto APIData = downloadToString("https://amiiboapi.org/api/amiibo/");
    if (APIData) {
        out << *APIData;
    } else {
        auto buffer = extractSingleFileZip("romfs:/API.cache");
        out.write(buffer.data(), buffer.size());
    }
}

static void installEmuiibo() {
    if (checkIfFileExists("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp")) return;
    JsonDoc emuiiboInfo;
    bool hasValidEmuiiboJSON = false;
    while (!hasValidEmuiiboJSON) {
        printf("Downloading Emuiibo zip\n");
        while (!hasNetworkConnection()) continue;
        if (checkIfFileExists("sdmc:/config/amiigo/emuiibo.tmp")) remove("sdmc:/config/amiigo/emuiibo.tmp");
        std::optional<std::string> emuiiboReleaseInfo;
        while (!(emuiiboReleaseInfo = downloadToString("https://api.github.com/repos/XorTroll/Emuiibo/releases"))) continue;
        emuiiboInfo = parseJsonString(*emuiiboReleaseInfo);
        hasValidEmuiiboJSON = !emuiiboInfo.is_discarded();
    }
    while (!retrieveToFile(emuiiboInfo[0]["assets"][0]["browser_download_url"].get<std::string>(), "sdmc:/config/amiigo/emuiibo.tmp")) continue;

    printf("Unzipping\n");
    mkdir("sdmc:/atmosphere/contents/", 0);
    mkdir("sdmc:/atmosphere/contents/0100000000000352/", 0);
    mkdir("sdmc:/atmosphere/contents/0100000000000352/flags/", 0);
    std::ofstream fileStream("sdmc:/atmosphere/contents/0100000000000352/flags/boot2.flag");
    fileStream.close();
    extractFileFromZip("sdmc:/config/amiigo/emuiibo.tmp", "SdOut/atmosphere/contents/0100000000000352/exefs.nsp", "sdmc:/atmosphere/contents/0100000000000352/exefs.nsp");
    printf("Unzip done\n");
    remove("sdmc:/config/amiigo/emuiibo.tmp");

    pmshellInitialize();
    NcmProgramLocation emuiiboLoc = {0x0100000000000352, NcmStorageId_None};
    pmshellLaunchProgram(0, &emuiiboLoc, nullptr);
    pmshellExit();
    while (!emu::IsAvailable()) continue;
}

static void applyPendingUpdate() {
    if (!checkIfFileExists("sdmc:/config/amiigo/update.flag")) return;
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

void firstTimeSetup() {
    setupAPICache();
    installEmuiibo();
    applyPendingUpdate();
}

bool checkForUpdates() {
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
    auto amiigoReleaseInfo = downloadToString("https://api.github.com/repos/CompSciOrBust/Amiigo/releases");
    // User is probably being rate limited
    if (!amiigoReleaseInfo || amiigoReleaseInfo->size() < 300) {
        printf("%s\n", amiigoReleaseInfo.value_or("").c_str());
        printf("Error, getting Amiigo update info failed\n");
        return false;
    }

    JsonDoc amiigoInfoParsed = parseJsonString(*amiigoReleaseInfo);
    if (amiigoInfoParsed.is_discarded()) {
        printf("%s\n", amiigoReleaseInfo->c_str());
        printf("Error, Amiigo update info corrupt\n");
        return false;
    }

    // If on the latest version wait another 24 hours before checking again
    if (amiigoInfoParsed[0]["tag_name"].get<std::string>() == VERSION) {
        Amiigo::Settings::updateTime = time + 86400;
        Amiigo::Settings::saveSettings();
        return false;
    } else {
        updateURL = amiigoInfoParsed[0]["assets"][0]["browser_download_url"].get<std::string>();
    }
    return true;
}
