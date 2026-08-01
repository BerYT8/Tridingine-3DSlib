#pragma once
#ifdef __cplusplus

#include <unordered_map>
#include <variant>
#include <string>
#include <vector>
#include <future>
#include <cstdint>


extern "C"
{
#include "types.h"
}

/* ========================================================= */
/* ===================== ASYNC TYPES ======================= */
/* ========================================================= */

using AV = std::variant<
    nullValue,
    bool,
    int32_t,
    uint32_t,
    float,
    double,
    std::string,
    std::vector<uint8_t>>;

class AsyncVariant
{
private:
    AV value;

public:
    AsyncVariant() = default;

    template <typename T>
    AsyncVariant(const T &v)
        : value(v)
    {
    }

    template<typename T>
    AsyncVariant& operator=(const T& v)
    {
        value = v;
        return *this;
    }
    //========================
    // is<T>()
    //========================

    template <typename T>
    bool is() const
    {
        return std::holds_alternative<T>(value);
    }

    //========================
    // as<T>()
    //========================

    template <typename T>
    T &asN()
    {
        return std::get<T>(value);
    }

    template <typename T>
    const T &as() const
    {
        return std::get<T>(value);
    }

    //========================
    // Helpers
    //========================

    bool isNull() const
    {
        return is<nullValue>();
    }

    bool isBool() const
    {
        return is<bool>();
    }

    bool isInt() const
    {
        return is<int32_t>();
    }

    bool isUInt() const
    {
        return is<uint32_t>();
    }

    bool isFloat() const
    {
        return is<float>();
    }

    bool isDouble() const
    {
        return is<double>();
    }

    bool isString() const
    {
        return is<std::string>();
    }

    bool isBytes() const
    {
        return is<std::vector<uint8_t>>();
    }
};

struct AsyncValue
{
    std::string name;
    AsyncVariant value;
};

class AsyncSaveData
{
private:
    std::vector<AsyncValue> values;

public:
    std::vector<AsyncValue> &getValuesList() noexcept;

    void clearAll();

    void addValue(std::string n, AsyncVariant v);
    void removeValue(std::string n);

    int32_t getInt32Value(std::string n, int32_t defaultValue);
    uint32_t getUint32Value(std::string n, uint32_t defaultValue);
    double getDoubleValue(std::string n, double defaultValue);
    float getFloatValue(std::string n, float defaultValue);
    bool getBoolValue(std::string n, bool defaultValue);
    std::string getStringValue(std::string n, std::string defaultValue);
    std::vector<uint8_t> getBlobValue(std::string n, std::vector<uint8_t> defaultValue);
};

/* ========================================================= */
/* ========================= SAVE ========================== */
/* ========================================================= */

bool saveSimple(
    const std::string &path,
    AsyncSaveData &data);

std::future<bool> saveSimpleAsync(
    const std::string &path,
    AsyncSaveData &data);

/* ========================================================= */
/* ========================= LOAD ========================== */
/* ========================================================= */

bool loadSimple(
    const std::string &path,
    AsyncSaveData &out);

#endif