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

std::optional<std::vector<char>> downloadToRAM(const std::string& url) {
    if (!hasNetworkConnection()) return std::nullopt;
    CURL *curl = curl_easy_init();
    if (!curl) return std::nullopt;
    std::vector<char> outData;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Amiigo");
    curl_easy_setopt(curl, CURLOPT_CAINFO, "romfs:/certificate.pem");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, memDataWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outData);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

    CURLcode errorCode = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (errorCode != CURLE_OK) {
        printf("ERROR: Failed to download %s\n", url.c_str());
        return std::nullopt;
    }
    return outData;
}

bool retrieveToFile(std::string URL, std::string path) {
    if (!hasNetworkConnection()) return false;
    std::ofstream file(path.c_str(), std::ofstream::trunc | std::ofstream::binary);
    if (file) {
        CURL *curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Amiigo");
        curl_easy_setopt(curl, CURLOPT_CAINFO, "romfs:/certificate.pem");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](const char* in, std::size_t size, std::size_t num, std::ofstream* out){
            out->write(in, size * num);
            return (size * num);
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        CURLcode errorCode = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (errorCode != CURLE_OK) {
            printf("ERROR: failed to download %s to %s\nCurl error code:%d\n", URL.c_str(), path.c_str(), errorCode);
            file.close();
            return false;
        }
    } else {
        return false;
    }
    file.close();
    return true;
}

bool downloadToString(std::string URL, std::string *out) {
    auto data = downloadToRAM(URL);
    if (!data) {
        printf("Failed to download data from %s\n", URL.c_str());
        return false;
    }
    printf("Downloaded %zu bytes from %s\n", data->size(), URL.c_str());
    out->assign(data->begin(), data->end());
    return true;
}

bool hasNetworkConnection() {
    NifmInternetConnectionStatus status;
    nifmGetInternetConnectionStatus(nullptr, nullptr, &status);
    return status == NifmInternetConnectionStatus_Connected;
}
