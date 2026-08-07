#include "html.h"

#include <pak_loader/pak_loader.h>

#include "../romfs_path.h"

HtmlTags parseTag(const std::string& tag)
{
    for (size_t i = 0; i < std::size(HtmlTagsName); ++i)
    {
        if (tag == HtmlTagsName[i])
            return static_cast<HtmlTags>(i);
    }

    return HtmlTags::unknown;
}

std::string readTagName(PAK_FILE* f)
{
    std::string tag;
    int c;

    while ((c = PAKL_fgetc(f)) != EOF)
    {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
            c == '>' || c == '/')
        {
            PAKL_ungetc(c, f);
            break;
        }

        tag += (char)c;
    }

    return tag;
}

bool readAttributes(PAK_FILE* f, HtmlContentObject&)
{
    int c;
    bool inString = false;

    while ((c = PAKL_fgetc(f)) != EOF)
    {
        if (c == '"')
        {
            inString = !inString;
            continue;
        }

        if (inString)
            continue;

        if (c == '>')
            return false;   // etiqueta normal

        if (c == '/')
        {
            int next = PAKL_fgetc(f);

            if (next == '>')
                return true;    // etiqueta autocerrada

            if (next != EOF)
                PAKL_ungetc(next, f);
        }

        // TODO: leer atributos
    }

    return false;
}

std::string readText(PAK_FILE* f)
{
    std::string text;
    int c;

    while ((c = PAKL_fgetc(f)) != EOF)
    {
        if (c == '<')
        {
            PAKL_ungetc(c, f);
            break;
        }

        text += (char)c;
    }

    return text;
}

void skipComment(PAK_FILE* f)
{
    int c;

    while ((c = PAKL_fgetc(f)) != EOF)
    {
        if (c != '-')
            continue;

        int c2 = PAKL_fgetc(f);
        if (c2 == EOF)
            return;

        if (c2 != '-')
            continue;

        int c3 = PAKL_fgetc(f);
        if (c3 == EOF)
            return;

        if (c3 == '>')
            return;

        PAKL_ungetc(c3, f);
    }
}

void skipDeclaration(PAK_FILE* f)
{
    int c;

    while ((c = PAKL_fgetc(f)) != EOF)
    {
        if (c == '>')
            return;
    }
}

std::string readRawUntilClosingTag(PAK_FILE* f, const std::string& tag)
{
    std::string text;
    std::string endTag = "</" + tag + ">";

    size_t matched = 0;
    int c;

    while ((c = PAKL_fgetc(f)) != EOF)
    {
        text += (char)c;

        if ((char)c == endTag[matched])
        {
            matched++;

            if (matched == endTag.size())
            {
                // Eliminar "</tag>" del texto
                text.resize(text.size() - endTag.size());
                break;
            }
        }
        else
        {
            matched = ((char)c == endTag[0]) ? 1 : 0;
        }
    }

    return text;
}

HtmlNode parseNode(PAK_FILE* f)
{
    HtmlContentObject object;

    std::string tagName = readTagName(f);
    object.tag = parseTag(tagName);

    bool selfClosing = readAttributes(f, object);

    if (selfClosing)
        return object;

    // Caso especial para <title>
    if (object.tag == HtmlTags::title)
    {
        object.objects.push_back(readRawUntilClosingTag(f, tagName));
        return object;
    }

    while (true)
    {
        int c = PAKL_fgetc(f);

        if (c == EOF)
            break;

        if (c == '<')
        {
            c = PAKL_fgetc(f);

            if (c == '/')
            {
                std::string closing = readTagName(f);

                if (parseTag(closing) != object.tag)
                {
                    // TODO: error de etiquetas
                }

                // Consumir hasta '>'
                while ((c = PAKL_fgetc(f)) != EOF && c != '>');

                break;
            }

            if (c == '!')
            {
                int a = PAKL_fgetc(f);

                if (a == '-')
                {
                    int b = PAKL_fgetc(f);

                    if (b == '-')
                    {
                        skipComment(f);
                        continue;
                    }

                    PAKL_ungetc(b, f);
                }

                PAKL_ungetc(a, f);

                skipDeclaration(f);
                continue;
            }

            PAKL_ungetc(c, f);

            object.objects.push_back(parseNode(f));
        }
        else
        {
            PAKL_ungetc(c, f);

            std::string text = readText(f);

            if (!text.empty())
                object.objects.push_back(text);
        }
    }

    return object;
}

HtmlContent parseHtml(std::string path)
{
    HtmlContent content;
    content.loaded = false;

    std::string fp = getRomfsPath(path.c_str());

    PAK_FILE* f = PAKL_LoadFile(fp.c_str());

    if (!f)
        return content;

    int c;

    while ((c = PAKL_fgetc(f)) != EOF)
    {
        if (c != '<')
            continue;

        c = PAKL_fgetc(f);

        if (c == '!')
        {
            int a = PAKL_fgetc(f);

            if (a == '-')
            {
                int b = PAKL_fgetc(f);

                if (b == '-')
                {
                    skipComment(f);
                    continue;
                }

                PAKL_ungetc(b, f);
            }

            PAKL_ungetc(a, f);

            skipDeclaration(f);
            continue;
        }

        PAKL_ungetc(c, f);

        content.objects.push_back(parseNode(f));
    }

    PAKL_CloseFile(f);

    content.loaded = true;

    return content;
}

bool html::loaded()
{
    return content.loaded;
}

HtmlContent &html::GetContent()
{
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
