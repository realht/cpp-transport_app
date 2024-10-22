#define _USE_MATH_DEFINES
#include <cmath>

#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

void TextToSVGFormat(std::ostream& out, std::string_view text) {
    for (char c : text) {
        switch (c){
        case '"':
            out << "&quot;"sv;
            break;
        case '<':
            out << "&lt;"sv;
            break;
        case '>':
            out << "&gt;"sv;
            break;
        case '&':
            out << "&amp;"sv;
            break;
        case '\'':
            out << "&apos;"sv;
            break;
        default:
            out << c;
            break;
        }
    }
}

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& value) {
    std::string_view slc;
    switch (value){
    case StrokeLineCap::BUTT:
        slc = "butt"sv;
        break;
    case StrokeLineCap::ROUND:
        slc = "round"sv;
        break;
    case StrokeLineCap::SQUARE:
        slc = "square"sv;
        break;    
    }
    return out << slc;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& value) {
    std::string_view slj;
    switch (value){
    case StrokeLineJoin::ARCS:
        slj = "arcs"sv;
        break;
    case StrokeLineJoin::BEVEL:
        slj = "bevel"sv;
        break;
    case StrokeLineJoin::MITER:
        slj = "miter"sv;
        break;
    case StrokeLineJoin::MITER_CLIP:
        slj = "miter-clip"sv;
        break;
    case StrokeLineJoin::ROUND:
        slj = "round"sv;
        break;
    }
    return out << slj;
}

std::ostream& operator<<(std::ostream& out, const Color& value) {
    std::ostringstream strm;
    std::visit(ColorPrinter{ strm }, value);
    return out << strm.str();
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.emplace_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first_point = true;
    for (const Point& point : points_) {
        if (is_first_point) {
            out << point.x << "," << point.y;
            is_first_point = false;
        }
        else{
            out << " " << point.x << "," << point.y;
        }
    }
    out << "\"";
        RenderAttrs(context.out); 
    out << "/>"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<text";
    RenderAttrs(context.out);
    out << " x=\""sv << position_.x << "\" y=\""sv << position_.y;
    out << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y;
    out << "\" font-size=\""sv << size_ << "\"";
    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\"";
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\"";
    }
    out << ">";
    TextToSVGFormat(out, data_);
    out << "</text>";
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    RenderContext indentation(out, 2, 2);

    for (const auto &o : objects_) {
        o->Render(indentation);
    }
    out << "</svg>"sv;
}

}  // namespace svg