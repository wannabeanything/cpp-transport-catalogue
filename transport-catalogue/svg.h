#pragma once
#define _USE_MATH_DEFINES 
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <variant>
namespace svg {
struct Rgb {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;

    Rgb() = default;
    Rgb(uint8_t r, uint8_t g, uint8_t b)
        : red(r), green(g), blue(b) {}
};

struct Rgba {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;

    Rgba() = default;
    Rgba(uint8_t r, uint8_t g, uint8_t b, double a)
        : red(r), green(g), blue(b), opacity(a) {}
};
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

// Объявив в заголовочном файле константу со спецификатором inline,
// мы сделаем так, что она будет одной на все единицы трансляции,
// которые подключают этот заголовок.
// В противном случае каждая единица трансляции будет использовать свою копию этой константы
inline const Color NoneColor = std::monostate{};

struct ColorVisitor {
    std::ostream& os;
    void operator()(std::monostate) const { os << "none"; }
    void operator()(const std::string& color) const { os << color; }
    void operator()(const Rgb& rgb) const { os << "rgb(" << int(rgb.red) << "," << int(rgb.green) << "," << int(rgb.blue) << ")"; }
    void operator()(const Rgba& rgba) const { os << "rgba(" << int(rgba.red) << "," << int(rgba.green) << "," << int(rgba.blue) << "," << rgba.opacity << ")"; }
};

inline std::ostream& operator<<(std::ostream& os, const Color& color) {
    std::visit(ColorVisitor{os}, color);
    return os;
}
struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x), y(y) {
    }
    double x = 0;
    double y = 0;
};

struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out), indent_step(indent_step), indent(indent) {
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

class Object {
public:
    void Render(const RenderContext& context) const;
    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};
// ObjectContainer interface
class ObjectContainer {
public:
    virtual ~ObjectContainer() = default;

    template <typename T>
    void Add(T&& obj) {
        static_assert(std::is_base_of_v<Object, std::decay_t<T>>, "T must derive from Object");
        AddPtr(std::make_unique<std::decay_t<T>>(std::forward<T>(obj)));
    }

protected:
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
};
// Drawable interface
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

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};
inline std::ostream& operator<<(std::ostream& out, StrokeLineCap cap) {
    switch (cap) {
        case StrokeLineCap::BUTT: return out << "butt";
        case StrokeLineCap::ROUND: return out << "round";
        case StrokeLineCap::SQUARE: return out << "square";
    }
    return out;
}

inline std::ostream& operator<<(std::ostream& out, StrokeLineJoin join) {
    switch (join) {
        case StrokeLineJoin::ARCS: return out << "arcs";
        case StrokeLineJoin::BEVEL: return out << "bevel";
        case StrokeLineJoin::MITER: return out << "miter";
        case StrokeLineJoin::MITER_CLIP: return out << "miter-clip";
        case StrokeLineJoin::ROUND: return out << "round";
    }
    return out;
}
template<typename Owner>
class PathProps{
public:
    Owner& SetFillColor(Color color){
        fill_color_ = color;
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color){
        stroke_color_ = color;
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width){
        stroke_width_ = width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap){
        line_cap_ = line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join){
        line_join_ = line_join;
        return AsOwner();
    }
protected:
    ~PathProps() = default;
    std::optional<Color> GetFillColor() const{
        return fill_color_;
    }
    std::optional<Color> GetStrokeColor() const{
        return stroke_color_;
    }
    std::optional<double> GetStrokeWidth() const {
        return stroke_width_;
    }
    std::optional<StrokeLineCap> GetLineCap() const{
        return line_cap_;
    }
    std::optional<StrokeLineJoin> GetLineJoin() const{
        return line_join_;
    }
    virtual void RenderAttrs(std::ostream& out) const {
        if (GetFillColor() != std::nullopt) {
           out << " fill=\"" << *fill_color_ << "\"";
        }
        if (GetStrokeColor() != std::nullopt) {
            out << " stroke=\"" << *stroke_color_ << "\"";
        }
        if (GetStrokeWidth() != std::nullopt) {
            out << " stroke-width=\"" << *stroke_width_ << "\"";
        }
        if (GetLineCap() != std::nullopt) {
            out << " stroke-linecap=\"" << *line_cap_ << "\"";
        }
        if (GetLineJoin() != std::nullopt) {
            out << " stroke-linejoin=\"" << *line_join_ << "\"";
        }
}
private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;    
};


