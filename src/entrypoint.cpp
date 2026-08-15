#if defined(PLATFORM_PC)

#if defined(_WIN32)
#include <windows.h>
#endif

extern int app_main() __attribute__((weak));
extern int app_main(int argc, char* argv[]) __attribute__((weak));

using AppMainNoArgs = int (*)();
using AppMainArgs   = int (*)(int, char**);

static int call_app_main(int argc, char** argv)
{
    AppMainArgs with_args = static_cast<AppMainArgs>(app_main);
    AppMainNoArgs no_args = static_cast<AppMainNoArgs>(app_main);

    if (with_args)
        return with_args(argc, argv);

    if (no_args)
        return no_args();

    return -1;
}

#if defined(_WIN32)

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return call_app_main(__argc, __argv);
}

#else

int main(int argc, char* argv[])
{
    return call_app_main(argc, argv);
}

#endif

#elif defined(PLATFORM_3DS)

extern int app_main() __attribute__((weak));
extern int app_main(int argc, char* argv[]) __attribute__((weak));

using AppMainNoArgs = int (*)();
using AppMainArgs   = int (*)(int, char**);

static int call_app_main(int argc, char** argv)
{
    AppMainArgs with_args = static_cast<AppMainArgs>(app_main);
    AppMainNoArgs no_args = static_cast<AppMainNoArgs>(app_main);

    if (with_args)
        return with_args(argc, argv);

    if (no_args)
        return no_args();

    return -1;
}

int main(int argc, char* argv[])
{
    return call_app_main(argc, argv);
}

#endif