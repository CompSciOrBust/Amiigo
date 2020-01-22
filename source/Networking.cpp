#include <switch.h>
#include <curl/curl.h>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <AmiigoUI.h>
#include <chrono>
#include <thread>
#include "Utils.h"

//Stolen from Goldleaf
//Thank you XOR
std::size_t CurlStrWrite(const char* in, std::size_t size, std::size_t num, std::string* out)
{
    const size_t totalBytes(size * num);
    out->append(in, totalBytes);
    return totalBytes;
}

std::size_t CurlFileWrite(const char* in, std::size_t size, std::size_t num, FILE* out)
{
    fwrite(in, size, num, out);
    return (size * num);
}

std::string RetrieveContent(std::string URL, std::string MIMEType)
{
    socketInitializeDefault();
    std::string cnt;
    CURL *curl = curl_easy_init();
    if(!MIMEType.empty())
    {
        curl_slist *headerdata = NULL;
        headerdata = curl_slist_append(headerdata, ("Content-Type: " + MIMEType).c_str());
        headerdata = curl_slist_append(headerdata, ("Accept: " + MIMEType).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerdata);
    }
    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Amiigo"); //Turns out this was important and I should not have deleted it
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlStrWrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cnt);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    socketExit();
    return cnt;
}

void RetrieveToFile(std::string URL, std::string Path)
{
    socketInitializeDefault();
    FILE *f = fopen(Path.c_str(), "wb");
    if(f)
    {
        CURL *curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Amiigo");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlFileWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    fclose(f);
    socketExit();
}

//I made this so even though it's only one two calls it's probably janky.
std::string FormatURL(std::string TextToFormat)
{
	CURL *curl = curl_easy_init();
	return curl_easy_escape(curl, TextToFormat.c_str(), 0);
}

//More stuff from Xortroll Industries
bool HasConnection()
{
    u32 strg = 0;
	nifmInitialize(NifmServiceType_User);
    nifmGetInternetConnectionStatus(NULL, &strg, NULL);
	return (strg > 0);
}

void APIDownloader()
{
	mkdir("sdmc:/config/amiigo/", 0);
	if(HasConnection())
	RetrieveToFile("https://www.amiiboapi.com/api/amiibo", "sdmc:/config/amiigo/API-D.json");
	if(CheckFileExists("sdmc:/config/amiigo/API-D.json"))
	rename("sdmc:/config/amiigo/API.json", "sdmc:/config/amiigo/API-old.json");
	rename("sdmc:/config/amiigo/API-D.json", "sdmc:/config/amiigo/API.json");
	remove("sdmc:/config/amiigo/API-old.json");
	remove("sdmc:/config/amiigo/API-D.json");
	mkdir("sdmc:/config/amiigo/IMG/", 0);
	mkdir("sdmc:/config/amiigo/IMG/Cache/", 0);
		ifstream DataFileReader("sdmc:/config/amiigo/API.json");
		string AmiiboAPIString;
		getline(DataFileReader, AmiiboAPIString);
		DataFileReader.close();		
	if(json::accept(AmiiboAPIString))
	{
		//Parse and use the JSON data
		json JData;
		int JDataSize = 0;
		JData = json::parse(AmiiboAPIString);
		JDataSize = JData["amiibo"].size();
		
		//Get all of the Series' names and add Amiibos to the AmiiboVarsVec
		for(int i = 0; i < JDataSize; i++)
		{
			string amiiboicon = JData["amiibo"][i]["image"].get<std::string>();
			string amiiID = "sdmc:/config/amiigo/IMG/"+JData["amiibo"][i]["head"].get<std::string>()+JData["amiibo"][i]["tail"].get<std::string>()+".png";
			string amiifail = "sdmc:/config/amiigo/IMG/Cache/"+JData["amiibo"][i]["head"].get<std::string>()+JData["amiibo"][i]["tail"].get<std::string>()+".png";
			if(!CheckFileExists(amiiID))
			{
				RetrieveToFile(amiiboicon, amiifail);
				rename(amiifail.c_str(), amiiID.c_str());
			}
		}
	}
}

void IconDownloader()
{
	
}