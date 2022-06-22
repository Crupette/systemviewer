#include "camera.hpp"
#include "shape.hpp"
#include <cmath>
#include <algorithm>
#include <variant>
#include <typeinfo>
#include <any>

void 
Camera::updateFrustum(){
    if(m_dirty) {
        *m_viewport << straw::clear(' ');
        m_frustum = shapes::rectangle<long>(0, 0, (long)m_viewport->getwidth(), (long)m_viewport->getheight());
        m_frustum.position -= (m_frustum.bounds / 2);
        m_frustum.position += m_position + m_origin;
        m_dirty = false;
    }
}

void
RenderBatchEntry::translate(Camera *camera)
{
    std::visit([camera](auto &shapeval) {
        shapeval.translate(-camera->m_frustum.position);
        shapeval.translate((camera->m_frustum.bounds * camera->m_scale) / 2);
        shapeval.position = vex::vec2<long>(
                (long)std::floor((double)shapeval.position[0] / (double)camera->m_scale),
                (long)std::floor((double)shapeval.position[1] / (double)camera->m_scale));
        shapeval.scale(camera->m_scale);
        }, shape);
}

void 
Camera::draw()
{
    if(!m_dirty) return;
    updateFrustum();
    shapes::rectangle<long> ssfrustum{vex::vec2<long>(0, 0), vex::vec2<long>(m_viewport->getwidth(), m_viewport->getheight())};

    //Remove all batches not intersecting the screenspace frustum
    m_shapeBatch.erase(
            std::remove_if(
                m_shapeBatch.begin(), 
                m_shapeBatch.end(),
                [ssfrustum, this](RenderBatchEntry &entry){
                    entry.translate(this);
                    return std::visit([ssfrustum](auto &&shape) {
                        return shapes::intersects(shape, ssfrustum) == 0;
                    }, entry.shape);
                }),
            m_shapeBatch.end());
    for(RenderBatchEntry &entry : m_shapeBatch) entry.plot(this);
    m_shapeBatch.clear();
}

void
RenderBatchEntry::plotPoint(shapes::point<long> point, Camera *camera) {
    long ploty = camera->m_viewport->getheight() - point.position[1];
    if(point.position[0] < 0 || point.position[0] >= camera->m_viewport->getwidth() ||
            ploty < 0 || ploty >= camera->m_viewport->getheight()) return;
    *camera->m_viewport << straw::setcolor(fg, bg) << straw::plot(point.position[0], ploty, c);
}

void
RenderBatchEntry::plotLine(shapes::line<long> line, Camera *camera) {
    if(line.end == line.position) {
        plotPoint(shapes::point<long>(line.position), camera);
    }
    vex::vec2<long> vpdim(camera->m_viewport->getwidth(),
                          camera->m_viewport->getheight());
    vex::vec2<long> d(line.end - line.position);
    long adx = std::abs(d[0]);
    long ady = std::abs(d[1]);

    long offX= d[0] > 0 ? 1 : -1;
    long offY = d[1] > 0 ? 1 : -1;

    *camera->m_viewport << straw::setcolor(fg, bg);
    if(adx < ady) {
        long err = ady / 2;
        long x = line.position[0];
        long y = line.position[1];
        for(long i = 0; i < ady; i++) {
            if(x > 0 && x < vpdim[0] && y > 0 && y < vpdim[1]) {
                long ploty = vpdim[1] - y;
                *camera->m_viewport << straw::plot(x, ploty, c);
            }
            if(err >= ady) {
                x += offX;
                y += offY;
                err += adx - ady;
            }else {
                y += offY;
                err += adx;
            }
        }

    }else{
        long err = adx / 2;
        long x = line.position[0];
        long y = line.position[1];
        for(long i = 0; i < adx; i++) {
            if(x > 0 && x < vpdim[0] && y > 0 && y < vpdim[1]) {
                long ploty = vpdim[1] - y;
                *camera->m_viewport << straw::plot(x, ploty, c);
            }
            if(err >= adx) {
                x += offX;
                y += offY;
                err += ady - adx;
            }else {
                x += offX;
                err += ady;
            }
        }
    }
}

