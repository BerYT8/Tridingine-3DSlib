#include <sys/system_language.h>
#include <string>

#if defined(PLATFORM_PC)
#if defined(_WIN32)
#include <windows.h>

std::string WStringToString(const std::wstring& wstr)
{
    if (wstr.empty())
        return {};

    int sizeNeeded = WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.c_str(),
        (int)wstr.size(),
        nullptr,
        0,
        nullptr,
        nullptr
    );

    std::string result(sizeNeeded, 0);

    WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.c_str(),
        (int)wstr.size(),
        result.data(),
        sizeNeeded,
        nullptr,
        nullptr
    );

    return result;
}

std::string GetSysLang()
{
    wchar_t localeName[LOCALE_NAME_MAX_LENGTH];

    if (GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH))
    {
        std::wstring ws(localeName);
        return WStringToString(ws); // ✔ conversión correcta UTF-8
    }

    return "unknown";
}
#elif defined(__linux__) || defined(__APPLE__)

#include <clocale>
#include <cstdlib>
#include <string>

std::string GetSysLang()
{
    // Intenta obtener el locale de mensajes del sistema
    const char* lang = std::setlocale(LC_MESSAGES, nullptr);

    if (!lang)
        lang = std::getenv("LANG");

    if (!lang)
        return "unknown";

    // Normalmente viene como:
    // "es_ES.UTF-8", "en_US.UTF-8"
    std::string s(lang);

    // Opcional: normalizar formato a "es-ES"
    size_t dot = s.find('.');
    if (dot != std::string::npos)
        s = s.substr(0, dot);

    for (char& c : s)
    {
        if (c == '_')
            c = '-';
    }

    return s;
}
#endif
#elif defined(PLATFORM_3DS)
#include <3ds.h>

std::string GetSysLang()
{
    u8 lang = 0;
    Result res = CFGU_GetSystemLanguage(&lang);

    switch (lang)
    {
        case CFG_LANGUAGE_JP: return "ja-JP";
        case CFG_LANGUAGE_EN: return "en-US";
        case CFG_LANGUAGE_FR: return "fr-FR";
        case CFG_LANGUAGE_DE: return "de-DE";
        case CFG_LANGUAGE_IT: return "it-IT";
        case CFG_LANGUAGE_ES: return "es-ES";
        case CFG_LANGUAGE_ZH: return "zh-CN";
        case CFG_LANGUAGE_KO: return "ko-KR";
        case CFG_LANGUAGE_NL: return "nl-NL";
        case CFG_LANGUAGE_PT: return "pt-PT";
        case CFG_LANGUAGE_RU: return "ru-RU";
        case CFG_LANGUAGE_TW: return "zh-TW"; // chino tradicional
        default: return "unknown";
    }
}
#else
std::string GetSysLang()
{
    return "unknown";
}
#endif


std::string language = "";

void System_SetCurrentLang(const char *lang)
{
    language = lang;
}

const char* System_GetCurrentLang()
{
    static std::string sysLang;

    if (language.empty()) {
        sysLang = GetSysLang();
        return sysLang.c_str();
    }

    return language.c_str();
}