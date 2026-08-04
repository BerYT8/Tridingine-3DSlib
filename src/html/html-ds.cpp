#include <html/html-ds.h>
#include "html.h"

typedef struct HTML_PAGE
{
    html *html;
} HTML_PAGE;

HTML_PAGE *HTML_LoadPage(const char *path)
{
    HTML_PAGE *page = new HTML_PAGE();

    if(!page)
        return nullptr;

    page->html = new html(path);
    if(!page->html)
    {
        delete page;
        return nullptr;
    }

    return page;
}

void HTML_ClosePage(HTML_PAGE *page)
{
    if(!page)
        return;

    if(page->html)
        delete page->html;
    
    delete page;
}