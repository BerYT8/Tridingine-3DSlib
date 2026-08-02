cmake_minimum_required(VERSION 3.10)

project(game C CXX)

if(NOT DEFINED GAME_NAME)
    # Asigna un valor por defecto si no se introduce en la terminal
    set(GAME_NAME ${PROJECT_NAME} CACHE STRING "Nombre del proyecto 3DS")
endif()

if(NOT DEFINED NAME)
    if(DEFINED PROJECT_NAME)
        set(NAME ${PROJECT_NAME} CACHE STRING "Nombre del proyecto 3DS")
    else()
        set(NAME "MiJuego3DS" CACHE STRING "Nombre del proyecto 3DS")
    endif()
endif()

if(NOT DEFINED DESCRIPTION)
    # Asigna un valor por defecto si no se introduce en la terminal
    set(DESCRIPTION "MiJuego3DS" CACHE STRING "Descripción del proyecto 3DS")
endif()

if(NOT DEFINED AUTHOR)
    # Asigna un valor por defecto si no se introduce en la terminal
    set(AUTHOR "Yo" CACHE STRING "Autor del proyecto 3DS")
endif()

if(NOT DEFINED CODE)
    # Asigna un valor por defecto si no se introduce en la terminal
    set(CODE "code.elf" CACHE STRING "Código del proyecto 3DS")
endif()

# ---------------- C++ ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mi_target_externo IMPORTED)
set_property(TARGET mi_target_externo PROPERTY IMPORTED_LOCATION ${CODE})
    message(STATUS "ROMFS: ${CMAKE_CURRENT_SOURCE_DIR}/../romfs/3ds")

ctr_generate_smdh(
    OUTPUT "${GAME_NAME}.smdh"
    NAME ${NAME}
    DESCRIPTION ${DESCRIPTION}
    AUTHOR ${AUTHOR}
)

set(ROMFS_DIR "${CMAKE_SOURCE_DIR}/../romfs/3ds")

ctr_create_3dsx(${GAME_NAME}
    TARGET mi_target_externo
    SMDH "${GAME_NAME}.smdh"
    ROMFS "${ROMFS_DIR}"
)

# --- GENERACIÓN AUTOMÁTICA DE RSF ---
# Definir la ruta del archivo RSF si no está definida
set(RSF_PATH "${CMAKE_SOURCE_DIR}/App.rsf")

