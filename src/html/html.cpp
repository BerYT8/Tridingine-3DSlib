#include "html.h"

#include <pak_loader/pak_loader.h>

#include "../romfs_path.h"

HtmlContent parseHtml(std::string path)
{
    HtmlContent content;
    content.title = "";
    content.objects = {};

    std::string fp = getRomfsPath(path.c_str());

    PAK_FILE *f = PAKL_LoadFile(fp.c_str());

    if(!f)
        return content;
    // Parse HTML
    
    PAKL_CloseFile(f);

    return content;
}

html::html(std::string path)
{
    pathPage = path;
    content = parseHtml(path);
}

html::~html()
{

}
