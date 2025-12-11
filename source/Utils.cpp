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
#include <nlohmann/json.hpp>

#include <arribaText.h>
#include <codecvt> // TODO: Replace with custom transcoder

bool checkIfFileExists(const char* path) {
    return !access(path, F_OK);
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
            amiibo.name = Arriba::Text::ASCIIToUnicode(entry->d_name);
            char flagPath[512] = "";
            strcat(flagPath, path);
            strcat(flagPath, "/");
            strcat(flagPath, entry->d_name);
            amiibo.path = flagPath;
            strcat(flagPath, "/amiibo.flag");
            amiibo.isCategory = !checkIfFileExists(flagPath);
            amiibos.push_back(amiibo);
        }
        closedir(folder);
        // Sort alphabetically
        std::sort(amiibos.begin(), amiibos.end(), [](AmiiboEntry amiiboA, AmiiboEntry amiiboB)->bool{
            int maxLength = (amiiboA.name.length() < amiiboB.name.length()) ? amiiboA.name.length() : amiiboB.name.length();
            int iterations = 0;
            while (iterations < maxLength) {
                if (std::tolower(amiiboA.name[iterations]) != std::tolower(amiiboB.name[iterations]))
                    return std::tolower(amiiboA.name[iterations]) < std::tolower(amiiboB.name[iterations]);
                else
                    iterations++;
            }
            return false;
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
        return amiibos;
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
            amiibos.push_back({
                Arriba::Text::ASCIIToUnicode(tempLine.substr(tempLine.find_last_of('/')+1, tempLine.length()+1 - tempLine.find_last_of('/')).c_str()),
                !checkIfFileExists(flagPath),
                tempLine});
        }
        fileStream.close();
        return amiibos;
    }
}

std::vector<std::string> getListOfSeries() {
    std::vector<std::string> series;
    if (checkIfFileExists("sdmc:/config/amiigo/API.json")) {
        std::string APIData;
        std::string tempLine;
        std::ifstream fileStream("sdmc:/config/amiigo/API.json");
        // Read the file from disk
        while (getline(fileStream, tempLine)) {
            APIData += tempLine;
        }
        fileStream.close();
        nlohmann::json APIJson = nlohmann::json::parse(APIData, nullptr, false);
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
            for (size_t i = 0; i < series.size(); i++) {
                if (series[i] == seriesName) isKnown = true;
            }
            // If not add it to the list
            if (!isKnown) series.push_back(seriesName);
        }
    } else {
        return {"Error, no API cache!"};
    }

    // Sort alphabetically
    std::sort(series.begin(), series.end(), [](std::string seriesA, std::string seriesB)->bool{
        int maxLength = (seriesA.length() < seriesB.length()) ? seriesA.length() : seriesB.length();
        int iterations = 0;
        while (iterations < maxLength) {
            if (std::tolower(seriesA[iterations]) != std::tolower(seriesB[iterations]))
                return std::tolower(seriesA[iterations]) < std::tolower(seriesB[iterations]);
            else
                iterations++;
        }
        return false;
    });
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
        std::wstring APIData;
        std::wstring tempLine;
        std::wifstream fileStream("sdmc:/config/amiigo/API.json");
        std::locale utf8_locale(std::locale(), new std::codecvt_utf8<wchar_t>);
        fileStream.imbue(utf8_locale);
        // Read the file from disk
        while (getline(fileStream, tempLine)) {
            APIData += tempLine;
        }
        nlohmann::json APIJson = nlohmann::json::parse(APIData);
        fileStream.close();
        // Loop over every entry under the AMiibo object
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
    std::sort(amiibos.begin(), amiibos.end(), [](AmiiboCreatorData amiiboA, AmiiboCreatorData amiiboB)->bool{
        int maxLength = (amiiboA.name.length() < amiiboB.name.length()) ? amiiboA.name.length() : amiiboB.name.length();
        int iterations = 0;
        while (iterations < maxLength) {
            if (std::tolower(amiiboA.name[iterations]) != std::tolower(amiiboB.name[iterations]))
                return std::tolower(amiiboA.name[iterations]) < std::tolower(amiiboB.name[iterations]);
            else
                iterations++;
        }
        return false;
    });
    return amiibos;
}

