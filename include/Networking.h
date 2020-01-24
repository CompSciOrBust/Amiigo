#include <switch.h>
#include <curl/curl.h>
#include <string>

std::size_t CurlStrWrite(const char* in, std::size_t size, std::size_t num, std::string* out);
std::size_t CurlFileWrite(const char* in, std::size_t size, std::size_t num, FILE* out);
std::string RetrieveContent(std::string URL, std::string MIMEType);
void RetrieveToFile(std::string, std::string);
std::string FormatURL(std::string TextToFormat);
bool HasConnection();
void APIDownloader();
void Scandownload(std::string folder);