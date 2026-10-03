#include <AmiigoLang.h>
#include <JsonUtils.h>
#include <unordered_map>
#include <switch.h>

namespace {
    std::unordered_map<std::string, std::u32string> langStrings;
    std::unordered_map<std::string, std::u32string> fallbackStrings;
    std::unordered_map<std::string, std::u32string> sentinels;

    void loadInto(std::unordered_map<std::string, std::u32string>& map, const char* path) {
        JsonDoc j = loadJsonFile(path);
        if (j.is_discarded() || !j.is_object()) return;
        for (auto it = j.begin(); it != j.end(); ++it) {
            if (it.value().is_string()) map[it.key()] = Amiigo::Lang::utf8ToU32(it.value().get<std::string>());
        }
    }
}

namespace Amiigo::Lang {
    std::u32string utf8ToU32(const std::string& utf8) {
        std::u32string result;
        size_t i = 0;
        while (i < utf8.size()) {
            unsigned char c = utf8[i];
            char32_t cp;
            if (c < 0x80) {
                cp = c; i++;
            } else if ((c & 0xE0) == 0xC0) {
                cp = c & 0x1F; i++;
                cp = (cp << 6) | (static_cast<unsigned char>(utf8[i++]) & 0x3F);
            } else if ((c & 0xF0) == 0xE0) {
                cp = c & 0x0F; i++;
                cp = (cp << 6) | (static_cast<unsigned char>(utf8[i++]) & 0x3F);
                cp = (cp << 6) | (static_cast<unsigned char>(utf8[i++]) & 0x3F);
            } else {
                cp = c & 0x07; i++;
                cp = (cp << 6) | (static_cast<unsigned char>(utf8[i++]) & 0x3F);
                cp = (cp << 6) | (static_cast<unsigned char>(utf8[i++]) & 0x3F);
                cp = (cp << 6) | (static_cast<unsigned char>(utf8[i++]) & 0x3F);
            }
            result += cp;
        }
        return result;
    }

    std::string u32ToUtf8(const std::u32string& u32) {
        std::string result;
        for (char32_t c : u32) {
            if (c < 0x80) {
                result += static_cast<char>(c);
            } else if (c < 0x800) {
                result += static_cast<char>(0xC0 | (c >> 6));
                result += static_cast<char>(0x80 | (c & 0x3F));
            } else if (c < 0x10000) {
                result += static_cast<char>(0xE0 | (c >> 12));
                result += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
                result += static_cast<char>(0x80 | (c & 0x3F));
            } else {
                result += static_cast<char>(0xF0 | (c >> 18));
                result += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
                result += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
                result += static_cast<char>(0x80 | (c & 0x3F));
            }
        }
        return result;
    }

    void init() {
        static const char* languageNames[SetLanguage_Total] = {
            "ja", "en-US", "fr", "de", "it", "es", "zh-CN", "ko",
            "nl", "pt", "ru", "zh-TW", "en-GB", "fr-CA", "es-419",
            "zh-Hans", "zh-Hant", "pt-BR"
        };

        std::string lang = "en-GB";
        setInitialize();
        u64 languageCode;
        SetLanguage setLanguage;
        if (R_SUCCEEDED(setGetSystemLanguage(&languageCode)) &&
            R_SUCCEEDED(setMakeLanguage(languageCode, &setLanguage)) &&
            setLanguage < SetLanguage_Total) {
                lang = languageNames[setLanguage];
            }
        setExit();

        loadInto(fallbackStrings, "romfs:/lang/en-GB.json");
        loadInto(langStrings, ("romfs:/lang/" + lang + ".json").c_str());
    }

    const std::u32string& get(const char* key) {
        auto it = langStrings.find(key);
        if (it != langStrings.end()) return it->second;
        auto fbIt = fallbackStrings.find(key);
        if (fbIt != fallbackStrings.end()) return fbIt->second;
        auto& sentinel = sentinels[key];
        if (sentinel.empty()) sentinel = utf8ToU32(key);
        return sentinel;
    }

    std::string getNarrow(const char* key) {
        return u32ToUtf8(get(key));
    }
}
