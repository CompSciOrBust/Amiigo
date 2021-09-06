#include <Networking.h>
#include <curl/curl.h>
#include <string>
#include <switch.h>


bool retrieveToFile(std::string URL, std::string path)
{
    if(!hasNetworkConnection()) return false;
    FILE* file = fopen(path.c_str(), "wb");
    if(file)
    {
        CURL *curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Amiigo");
        curl_easy_setopt(curl, CURLOPT_CAINFO, "romfs:/certificate.pem");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](const char* in, std::size_t size, std::size_t num, FILE* out){
            fwrite(in, size, num, out);
            return (size * num);
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    fclose(file);
    return true;
}

//https://github.com/XorTroll/Goldleaf/blob/e1f5f9f9c797911e1902df37df2f0cdcc8940868/Goldleaf/source/net/net_Network.cpp#L48
std::string retrieveToString(std::string URL, std::string mimeType)
{
    CURL *curl = curl_easy_init();
    if(!mimeType.empty())
    {
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
    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StringWriteImpl);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](const char* in, std::size_t size, std::size_t count, std::string *out){
        const auto total_size = size * count;
        out->append(in, total_size);
        return total_size;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return content;
}

bool hasNetworkConnection()
{
    NifmInternetConnectionStatus status;
    nifmGetInternetConnectionStatus(nullptr, nullptr, &status);
    return status == NifmInternetConnectionStatus_Connected;
}