void
RenderBatchEntry::plotCircle(shapes::circle<long> circle, Camera *camera) {
    if(circle.radius == 1) {
        plotPoint(shapes::point<long>(circle.position), camera);
        return;
    }
    long sy = std::max<long>(1, circle.position[1] - circle.radius);
    long ey = std::min<long>(camera->m_viewport->getheight(), circle.position[1] + circle.radius);
    *camera->m_viewport << straw::setcolor(fg, bg);
    for(long y = sy; y <= ey; y++) {
        long ploty = camera->m_viewport->getheight() - y;
        long r2 = circle.radius * circle.radius;
        long dy = circle.position[1] - y;
        long dx = (long)std::sqrt(r2 - (dy * dy));
        long sx = std::max<long>(0, circle.position[0] - dx + 1);
        long ex = std::min<long>(camera->m_viewport->getwidth(), circle.position[0] + dx);
        for(unsigned x = (unsigned)sx; x < ex; x++) {
            *camera->m_viewport << straw::plot(x, ploty, c);
        }
    }
}

void
RenderBatchEntry::plotRectangle(shapes::rectangle<long> rectangle, Camera *camera)
{
    if(rectangle.bounds[1] == 1) {
        plotPoint(shapes::point<long>(rectangle.position), camera);
        return;
    }
    long sy = std::max<long>(1, rectangle.position[1]);
    long ey = std::min<long>(camera->m_viewport->getheight(), rectangle.position[1] + rectangle.bounds[1]);
    *camera->m_viewport << straw::setcolor(fg, bg);
    for(long y = sy; y < ey; y++) {
        long ploty = camera->m_viewport->getheight() - y;
        long sx = std::max<long>(0, rectangle.position[1]);
        long ex = std::min<long>(camera->m_viewport->getwidth(), rectangle.position[1] + rectangle.bounds[1]);
        for(unsigned x = (unsigned)sx; x < ex; x++) {
            *camera->m_viewport << straw::plot(x, ploty, c);
        }
    }
}

void
RenderBatchEntry::plotEllipse(shapes::ellipse<long> shapeval, Camera *camera) {
    if(shapeval.a == 1) {
        plotPoint(shapes::point<long>(shapeval.position), camera);
        return;
    }
    long sy = std::max<long>(1, shapeval.position[1] - shapeval.b);
    long ey = std::min<long>(camera->m_viewport->getheight(), shapeval.position[1] + shapeval.b);
    *camera->m_viewport << straw::setcolor(fg, bg);
    for(long y = sy; y <= ey; y++) {
        long ploty = camera->m_viewport->getheight() - y;
        long dy = shapeval.position[1] - y;
        long dy2 = dy * dy;
        long a2 = shapeval.a * shapeval.a;
        long b2 = shapeval.b * shapeval.b;
        long dx = (long)(((2.0 * (double)shapeval.a) /
                    (double)shapeval.b) * std::sqrt(b2 - dy2) / 2.0);
        long sx = std::max<long>(0, shapeval.position[0] - dx + 1);
        long ex = std::min<long>(camera->m_viewport->getwidth(), shapeval.position[0] + dx);

        for(unsigned x = (unsigned)sx; x < ex; x++) {
            *camera->m_viewport << straw::plot(x, ploty, c);
        }       
    }
}
void
RenderBatchEntry::plot(Camera *camera) 
{
    std::visit([this, camera](auto &shapeval) -> void{
        using T = std::decay_t<decltype(shapeval)>;
        if constexpr(std::is_same_v<T, shapes::point<long>>) plotPoint(shapeval, camera);
        if constexpr(std::is_same_v<T, shapes::line<long>>) plotLine(shapeval, camera);
        if constexpr(std::is_same_v<T, shapes::ellipse<long>>) plotEllipse(shapeval, camera);
        if constexpr(std::is_same_v<T, shapes::circle<long>>) plotCircle(shapeval, camera);
        if constexpr(std::is_same_v<T, shapes::rectangle<long>>) plotRectangle(shapeval, camera);
    }, shape);
}
