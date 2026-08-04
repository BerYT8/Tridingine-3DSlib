#pragma once

#include <string>
#include <vector>
#include <variant>

using PropertyValue = std::variant<int, float, std::string, bool>;

enum class HtmlTags {
    html,
    head,
    body,
    h1,
    p,
};

constexpr std::string_view HtmlTagsName[] = {
    "html",
    "head",
    "body",
    "h1",
    "p",
};

struct ObejctProperty
{
    std::string name;
    PropertyValue value;
};

using HtmlNode = std::variant<std::string, HtmlContentObject>;

struct HtmlContentObject
{
    HtmlTags tag;
    std::vector<ObejctProperty> properties;
    std::vector<HtmlNode> objects;
};


struct HtmlContent
{
    std::string title;
    std::vector<HtmlNode> objects;
};


class html
{
private:
    std::string pathPage;
    HtmlContent content;
public:
    html(std::string path);
    ~html();
};