#include <utils/save_system/asyncFuncs.h>

#if defined(PLATFORM_3DS)
#include <3ds.h>
#endif

extern "C"
{
#include <utils/save_system/loadSys.h>
#include <utils/save_system/saveSys.h>
}

#include <future>
#include <cstring>
#include <variant>
#include <algorithm>

/* ====================== CLASS UTIL =======================*/

AsyncValue &getValueWithName(
    const std::string &name,
    std::vector<AsyncValue> &list)
{
    for (auto &v : list)
    {
        if (v.name == name)
            return v;
    }

    throw std::runtime_error("Not found");
}

std::vector<AsyncValue> &AsyncSaveData::getValuesList() noexcept
{
    return values;
}

void AsyncSaveData::clearAll()
{
    values.clear();
}

void AsyncSaveData::addValue(std::string n, AsyncVariant v)
{
    try
    {
        getValueWithName(n, values).value = v;
    }
    catch (...)
    {
        values.push_back({n, v});
    }
}
void AsyncSaveData::removeValue(std::string n)
{
    try
    {
        getValueWithName(n, values).value = nullValue();
    }
    catch (...)
    {
        values.push_back({n, nullValue()});
    }
}

template <typename T>
T getValue(const std::string &name,
           const std::vector<AsyncValue> &values,
           T defaultValue)
{
    auto it = std::find_if(
        values.begin(),
        values.end(),
        [&](const AsyncValue &v)
        {
            return v.name == name;
        });

    if (it == values.end())
        return defaultValue;

    if (it->value.template is<T>())
        return it->value.template as<T>();

    return defaultValue;
}
int32_t AsyncSaveData::getInt32Value(std::string n, int32_t defaultValue)
{
    return getValue<int32_t>(n, values, defaultValue);
}
uint32_t AsyncSaveData::getUint32Value(std::string n, uint32_t defaultValue)
{
    return getValue<uint32_t>(n, values, defaultValue);
}
double AsyncSaveData::getDoubleValue(std::string n, double defaultValue)
{
    return getValue<double>(n, values, defaultValue);
}
float AsyncSaveData::getFloatValue(std::string n, float defaultValue)
{
    return getValue<float>(n, values, defaultValue);
}
bool AsyncSaveData::getBoolValue(std::string n, bool defaultValue)
{
    return getValue<bool>(n, values, defaultValue);
}
std::string AsyncSaveData::getStringValue(std::string n, std::string defaultValue)
{
    return getValue<std::string>(n, values, defaultValue);
}
std::vector<uint8_t> AsyncSaveData::getBlobValue(std::string n, std::vector<uint8_t> defaultValue)
{
    return getValue<std::vector<uint8_t>>(n, values, defaultValue);
}

/* ========================================================= */
/* ===================== TYPE HELPERS ====================== */
/* ========================================================= */

static void addVariantToSave(
    SaveContext *ctx,
    const AsyncValue &value)
{
    const AsyncVariant &v = value.value;

    /* ================= NULL ================= */

    if (v.isNull())
    {
        save_remove_value(
            ctx,
            value.name.c_str());

        return;
    }

    /* ================= BOOL ================= */

    if (v.isBool())
    {
        save_add_bool(
            ctx,
            value.name.c_str(),
            v.as<bool>());

        return;
    }

    /* ================= INT32 ================= */

    if (v.isInt())
    {
        save_add_int32(
            ctx,
            value.name.c_str(),
            v.as<int32_t>());

        return;
    }

    /* ================= UINT32 ================= */

    if (v.isUInt())
    {
        save_add_uint32(
            ctx,
            value.name.c_str(),
            v.as<uint32_t>());

        return;
    }

    /* ================= FLOAT ================= */

    if (v.isFloat())
    {
        save_add_float(
            ctx,
            value.name.c_str(),
            v.as<float>());

        return;
    }

    /* ================= DOUBLE ================= */

    if (v.isDouble())
    {
        save_add_blob(
            ctx,
            value.name.c_str(),
            &v.as<double>(),
            sizeof(double));

        return;
    }

    /* ================= STRING ================= */

    if (v.isString())
    {
        save_add_string(
            ctx,
            value.name.c_str(),
            v.as<std::string>().c_str());

        return;
    }

    /* ================= BLOB ================= */

    if (v.isBytes())
    {
        save_add_blob(
            ctx,
            value.name.c_str(),
            v.as<std::vector<uint8_t>>().data(),
            (uint32_t)v.as<std::vector<uint8_t>>().size());

        return;
    }
}