# VALIDACIÓN: Verificar si el archivo NO existe
if(NOT EXISTS "${RSF_PATH}")
    message(STATUS "Archivo App.rsf no encontrado. Generando uno nuevo de forma automática...")
    
    # 1. Generar un número aleatorio basado en el tiempo
    string(TIMESTAMP CURRENT_SEC "%S") 
    math(EXPR RANDOM_NUM "983040 + (${CURRENT_SEC} * 1092) % 65535")
    math(EXPR RANDOM_HEX "${RANDOM_NUM}" OUTPUT_FORMAT HEXADECIMAL)

    # 2. Crear la estructura del archivo RSF de forma dinámica
    file(WRITE "${RSF_PATH}"
"# Archivo RSF Generado Automáticamente
BasicInfo:
  Title                   : ${NAME}
  ProductCode             : CTR-P-CTAP
  Logo                    : Nintendo # Nintendo / Licensed / Distributed / iQue / iQueForSystem

RomFs:
  # Specifies the root path of the read only file system to include in the ROM.
  #RootPath                : romfs

TitleInfo:
  Category                : Application
  UniqueId                : ${RANDOM_HEX}

Option:
  UseOnSD                 : true # true if App is to be installed to SD
  FreeProductCode         : true # Removes limitations on ProductCode
  MediaFootPadding        : false # If true CCI files are created with padding
  EnableCrypt             : false # Enables encryption for NCCH and CIA
  EnableCompress          : true # Compresses where applicable (currently only exefs:/.code)
  
AccessControlInfo:
  CoreVersion                   : 2

  # Exheader Format Version
  DescVersion                   : 2
  
  # Minimum Required Kernel Version (below is for 4.5.0)
  ReleaseKernelMajor            : "02"
  ReleaseKernelMinor            : "33" 

  # ExtData
  UseExtSaveData                : false # enables ExtData       
  #ExtSaveDataId                : 0x300 # only set this when the ID is different to the UniqueId

  # FS:USER Archive Access Permissions
  # Uncomment as required
  FileSystemAccess:
   #- CategorySystemApplication
   #- CategoryHardwareCheck
   #- CategoryFileSystemTool
   #- Debug
   #- TwlCardBackup
   #- TwlNandData
   #- Boss
   - DirectSdmc
   #- Core
   #- CtrNandRo
   #- CtrNandRw
   #- CtrNandRoWrite
   #- CategorySystemSettings
   #- CardBoard
   #- ExportImportIvs
   #- DirectSdmcWrite
   #- SwitchCleanup
   #- SaveDataMove
   #- Shop
   #- Shell
   #- CategoryHomeMenu

  # Process Settings
  MemoryType                    : Application # Application/System/Base
  SystemMode                    : 64MB # 64MB(Default)/96MB/80MB/72MB/32MB
  IdealProcessor                : 0
  AffinityMask                  : 1
  Priority                      : 16
  MaxCpu                        : 0 # Let system decide
  HandleTableSize               : 0x200
  DisableDebug                  : false
  EnableForceDebug              : false
  CanWriteSharedPage            : true
  CanUsePrivilegedPriority      : false
  CanUseNonAlphabetAndNumber    : true
  PermitMainFunctionArgument    : true
  CanShareDeviceMemory          : true
  RunnableOnSleep               : false
  SpecialMemoryArrange          : true

  # New3DS Exclusive Process Settings
  SystemModeExt                 : 124MB # Legacy(Default)/124MB/178MB  Legacy:Use Old3DS SystemMode
  CpuSpeed                      : 804MHz # 268MHz(Default)/804MHz
  EnableL2Cache                 : true # false(default)/true
  CanAccessCore2                : true 

  # Virtual Address Mappings
  IORegisterMapping:
   - 1ff00000-1ff7ffff   # DSP memory
  MemoryMapping: 
   - 1f000000-1f5fffff:r # VRAM

  # Accessible SVCs, <Name>:<ID>
  SystemCallAccess: 
    ArbitrateAddress: 34
    Break: 60
    CancelTimer: 28
    ClearEvent: 25
    ClearTimer: 29
    CloseHandle: 35
    ConnectToPort: 45
    ControlMemory: 1
    CreateAddressArbiter: 33
    CreateEvent: 23
    CreateMemoryBlock: 30
    CreateMutex: 19
    CreateSemaphore: 21
    CreateThread: 8
    CreateTimer: 26
    DuplicateHandle: 39
    ExitProcess: 3
    ExitThread: 9
    GetCurrentProcessorNumber: 17
    GetHandleInfo: 41
    GetProcessId: 53
    GetProcessIdOfThread: 54
    GetProcessIdealProcessor: 6
    GetProcessInfo: 43
    GetResourceLimit: 56
    GetResourceLimitCurrentValues: 58
    GetResourceLimitLimitValues: 57
    GetSystemInfo: 42
    GetSystemTick: 40
    GetThreadContext: 59
    GetThreadId: 55
    GetThreadIdealProcessor: 15
    GetThreadInfo: 44
    GetThreadPriority: 11
    MapMemoryBlock: 31
    OutputDebugString: 61
    QueryMemory: 2
    ReleaseMutex: 20
    ReleaseSemaphore: 22
    SendSyncRequest1: 46
    SendSyncRequest2: 47
    SendSyncRequest3: 48
    SendSyncRequest4: 49
    SendSyncRequest: 50
    SetThreadPriority: 12
    SetTimer: 27
    SignalEvent: 24
    SleepThread: 10
    UnmapMemoryBlock: 32
    WaitSynchronization1: 36
    WaitSynchronizationN: 37
    Backdoor: 123

  # Service List
  # Maximum 34 services (32 if firmware is prior to 9.3.0)
  ServiceAccessControl:
   - cfg:u
   - fs:USER
   - gsp::Gpu
   - hid:USER
   - ndm:u
   - pxi:dev
   - APT:U
   - ac:u
   - act:u
   - am:net
   - boss:U
   - cam:u
   - cecd:u
   - csnd:SND
   - frd:u
   - http:C
   - ir:USER
   - ir:u
   - ir:rst
   - ldr:ro
   - mic:u
   - news:u
   - nfc:u
   - nim:aoc
   - nwm::UDS
   - ptm:u
   - qtm:u
   - soc:U
   - ssl:C
   - y2r:u


SystemControlInfo:
  SaveDataSize: 0K
  RemasterVersion: 0
  StackSize: 0x40000

  # Modules that run services listed above should be included below
  # Maximum 48 dependencies
  # If a module is listed that isn't present on the 3DS, the title will get stuck at the logo (3ds waves)
  # So act, nfc and qtm are commented for 4.x support. Uncomment if you need these.
  # <module name>:<module titleid>
  Dependency: 
    ac: 0x0004013000002402
    #act: 0x0004013000003802
    am: 0x0004013000001502
    boss: 0x0004013000003402
    camera: 0x0004013000001602
    cecd: 0x0004013000002602
    cfg: 0x0004013000001702
    codec: 0x0004013000001802
    csnd: 0x0004013000002702
    dlp: 0x0004013000002802
    dsp: 0x0004013000001a02
    friends: 0x0004013000003202
    gpio: 0x0004013000001b02
    gsp: 0x0004013000001c02
    hid: 0x0004013000001d02
    http: 0x0004013000002902
    i2c: 0x0004013000001e02
    ir: 0x0004013000003302
    mcu: 0x0004013000001f02
    mic: 0x0004013000002002
    ndm: 0x0004013000002b02
    news: 0x0004013000003502
    #nfc: 0x0004013000004002
    nim: 0x0004013000002c02
    nwm: 0x0004013000002d02
    pdn: 0x0004013000002102
    ps: 0x0004013000003102
    ptm: 0x0004013000002202
    #qtm: 0x0004013020004202
    ro: 0x0004013000003702
    socket: 0x0004013000002e02
    spi: 0x0004013000002302
    ssl: 0x0004013000002f02
")
else()
    message(STATUS "Archivo App.rsf encontrado en: ${RSF_PATH}")
endif()


# ------------------------------------------------------------------
# Herramientas
# ------------------------------------------------------------------

find_program(THREEDSTOOL_EXE
    3dstool
    PATHS "${CMAKE_SOURCE_DIR}/../tools"
    REQUIRED
)

find_program(MAKEROM_EXE
    makerom
    PATHS "${CMAKE_SOURCE_DIR}/../tools"
    REQUIRED
)

# ------------------------------------------------------------------
# Archivos
# ------------------------------------------------------------------

set(TARGET_NAME ${GAME_NAME})

set(BINARY_ROMFS_FILE
    "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.romfs"
)

# ------------------------------------------------------------------
# Generar RomFS IVFC y crear CIA
# ------------------------------------------------------------------

add_custom_command(
    TARGET ${TARGET_NAME}
    POST_BUILD

    # Genera un RomFS IVFC a partir del directorio romfs
    COMMAND
        ${THREEDSTOOL_EXE}
        -cvtf romfs
        "${BINARY_ROMFS_FILE}"
        --romfs-dir "${ROMFS_DIR}"

    # Genera el CIA
    COMMAND
        ${MAKEROM_EXE}
        -f cia
        -v
        -target t
        -desc app:2
        -o "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.cia"
        -rsf "${RSF_PATH}"
        -elf "${CODE}"
        -romfs "${BINARY_ROMFS_FILE}"

    COMMENT "Generando RomFS IVFC y creando CIA..."
)