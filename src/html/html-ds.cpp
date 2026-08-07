#include <html/html-ds.h>
#include "html.h"

#include <console/console.h>

#include <draw/2d/2d_shapes.h>
#include "../screens/screensValues.h"

typedef struct HTML_PAGE
{
    bool correctLoaded;
    html *htmlcode;
} HTML_PAGE;

#define TITLE_BAR_HEIGHT 20
#define TITLE_BAR_WIDTH 60
#define TITLE_BAR_EXTRA_X 2

std::string GetTitle(std::vector<HtmlNode> objects)
{
    std::string title = "";
    for (auto &item : objects)
    {
        if (std::holds_alternative<std::string>(item))
        {
            const std::string& text = std::get<std::string>(item);
            title += text;
        }
    }
    if(title.length() > 10)
    {
        title.erase(8, std::string::npos);
        title += "...";
    }
    return title;
}

void DrawNodes(std::vector<HtmlNode> objects, float x, float y, float depth, float w, float h)
{
    for (auto &item : objects)
    {
        if (std::holds_alternative<std::string>(item))
        {
            const std::string& text = std::get<std::string>(item);

            // Es texto
        }
        else if (std::holds_alternative<HtmlContentObject>(item))
        {
            const HtmlContentObject& child = std::get<HtmlContentObject>(item);

            switch (child.tag)
            {
            case HtmlTags::title:
                {
                    Vec2 sSize = S2S_GetScreenSize(currScreen);
                    std::string title = GetTitle(child.objects);
                    D2D_DrawText(title.c_str(), System_GetDefaultFont(), -1, Color_Black, x + TITLE_BAR_EXTRA_X*1.5, y, depth, TITLE_BAR_WIDTH * (w > 1.f ? w : 1.f)/(sSize.x > 1.f ? sSize.x : 1.f), TITLE_BAR_HEIGHT, 0, 0, 0, 0, 0, 0, WRAP_NONE);
                    break;
                }
            default:
                DrawNodes(child.objects, x, y, depth, w, h);
                break;
            }
        }
    }
}

HTML_PAGE *HTML_LoadPage(const char *path)
{
    HTML_PAGE *page = new HTML_PAGE();

    if(!page)
        return nullptr;

    page->htmlcode = new html(path);
    if(!page->htmlcode)
    {
        delete page;
        return nullptr;
    }
    page->correctLoaded = true;
    if(!page->htmlcode->loaded())
        page->correctLoaded = false;

    return page;
}

void DrawHtmlPage(HTML_PAGE *page, 
            float x, float y, float depth, 
            float w, float h, 
            float alignX, float alignY)
{
    if(!page)
        return;
    if(!page->htmlcode || !page->correctLoaded)
        return;

    D2D_DrawRectangle(x + TITLE_BAR_EXTRA_X, y, TITLE_BAR_WIDTH, TITLE_BAR_HEIGHT, 0, depth, alignX, alignY, Color_White, Color_White, Color_White, Color_White);
    D2D_DrawRectangle(x, y + TITLE_BAR_HEIGHT, w, h - TITLE_BAR_HEIGHT, 0, depth, alignX, alignY, Color_White, Color_White, Color_White, Color_White);

    auto &content = page->htmlcode->GetContent();

    DrawNodes(content.objects, x, y, depth, w, h);
}

void HTML_ClosePage(HTML_PAGE *page)
{
    if(!page)
        return;

    if(page->htmlcode)
        delete page->htmlcode;
    
    delete page;
}