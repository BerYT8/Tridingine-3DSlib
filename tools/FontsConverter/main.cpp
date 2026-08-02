/*------------------------------------------------------------------------------
 * Copyright (c) 2019
 *     Michael Theall (mtheall)
 *     piepie62
 *
 * This file is part of tex3ds.
 *
 * tex3ds is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tex3ds is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tex3ds.  If not, see <http://www.gnu.org/licenses/>.
 *----------------------------------------------------------------------------*/
/** @file main.cpp
 *  @brief main program entry point
 */
#include "bcfnt.h"
#include "freetype.h"
#include "future.h"

#include <getopt.h>

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

bool convert3DS(const fs::path& input, const fs::path& output)
{
    auto library = freetype::Library::makeLibrary();
    if (!library)
        return false;

    auto face = freetype::Face::makeFace(library, input.string(), 24.0);
    if (!face)
        return false;

    bcfnt::BCFNT font;
    std::vector<uint16_t> list;

    font.addFont(std::move(face), list, true);

    return font.serialize(output.string());
}

bool convertPC(const fs::path& input, const fs::path& output)
{
    std::error_code ec;
    fs::copy_file(input, output,
                  fs::copy_options::overwrite_existing,
                  ec);

    if(ec)
    {
        std::cerr << ec.message() << '\n';
        return false;
    }

    return true;
}

int main(int argc,char** argv)
{
    std::string input;
    std::string output;

    bool mode3ds=false;
    bool modePc=false;
    bool recursive=false;

    for(int i=1;i<argc;i++)
    {
        std::string arg=argv[i];

        if(arg=="-i" && i+1<argc)
            input=argv[++i];

        else if(arg=="-o" && i+1<argc)
            output=argv[++i];

        else if(arg=="-3ds")
            mode3ds=true;

        else if(arg=="-pc")
            modePc=true;

        else if(arg=="--all")
            recursive=true;
    }

    if(input.empty() || output.empty() || (!mode3ds && !modePc) || (mode3ds && modePc))
    {
        std::cout
            << "Usage:\n"
            << "FontsConverter [-3ds|-pc] -i input -o output [--all]\n";
        return 1;
    }

    fs::path in(input);
    fs::path out(output);

    if(recursive)
    {
        if(!fs::is_directory(in))
        {
            std::cerr<<"Input debe ser un directorio\n";
            return 1;
        }

        fs::create_directories(out);

        for(auto& file:fs::recursive_directory_iterator(in))
        {
            if(!file.is_regular_file())
                continue;

            if(file.path().extension()!=".ttf")
                continue;

            auto relative=fs::relative(file.path(),in);

            auto dstDir=out/relative.parent_path();

            fs::create_directories(dstDir);

            auto dst=dstDir/file.path().stem();

            if(mode3ds)
            {
                dst.replace_extension(".bcfnt");

                std::cout
                    << file.path()
                    << " -> "
                    << dst
                    << '\n';

                convert3DS(file.path(),dst);
            }
            else
            {
                dst.replace_extension(".ttf");

                std::cout
                    << file.path()
                    << " -> "
                    << dst
                    << '\n';

                convertPC(file.path(),dst);
            }
        }
    }
    else
    {
        fs::path dst=out;

        if(fs::is_directory(out))
        {
            dst/=in.stem();

            if(mode3ds)
                dst.replace_extension(".bcfnt");
            else
                dst.replace_extension(".ttf");
        }

        if(mode3ds)
            return convert3DS(in,dst)?0:1;
        else
            return convertPC(in,dst)?0:1;
    }

    return 0;
}