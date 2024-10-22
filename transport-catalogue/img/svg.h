#pragma once
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>


namespace svg {

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

//using Color = std::string;


struct Rgb {
    Rgb()
        :red(0), green(0), blue(0) {}

    Rgb(uint8_t r, uint8_t g, uint8_t b)
        : red(r), green(g), blue(b) {}

    uint8_t red = 0u;
    uint8_t green = 0u;
    uint8_t blue = 0u;
};

struct Rgba {
    Rgba()
        :red(0), green(0), blue(0), opacity(1) 
    {}

    Rgba(uint8_t r, uint8_t g, uint8_t b, double o)
        : red(r), green(g), blue(b), opacity(o)
    {}

    uint8_t red = 0u;
    uint8_t green = 0u;
    uint8_t blue = 0u;
    double opacity = 1;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{ "none" };

struct ColorPrinter {
    std::ostream& out;
    
    void operator()(std::monostate) const {
        out << "none";
    }
    void operator()(std::string str) const {
        out << str;
    }
    void operator()(Rgb rgb) const {
        out << "rgb(" << static_cast<int>(rgb.red) << "," << static_cast<int>(rgb.green) << "," << static_cast<int>(rgb.blue) << ")";
    }
    void operator()(Rgba rgba) const {
        out << "rgba(" << static_cast<int>(rgba.red) << "," << static_cast<int>(rgba.green) << "," << static_cast<int>(rgba.blue) << "," << rgba.opacity << ")";
    }
};

std::ostream& operator<<(std::ostream& out, const Color& value);


/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

class ObjectContainer {
public:

    template <typename Obj>
    void Add(Obj obj) {
        AddPtr(std::make_unique<Obj>(std::move(obj)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    virtual ~ObjectContainer() = default;

private:

    std::vector<std::unique_ptr<Object>> objects_;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& value);

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& value);

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color fill_color) {
        fill_color_ = std::move(fill_color);
        //fill_color_ = visit(ColorPrinter{ cout }, fill_color);
        return AsOwner();
    }

    Owner& SetStrokeColor(Color stroke_color) {
        stroke_color_ = std::move(stroke_color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = std::move(width);
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_linecap_ = std::move(line_cap);
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_linejoin_ = std::move(line_join);
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\"" << *stroke_color_ << "\"";
        }
        if (stroke_width_) {
            out << " stroke-width=\"" << *stroke_width_ << "\"";
        }
        if (stroke_linecap_) {
            out << " stroke-linecap=\"" << *stroke_linecap_ << "\"";
        }
        if (stroke_linejoin_) {
            out << " stroke-linejoin=\"" << *stroke_linejoin_ << "\"";
        }
    }

private:
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_linecap_;
    std::optional<StrokeLineJoin> stroke_linejoin_;
};

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle()
            : center_({ 0,0 }), radius_(1)
        {}

        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        Polyline()
            : points_()
        {}

        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:
        Text()
            : position_({ 0.0,0.0 }),
            offset_({ 0.0,0.0 }),
            size_(1),
            font_family_(""),
            font_weight_(""),
            data_("")
        {}
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point position_;
        Point offset_;
        uint32_t size_;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };


class Document : public ObjectContainer {
public:

    void AddPtr(std::unique_ptr<Object>&& obj) override ;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

private:

    std::vector<std::unique_ptr<Object>> objects_;

};


}  // namespace svg