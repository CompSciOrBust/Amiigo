#include <Networking.h>
#include <curl/curl.h>
#include <switch.h>
#include <string>
#include <fstream>
#include <vector>

size_t memDataWriteCallback(char* inData, size_t chunkSize, size_t numChunks, void* outData_) {
    std::vector<char>* outData = (std::vector<char>*)outData_;
    size_t realSize = chunkSize * numChunks;
    outData->insert(outData->end(), inData, inData + realSize);
    return realSize;
}

size_t fileDataWriteCallback(char* inData, size_t chunkSize, size_t numChunks, void* outFile) {
    size_t realSize = chunkSize * numChunks;
    static_cast<std::ofstream*>(outFile)->write(inData, realSize);
    return realSize;
}

static void configureCurl(CURL* curl, const std::string& url, curl_write_callback writeFunc, void* writeData) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Amiigo");
    curl_easy_setopt(curl, CURLOPT_CAINFO, "romfs:/certificate.pem");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, writeData);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
}

std::optional<std::vector<char>> downloadToRAM(const std::string& url) {
    if (!hasNetworkConnection()) return std::nullopt;
    CURL* curl = curl_easy_init();
    if (!curl) return std::nullopt;
    std::vector<char> outData;

    configureCurl(curl, url, memDataWriteCallback, &outData);

    CURLcode errorCode = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (errorCode != CURLE_OK) {
        printf("ERROR: Failed to download %s\n", url.c_str());
        return std::nullopt;
    }
    return outData;
}

bool retrieveToFile(const std::string& url, const std::string& path) {
    if (!hasNetworkConnection()) return false;
    std::ofstream file(path.c_str(), std::ofstream::trunc | std::ofstream::binary);
    if (!file) return false;
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    configureCurl(curl, url, fileDataWriteCallback, &file);
    CURLcode errorCode = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (errorCode != CURLE_OK) {
        printf("ERROR: failed to download %s to %s\nCurl error code:%d\n", url.c_str(), path.c_str(), errorCode);
        return false;
    }
    return true;
}

std::optional<std::string> downloadToString(const std::string& URL) {
    auto data = downloadToRAM(URL);
    if (!data) {
        printf("Failed to download data from %s\n", URL.c_str());
        return std::nullopt;
    }
    printf("Downloaded %zu bytes from %s\n", data->size(), URL.c_str());
    return std::string(data->begin(), data->end());
}

bool hasNetworkConnection() {
    NifmInternetConnectionStatus status;
    nifmGetInternetConnectionStatus(nullptr, nullptr, &status);
    return status == NifmInternetConnectionStatus_Connected;
}
