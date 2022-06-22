#ifndef CAMERA_HPP
#define CAMERA_HPP 1

#include "vex.hpp"
#include "shape.hpp"
#include "straw.hpp"

#include <vector>
#include <concepts>
#include <algorithm>

class Camera;
struct RenderBatchEntry {
    shapes::shapes_variant<long> shape;
    straw::color fg, bg;
    char c;

    template<class T>
    constexpr RenderBatchEntry(T Shape) : shape(Shape), fg(straw::WHITE), bg(straw::BLACK), c('#') {}
    template<class T>
    constexpr RenderBatchEntry(T Shape, char C) : shape(Shape), fg(straw::WHITE), bg(straw::BLACK), c(C) {}
    template<class T>
    constexpr RenderBatchEntry(T Shape, straw::color Fg, char C) : shape(Shape), fg(Fg), bg(straw::BLACK), c(C) {}
    template<class T>
    constexpr RenderBatchEntry(T Shape, straw::color Fg, straw::color Bg, char C) : shape(Shape), fg(Fg), bg(Bg), c(C) {}

    void translate(Camera *camera);
    void plot(Camera *camera);
private:
    void plotPoint(shapes::point<long> point, Camera *camera);
    void plotLine(shapes::line<long> line, Camera *camera);
    void plotRectangle(shapes::rectangle<long> rectangle, Camera *camera);
    void plotCircle(shapes::circle<long> circle, Camera *camera);
    void plotEllipse(shapes::ellipse<long> ellipse, Camera *camera);
};

class Camera
{
    friend struct RenderBatchEntry;

    vex::vec2<long> m_position;
    vex::vec2<long> m_origin;
    shapes::rectangle<long> m_frustum;

    std::vector<RenderBatchEntry> m_shapeBatch;

    long m_scale;
    bool m_dirty;

    straw::screen<char> *m_viewport;

    void updateFrustum();

    template<class T>
    void translateShape(T &shape);
public:
    Camera(straw::screen<char> *viewport) :
        m_position(0, 0),
        m_origin(0, 0),
        m_frustum(0, 0, 0, 0),
        m_scale(16384),
        m_dirty(true),
        m_viewport(viewport) {}
    ~Camera() {}

    constexpr long getscale() const { return m_scale; }
    constexpr vex::vec2<long> getpos() const { return m_position; }
    constexpr vex::vec2<long> getorigin() const { return m_origin; }

    constexpr bool dirty() const { return m_dirty; }
    void markDirty() { m_dirty = true; }

    void zoom(long by) { m_scale = std::max((unsigned)((int)m_scale + by), (unsigned)1); m_dirty = true; }
    void setscale(long scale) { m_scale = std::max(scale, (long)1); m_dirty = true; }

    void move(const vex::vec2<long> &pos) { m_position += pos; m_dirty = true; }
    void move(long x, long y) { move(vex::vec2<long>(x, y)); }

    void setpos(const vex::vec2<long> &pos) { m_position = pos; m_dirty = true; }
    void setpos(long x, long y) { setpos(vex::vec2<long>(x, y)); }

    void setorigin(const vex::vec2<long> &origin) { m_origin = origin; m_dirty = true; }

    template<class T>
    void batchShape(T shape) { m_shapeBatch.emplace_back(shape); }
    template<class T>
    void batchShape(T shape, char c) { m_shapeBatch.emplace_back(shape, c); }
    template<class T>
    void batchShape(T shape, straw::color fg, char c) { m_shapeBatch.emplace_back(shape, fg, c); }
    template<class T>
    void batchShape(T shape, straw::color fg, straw::color bg, char c) { m_shapeBatch.emplace_back(shape, fg, bg, c); }

    void draw();
};

#endif
