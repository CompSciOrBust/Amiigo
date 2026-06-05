#include <Networking.h>
#include <curl/curl.h>
#include <switch.h>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>

size_t memDataWriteCallback(char* inData, size_t chunkSize, size_t numChunks, void* outData_) {
    std::vector<char>* outData = (std::vector<char>*)outData_;
    size_t realSize = chunkSize * numChunks;
    outData->insert(outData->end(), inData, inData + realSize);
    return realSize;
}

bool downloadToRAM(std::string URL, char* &buffer, int &dataSize) {
    if (!hasNetworkConnection()) return false;
    CURL *curl = curl_easy_init();
    if (!curl) return false;
    std::vector<char> outData;
    
    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Amiigo");
    curl_easy_setopt(curl, CURLOPT_CAINFO, "romfs:/certificate.pem");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, memDataWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outData);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

    CURLcode errorCode = curl_easy_perform(curl);
    if (errorCode != CURLE_OK) {
        printf("ERROR: Failed to download %s\n", URL.c_str());
        curl_easy_cleanup(curl);
        return false;
    }

    buffer = (char*)malloc(outData.size());
    dataSize = outData.size();
    memcpy(buffer, outData.data(), dataSize);

    curl_easy_cleanup(curl);
    return true;
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
    char* downloadedData;
    int dataSize = 0;
    bool success = downloadToRAM(URL, downloadedData, dataSize);
    if (!success) {
        printf("Failed to download data from %s\n", URL.c_str());
        return false;
    }
    printf("Downloaded %d bytes from %s\n", dataSize, URL.c_str());
    out->clear();
    out->assign(downloadedData, dataSize);
    free(downloadedData);
    return true;
}

bool hasNetworkConnection() {
    NifmInternetConnectionStatus status;
    nifmGetInternetConnectionStatus(nullptr, nullptr, &status);
    return status == NifmInternetConnectionStatus_Connected;
}