std::string sanitizePathString(std::u32string path) {
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

void createVirtualAmiibo(AmiiboCreatorData amiibo) {
    std::string pathBase = "sdmc:/emuiibo/amiibo/";
    switch (Amiigo::Settings::categoryMode) {
        case Amiigo::Settings::saveByGameName:
        pathBase += sanitizePathString(amiibo.gameName) + "/";
        mkdir(pathBase.c_str(), 0);
        break;
    
        case Amiigo::Settings::saveByAmiiboSeries:
        pathBase += sanitizePathString(amiibo.amiiboSeries) + "/";
        mkdir(pathBase.c_str(), 0);
        break;

        case Amiigo::Settings::saveByCurrentFolder:
        if (Amiigo::UI::selectorPath != "Favorites") pathBase = Amiigo::UI::selectorPath + "/";
        break;;
    }
    pathBase += sanitizePathString(amiibo.name);
    mkdir(pathBase.c_str(), 0);
    std::ofstream fileStream(pathBase + "/amiibo.flag");
    fileStream.close();
    fileStream.open(pathBase + "/amiibo.json");
    nlohmann::json amiiboJson;
    amiiboJson["name"] = amiibo.name;
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
    fileStream << std::setw(4) << amiiboJson << std::endl;
    fileStream.close();

    // TODO: Save images in a different thread to avoid hanging the UI
    if (Amiigo::Settings::saveAmiiboImages) {
        std::string imagePath = pathBase + "/amiibo.png";
        if (!retrieveToFile(amiibo.imageURL, imagePath)) Amiigo::UI::updateStatusError(U"Failed to save Amiibo image");
    }
}

void firstTimeSetup() {
    // Extract local copy of the Amiibo API data
    if (!checkIfFileExists("sdmc:/config/amiigo/API.json")) {
        unzFile zipFile = unzOpen("romfs:/API.cache");
        unz_file_info fileInfo;
        unzOpenCurrentFile(zipFile);
        unzGetCurrentFileInfo(zipFile, &fileInfo, nullptr, 0, nullptr, 0, nullptr, 0);
        void* buffer = malloc(437793);
        FILE* outFile = fopen("sdmc:/config/amiigo/API.json", "wb");
        for (int i = unzReadCurrentFile(zipFile, buffer, 437793); i > 0; i = unzReadCurrentFile(zipFile, buffer, 437793)) fwrite(buffer, 1, i, outFile);
        fclose(outFile);
        free(buffer);
    }
    
    // Install emuiibo
    if (!checkIfFileExists("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp")) {
        bool hasValidEmuiiboJSON = false;
        nlohmann::json emuiiboInfo;
        while (!hasValidEmuiiboJSON) {
            printf("Downloading Emuiibo zip\n");
            while (!hasNetworkConnection()) continue;
            if (checkIfFileExists("sdmc:/config/amiigo/emuiibo.tmp")) remove("sdmc:/config/amiigo/emuiibo.tmp");
            // We should probably do this in a more robust way
            std::string emuiiboReleaseInfo;
            while (!retrieveToString("https://api.github.com/repos/XorTroll/Emuiibo/releases", "application/json", &emuiiboReleaseInfo)) continue;
            emuiiboInfo = nlohmann::json::parse(emuiiboReleaseInfo, nullptr, false);
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
            unzOpenCurrentFile(zipFile);
            unzGetCurrentFileInfo(zipFile, &fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0);
            printf("Zip index:%d is %s\n", i, fileName);
            if (strcmp("SdOut/atmosphere/contents/0100000000000352/exefs.nsp", fileName) == 0) {
                void* buffer = malloc(500000);
                FILE* outfile = fopen("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp", "wb");
                for (int j = unzReadCurrentFile(zipFile, buffer, 500000); j > 0; j = unzReadCurrentFile(zipFile, buffer, 500000)) {
                    fwrite(buffer, 1, j, outfile);
                }
                fclose(outfile);
                free(buffer);
                break;
            }
            unzCloseCurrentFile(zipFile);
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
    }
    // If flag exists download update
    if (checkIfFileExists("sdmc:/config/amiigo/update.flag")) {
        while (!hasNetworkConnection()) continue;
        bool DLSuccess = retrieveToFile(Amiigo::Settings::updateURL, "sdmc:/switch/Failed_Amiigo_Update.nro");
        if (checkIfFileExists("sdmc:/switch/Failed_Amiigo_Update.nro") && DLSuccess) {
            romfsExit();
            if (checkIfFileExists(Amiigo::Settings::amiigoPath)) remove(Amiigo::Settings::amiigoPath);
            rename("sdmc:/switch/Failed_Amiigo_Update.nro", Amiigo::Settings::amiigoPath);
        }
        if (Amiigo::Settings::amiigoPath) envSetNextLoad(Amiigo::Settings::amiigoPath, Amiigo::Settings::amiigoPath);
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
    long unsigned int time = Amiigo::Settings::updateTime;
    timeGetCurrentTime(TimeType_NetworkSystemClock, &time);
    timeExit();

    // Only check for updates once every 24 hours, unless an update is found.
    if (Amiigo::Settings::updateTime > time) {
        printf("Last check less than 24 hours ago\n");
        return false;
    }
    printf("Getting API data\n");
    std::string amiigoReleaseInfo;
    bool releaseInfoSuccess = retrieveToString("https://api.github.com/repos/CompSciOrBust/Amiigo/releases", "application/json", &amiigoReleaseInfo);
    // User is probably being rate limited
    if (amiigoReleaseInfo.size() < 300 || !releaseInfoSuccess) {
        printf("%s\n", amiigoReleaseInfo.c_str());
        printf("Error, getting Amiigo update info failed\n");
        return false;
    }
    nlohmann::json amiigoInfoParsed = nlohmann::json::parse(amiigoReleaseInfo, nullptr, false);

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
    } else {Amiigo::Settings::updateURL = amiigoInfoParsed[0]["assets"][0]["browser_download_url"].get<std::string>().c_str();}
    return true;
}