/* ========================================================= */
/* ======================== SAVE =========================== */
/* ========================================================= */

bool saveSimple(
    const std::string &path,
    AsyncSaveData &data)
{
    SaveContext ctx;

    save_init(
        &ctx,
        false,
        1,
        0,
        0);

    for (auto &value : data.getValuesList())
    {
        addVariantToSave(
            &ctx,
            value);
    }

    bool ok =
        save_write_file(
            &ctx,
            path.c_str());

    save_free(&ctx);

    return ok;
}

#if defined(PLATFORM_3DS)

struct ConectedAsync
{
    std::string path;
    AsyncSaveData data;
};

struct SaveThreadData
{
    ConectedAsync* payload;
    std::shared_ptr<std::promise<bool>> promise;
    Thread t;
};

void saveThreadWrapper(void* arg)
{
    std::unique_ptr<SaveThreadData> data(
        static_cast<SaveThreadData*>(arg)
    );

    bool result = saveSimple(
        data->payload->path,
        data->payload->data
    );

    data->promise->set_value(result);

    delete data->payload; // liberamos el payload aquí

    threadFree(data->t);
}

#endif

std::future<bool> saveSimpleAsync(
    const std::string &path,
    AsyncSaveData &data)
{
#if defined(PLATFORM_PC)

    return std::async(
        std::launch::async,
        [path, &data]()
        {
            return saveSimple(path, data);
        });

#elif defined(PLATFORM_3DS)

    ConectedAsync* a = new ConectedAsync();
    a->path = path;
    a->data = data;

    auto* taskData = new SaveThreadData();

    auto promise = std::make_shared<std::promise<bool>>();
    std::future<bool> fut = promise->get_future();

    taskData->payload = a;
    taskData->promise = promise;

    Thread t = threadCreate(
        saveThreadWrapper,
        taskData,
        4096,
        0x18,
        -2,
        true
    );

    if (!t)
    {
        promise->set_value(false);
        
        delete a;
        delete taskData;

        return fut; // cuidado: este future no se completará nunca
    }

    taskData->t = t;
    
    return fut; // cuidado: este future no se completará nunca
#endif
}

/* ========================================================= */
/* ========================= LOAD ========================== */
/* ========================================================= */

bool loadSimple(
    const std::string &path,
    AsyncSaveData &out)
{
    SaveFileData file;

    if (!load_file(path.c_str(), &file))
        return false;

    out.clearAll();

    for (uint32_t i = 0; i < file.entryCount; i++)
    {
        SaveEntry *e = &file.entries[i];

        AsyncValue value;

        value.name = e->name;

        uint32_t byteOffset =
            e->relativeBitOffset / 8;

        uint8_t *ptr =
            file.data + byteOffset;

        switch (e->type)
        {
        case SAVE_TYPE_BOOL:
        {
            bool b =
                load_get_bool(
                    &file,
                    e->name,
                    false);

            value.value = b;

            break;
        }

        case SAVE_TYPE_INT32:
        {
            int32_t v;

            memcpy(&v, ptr, sizeof(int32_t));

            value.value = v;

            break;
        }

        case SAVE_TYPE_UINT32:
        {
            uint32_t v;

            memcpy(&v, ptr, sizeof(uint32_t));

            value.value = v;

            break;
        }

        case SAVE_TYPE_FLOAT:
        {
            float v;

            memcpy(&v, ptr, sizeof(float));

            value.value = v;

            break;
        }

        case SAVE_TYPE_DOUBLE:
        {
            double v;

            memcpy(&v, ptr, sizeof(double));

            value.value = v;

            break;
        }

        case SAVE_TYPE_STRING:
        {
            value.value =
                std::string(
                    reinterpret_cast<char *>(ptr),
                    e->dataSizeBits / 8);

            break;
        }

        case SAVE_TYPE_BLOB:
        {
            uint32_t size =
                e->dataSizeBits / 8;

            std::vector<uint8_t> blob(size);

            memcpy(
                blob.data(),
                ptr,
                size);

            value.value = blob;

            break;
        }

        default:
            break;
        }

        out.addValue(value.name, value.value);
    }

    load_free(&file);

    return true;
}