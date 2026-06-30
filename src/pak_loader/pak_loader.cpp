#if defined(PLATFORM_PC)

#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#endif

#include <string>
#include <cstring>
#include <cstdio> // 👈 Añadido para dar soporte a printf

typedef struct __PAK_FILE
{
    const uint8_t* data;
    size_t size;
    size_t pos;
} __PAK_FILE;

// 1. Estructura que coincide EXACTAMENTE con el empaquetador de 4GB y rutas string
#pragma pack(push, 1)
typedef struct {
    char path[256];   // 256 bytes: Ruta de texto legible
    uint32_t offset;  // 4 bytes: Desplazamiento desde el fin del índice
    uint32_t size;    // 4 bytes: Tamaño del archivo
} PAK_Index;
#pragma pack(pop)

// Estructura principal del sistema PAK
typedef struct {
    FILE* file_handle;           // Puntero al archivo físico .pak
    PAK_Index* index_list;       // Array con todos los índices
    uint32_t total_files;        // Cantidad de archivos en la lista (Cambiado a uint32_t)
    long data_start_offset;      // Posición física del archivo donde TERMINAN los índices
} PAK_Package;

#include <pak_loader/pak_loader.h>
#include <iostream>
#include <filesystem>

static std::string pak;
static PAK_Package *file;

static bool mem = false;
static __PAK_FILE *memFile;

#if defined(_WIN32)
static HANDLE h;
#endif

static size_t PAKL_Read(void* ptr, size_t size, size_t count, FILE* f)
{
    if (mem)
    {
        return PAKL_fread(ptr,
                          size,
                          count,
                          reinterpret_cast<PAK_FILE*>(memFile));
    }

    return fread(ptr,
                 size,
                 count,
                 f);
}

PAK_Package* PAKL_OpenPackage(const char* pak_path)
{
    FILE* f = nullptr;

    if (!mem)
    {
#if defined(_WIN32)
        h = CreateFileA(
            pak_path,
            GENERIC_READ | GENERIC_WRITE,
            0,              // No compartir
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (h == INVALID_HANDLE_VALUE)
            return NULL;

        int fd = _open_osfhandle(reinterpret_cast<intptr_t>(h), O_RDONLY);
        f = _fdopen(fd, "rb");
#elif defined(__linux__) || defined(__APPLE__)
        f = fopen(pak_path, "rb");
#endif

        if (!f)
        {
#if defined(_WIN32)
            CloseHandle(h);
#endif
            printf("No se pudo abrir %s\n", pak_path);
            return nullptr;
        }
#if defined(__linux__) || defined(__APPLE__)
        flock(fileno(f), LOCK_EX);
#endif
    }
    else
    {
        if (!memFile)
            return nullptr;

        memFile->pos = 0;
    }

    char header[10] = {};

    if (PAKL_Read(header, 1, 9, f) != 9)
    {
        if (!mem)
            fclose(f);

        return nullptr;
    }

    if (memcmp(header, "PAKv1.0.0", 9) != 0)
    {
        printf("Cabecera inválida.\n");

        if (!mem)
            fclose(f);

        return nullptr;
    }

    uint32_t indexSize = 0;

    if (PAKL_Read(&indexSize, sizeof(indexSize), 1, f) != 1)
    {
        if (!mem)
            fclose(f);

        return nullptr;
    }

    if (indexSize == 0)
    {
        if (!mem)
            fclose(f);

        return nullptr;
    }

    uint32_t fileCount = indexSize / sizeof(PAK_Index);

    PAK_Package* package =
        (PAK_Package*)malloc(sizeof(PAK_Package));

    if (!package)
    {
        if (!mem)
            fclose(f);

        return nullptr;
    }

    package->index_list =
        (PAK_Index*)malloc(fileCount * sizeof(PAK_Index));

    if (!package->index_list)
    {
        free(package);

        if (!mem)
            fclose(f);

        return nullptr;
    }

    if (PAKL_Read(package->index_list,
                  sizeof(PAK_Index),
                  fileCount,
                  f) != fileCount)
    {
        free(package->index_list);
        free(package);

        if (!mem)
            fclose(f);

        return nullptr;
    }

    package->file_handle = mem ? nullptr : f;
    package->total_files = fileCount;
    package->data_start_offset = 9 + sizeof(uint32_t) + indexSize;

    return package;
}

PAK_Package* PAKL_OpenPackageFromMem(const void* data, size_t size)
{
    if (!data || size == 0)
        return nullptr;

    __PAK_FILE* f = new __PAK_FILE();
    if (!f)
        return nullptr;

    f->data = static_cast<const uint8_t*>(data);
    f->size = size;
    f->pos = 0;

    mem = true;
    memFile = f;

    PAK_Package* package = PAKL_OpenPackage("");

    if (!package)
    {
        delete f;
        memFile = nullptr;
        mem = false;
        return nullptr;
    }

    return package;
}

void PAKL_SetPakFromMem(const void* data, size_t size)
{
    if (!data || size == 0)
        return;

    PAKL_ClosePak();

    file = PAKL_OpenPackageFromMem(data, size);

    if (!file)
    {
        printf("[PAKL] Error: no se pudo abrir el PAK desde memoria.\n");
    }
}

void PAKL_SetPak(const char *path)
{
    if(!path) {
        printf("[PAKL] <- Saliendo de PAKL_SetPak (Error: path nulo)\n");
        return;
    }

    PAKL_ClosePak();
    
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);

    std::string exe_dir = buffer;
    size_t last_slash = exe_dir.find_last_of("\\/");
    if (last_slash != std::string::npos) {
        exe_dir = exe_dir.substr(0, last_slash + 1);
    }

    pak = exe_dir + path;
    
    file = PAKL_OpenPackage(pak.c_str());

    if(!file)
    {
        printf("[PAKL] <- Saliendo de PAKL_SetPak (Error: No se pudo abrir el archivo %s)\n", pak.c_str());
        pak = "";
        return;
    }
}

