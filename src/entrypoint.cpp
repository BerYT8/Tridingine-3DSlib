#include <Tridingine.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

extern int app_main(int argc, char* argv[]);

static void showHelp()
{
    std::cout << "Usage: tridingine [options]" << std::endl;
    std::cout << "Options:\n";
    std::cout << "  -h, --help                    Show this help message." << std::endl;
    std::cout << "  -v, --version                 Show version information." << std::endl;
    std::cout << "  -ns, --no-start               Do not start the application." << std::endl;
    std::cout << std::endl;
}

static int call_app_main(int argc, char** argv)
{
    bool stopCode = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            showHelp();
        }
        else if (arg == "-v" || arg == "--version") {
            bool ma = false;
            bool mi = false;
            bool mic = false;
            bool pushed = false;

            int iv = i;

            for (iv = i + 1; iv < argc; iv++) {
                std::string argver = argv[iv];

                std::transform(
                    argver.begin(),
                    argver.end(),
                    argver.begin(),
                    [](unsigned char c) {
                        return static_cast<char>(std::tolower(c));
                    }
                );

                if (argver == "major")
                    ma = true;
                else if (argver == "minor")
                    mi = true;
                else if (argver == "micro")
                    mic = true;
                else
                    break;
            }

            if (ma) {
                std::cout << "v" << TRIDINGINE_VERSION_MAJOR;
                pushed = true;
            }

            if (mi) {
                if (!pushed)
                    std::cout << "vX";

                std::cout << "." << TRIDINGINE_VERSION_MINOR;
                pushed = true;
            }
            else if (!mi && pushed) {
                std::cout << ".X";
            }

            if (mic) {
                if (!pushed)
                    std::cout << "vX.X";

                std::cout << "." << TRIDINGINE_VERSION_MICRO;
            }
            else if (!mic && pushed) {
                std::cout << ".X";
            }

            if (!ma && !mi && !mic) {
                std::cout
                    << "Tridingine by BerYT8 v"
                    << TRIDINGINE_VERSION_MAJOR
                    << "."
                    << TRIDINGINE_VERSION_MINOR
                    << "."
                    << TRIDINGINE_VERSION_MICRO;
            }

            std::cout << std::endl;
            i = iv - 1;
        }
        else if (arg == "-ns" || arg == "--no-start") {
            stopCode = true;
        }
    }

    if (stopCode)
        return 0;

    return app_main(argc, argv);
}

#if defined(PLATFORM_PC)

#if defined(_WIN32)

#include <windows.h>

#endif

int main(int argc, char* argv[])
{
#if defined(PLATFORM_PC) && defined(_WIN32)
    HWND console = GetConsoleWindow();

    if (console)
        ShowWindow(console, SW_HIDE);
#endif
    return call_app_main(argc, argv);
}

#elif defined(PLATFORM_3DS)

int main(int argc, char* argv[])
{
    return call_app_main(argc, argv);
}

#endif