#pragma once

typedef struct HTML_PAGE HTML_PAGE;

HTML_PAGE *HTML_LoadPage(const char *path);

void DrawHtmlPage(HTML_PAGE *page, 
            float x, float y, float depth, 
            float w, float h, 
            float alignX, float alignY);

void HTML_ClosePage(HTML_PAGE *page);