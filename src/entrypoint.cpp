extern int app_main(int argc, char* argv[]);
#if defined(PLATFORM_PC) && defined(_WIN32)
#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return app_main(__argc, __argv);
}
#elif defined(PLATFORM_3DS)
int main(int argc, char* argv[])
{
    return app_main(argc, argv);
}
#endif