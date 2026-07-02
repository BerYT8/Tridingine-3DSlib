#pragma once

#include <unordered_map>
#include <string>
#include <variant>
#include <vector>

using SaveValueVariant = std::variant<int, float, char, std::string>;

struct SaveEntry
{
    std::string name;
    SaveValueVariant value;
};

using SaveEntryArray = std::vector<SaveEntry>;

using SaveMap = std::unordered_map<
    std::string,
    SaveValueVariant>;

using AnySaveType = std::variant<SaveEntryArray, SaveMap>;

void save_all(const std::string &path, AnySaveType entries);

void save_async(const std::string &path, SaveEntryArray entries);
void save_async_map(const std::string &path, SaveMap entries);

SaveMap load_all(const std::string &path);

template <typename T>
T get_value(const SaveMap &data, const std::string &key, T def)
{
    auto it = data.find(key);

    if (it != data.end() && std::holds_alternative<T>(it->second))
        return std::get<T>(it->second);

    return def;
}

bool isValueInt(SaveValueVariant value);
bool isValueFloat(SaveValueVariant value);
bool isValueChar(SaveValueVariant value);
bool isValueString(SaveValueVariant value);