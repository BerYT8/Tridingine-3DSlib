#pragma once

typedef struct HTML_PAGE HTML_PAGE;

HTML_PAGE *HTML_LoadPage(const char *path);

void DrawHtmlPage(HTML_PAGE *page, 
            float x, float y, float depth, 
            float tw, float th, 
            float bw, float bh, 
            float topAlignX, float topAlignY, 
            float botAlignX, float botAlignY);

void HTML_ClosePage(HTML_PAGE *page);