class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;
    void RenderAttrs(std::ostream& out) const override{
        PathProps<Circle>::RenderAttrs(out);
    }
    Point center_;
    double radius_ = 1.0;
};

class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline& AddPoint(Point point);
    
private:
    void RenderObject(const RenderContext& context) const override;
    void RenderAttrs(std::ostream& out) const override{
        PathProps<Polyline>::RenderAttrs(out);
    }
    std::vector<Point> points_;
};

class Text final : public Object, public PathProps<Text> {
public:
    Text& SetPosition(Point pos);
    Text& SetOffset(Point offset);
    Text& SetFontSize(uint32_t size);
    Text& SetFontFamily(std::string font_family);
    Text& SetFontWeight(std::string font_weight);
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;
    void RenderAttrs(std::ostream& out) const override{
        PathProps<Text>::RenderAttrs(out);
    }
    Point position_;
    Point offset_;
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
};

class Document : public ObjectContainer {
public:
    void Render(std::ostream& out) const;
    void AddPtr(std::unique_ptr<Object>&& obj) override {
        objects_.push_back(std::move(obj));
    }

private:
    std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg
namespace shapes {

// Triangle class implementation
class Triangle : public svg::Drawable {
public:
    Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
        : p1_(p1), p2_(p2), p3_(p3) {
    }

    void Draw(svg::ObjectContainer& container) const override {
        container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
    }

private:
    svg::Point p1_, p2_, p3_;
};

// Helper function for Star
namespace {
svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
    using namespace svg;
    Polyline polyline;
    for (int i = 0; i <= num_rays; ++i) {
        double angle = 2 * M_PI * (i % num_rays) / num_rays;
        polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
        if (i == num_rays) {
            break;
        }
        angle += M_PI / num_rays;
        polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
    }
    return polyline;
}
} // anonymous namespace

// Star class implementation
class Star : public svg::Drawable {
public:
    Star(svg::Point center, double outer_radius, double inner_radius, int num_rays)
        : center_(center), outer_radius_(outer_radius), inner_radius_(inner_radius), num_rays_(num_rays) {
    }

    void Draw(svg::ObjectContainer& container) const override {
        container.Add(CreateStar(center_, outer_radius_, inner_radius_, num_rays_).SetFillColor("red").SetStrokeColor("black"));
    }

private:
    svg::Point center_;
    double outer_radius_;
    double inner_radius_;
    int num_rays_;
};

// Snowman class implementation
class Snowman : public svg::Drawable {
public:
    Snowman(svg::Point head_center, double head_radius)
        : head_center_(head_center), head_radius_(head_radius) {
    }

    void Draw(svg::ObjectContainer& container) const override {
        using namespace svg;


        container.Add(Circle()
            .SetCenter({head_center_.x, head_center_.y + 2 * head_radius_ + 3 * head_radius_}) 
            .SetRadius(2 * head_radius_).SetFillColor("rgb(240,240,240)").SetStrokeColor("black")); 

        // Draw the middle circle
        container.Add(Circle()
            .SetCenter({head_center_.x, head_center_.y + 2 * head_radius_}) 
            .SetRadius(1.5 * head_radius_).SetFillColor("rgb(240,240,240)").SetStrokeColor("black")); 

        // Draw the head circle
        container.Add(Circle()
            .SetCenter(head_center_) 
            .SetRadius(head_radius_).SetFillColor("rgb(240,240,240)").SetStrokeColor("black")); 
    }

private:
    svg::Point head_center_;
    double head_radius_;
};


} // namespace shapes