size_t PAKL_GetFileCount()
{
    if (!file)
        return 0;

    return static_cast<size_t>(file->total_files);
}
const char* PAKL_GetFileName(size_t index)
{
    if (!file)
        return nullptr;

    if (index >= file->total_files)
        return nullptr;

    return file->index_list[index].path;
}

PAK_FILE* PAKL_LoadFile(const char* filename)
{
    if (!filename || !file)
        return nullptr;

    PAK_Index* entry = nullptr;

    for (uint32_t i = 0; i < file->total_files; i++)
    {
        if (strcmp(file->index_list[i].path, filename) == 0)
        {
            entry = &file->index_list[i];
            break;
        }
    }

    if (!entry)
    {
        printf("[PAKL] Archivo no encontrado: %s\n", filename);
        return nullptr;
    }

    long realOffset = file->data_start_offset + static_cast<long>(entry->offset);

    if (!mem)
    {
        if (fseek(file->file_handle, realOffset, SEEK_SET) != 0)
        {
            printf("[PAKL] Error realizando fseek.\n");
            return nullptr;
        }
    }
    else
    {
        if (PAKL_fseek(memFile, realOffset, SEEK_SET) != 0)
        {
            printf("[PAKL] Error realizando fseek en memoria.\n");
            return nullptr;
        }
    }

    uint8_t* buffer = (uint8_t*)malloc(entry->size);

    if (!buffer)
    {
        printf("[PAKL] Sin memoria.\n");
        return nullptr;
    }

    size_t bytesRead;

    if (!mem)
    {
        bytesRead = fread(buffer, 1, entry->size, file->file_handle);
    }
    else
    {
        bytesRead = PAKL_fread(buffer, 1, entry->size, memFile);
    }

    if (bytesRead != entry->size)
    {
        printf("[PAKL] Error leyendo '%s'.\n", filename);
        free(buffer);
        return nullptr;
    }

    PAK_FILE* pakFile = new PAK_FILE();

    pakFile->data = buffer;
    pakFile->size = entry->size;
    pakFile->pos = 0;

    return pakFile;
}

size_t PAKL_GetFileSize(PAK_FILE* file)
{
    if (!file)
        return 0;

    return file->size;
}

long PAKL_ftell(PAK_FILE* f)
{
    if (!f)
        return -1L;

    return static_cast<long>(f->pos);
}

void PAKL_rewind(PAK_FILE* f)
{
    if (!f)
        return;

    f->pos = 0;
}

size_t PAKL_fread(void* buffer, size_t size, size_t count, PAK_FILE* f)
{
    if (!f || !buffer || size == 0 || count == 0)
        return 0;

    size_t bytesRequested = size * count;

    if (f->pos >= f->size)
        return 0;

    size_t bytesAvailable = f->size - f->pos;

    if (bytesRequested > bytesAvailable)
        bytesRequested = bytesAvailable;

    memcpy(buffer,
           f->data + f->pos,
           bytesRequested);

    f->pos += bytesRequested;

    return bytesRequested / size;
}

int PAKL_fseek(PAK_FILE* f, long offset, int origin)
{
    if (!f)
        return -1;

    long newPos;

    switch (origin)
    {
        case SEEK_SET:
            newPos = offset;
            break;

        case SEEK_CUR:
            newPos = static_cast<long>(f->pos) + offset;
            break;

        case SEEK_END:
            newPos = static_cast<long>(f->size) + offset;
            break;

        default:
            return -1;
    }

    if (newPos < 0)
        return -1;

    if (static_cast<size_t>(newPos) > f->size)
        return -1;

    f->pos = static_cast<size_t>(newPos);

    return 0;
}

void PAKL_CloseFile(PAK_FILE* f)
{
    if (!f)
        return;

    if (f->data)
        free((void*)f->data);

    delete f;
}

void PAKL_ClosePackage(PAK_Package* package)
{
    if (!package)
        return;

    if (package->file_handle)
    {
#if defined(_WIN32)
        if(!mem)
            CloseHandle(h);
#endif
        fclose(package->file_handle);
        package->file_handle = nullptr;
    }

    if (package->index_list)
    {
        free(package->index_list);
        package->index_list = nullptr;
    }

    free(package);
}

void PAKL_ClosePak()
{
    if (file)
    {
        PAKL_ClosePackage(file);
        file = nullptr;
    }

    if (memFile)
    {
        delete memFile;
        memFile = nullptr;
    }

    mem = false;
    pak.clear();
}

#endif
