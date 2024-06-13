#include <Networking.h>
#include <curl/curl.h>
#include <switch.h>
#include <string>
#include <fstream>

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
    }
    file.close();
    return true;
}

// https://github.com/XorTroll/Goldleaf/blob/e1f5f9f9c797911e1902df37df2f0cdcc8940868/Goldleaf/source/net/net_Network.cpp#L48
bool retrieveToString(std::string URL, std::string mimeType, std::string *out) {
    CURL *curl = curl_easy_init();
    if (!mimeType.empty()) {
        curl_slist *header_data = curl_slist_append(header_data, ("Content-Type: " + mimeType).c_str());
        header_data = curl_slist_append(header_data, ("Accept: " + mimeType).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_data);
    }
    std::string content;
    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Amiigo");
    curl_easy_setopt(curl, CURLOPT_CAINFO, "romfs:/certificate.pem");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StringWriteImpl);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](const char* in, std::size_t size, std::size_t count, std::string *out){
        const auto total_size = size * count;
        out->append(in, total_size);
        return total_size;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
    CURLcode errorCode = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (errorCode != CURLE_OK) {
        printf("ERROR: failed to download %s\nCurl error code:%d\n", URL.c_str(), errorCode);
        return false;
    }
    return true;
}

bool hasNetworkConnection() {
    NifmInternetConnectionStatus status;
    nifmGetInternetConnectionStatus(nullptr, nullptr, &status);
    return status == NifmInternetConnectionStatus_Connected;
}
