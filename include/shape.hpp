#ifndef SHAPE_HPP
#define SHAPE_HPP 1

#include "vex.hpp"
#include <iostream>
#include <variant>

namespace shapes {

template<vex::arithmetic T>
struct shape
{
    vex::vec2<T> position;

    constexpr shape(const vex::vec2<T> p) : position(p) {}
    constexpr shape(T x, T y) : position(x, y) {}

    virtual void scale(T by) = 0;
    virtual void translate(vex::vec2<T> by) { position += by; }
};

template<vex::arithmetic T>
struct point : public shape<T>
{
    constexpr point(T x, T y) : shape<T>(vex::vec2<T>(x, y)) {}
    constexpr point(const vex::vec2<T> &pos) : shape<T>(pos) {}

    void scale(T by) override { (void)by; }
};

template<vex::arithmetic T>
struct line : public shape<T>
{
    vex::vec2<T> end;
    constexpr line(T x, T y, T z, T w) : shape<T>(vex::vec2<T>(x, y)), end(z, w){}
    constexpr line(const vex::vec2<T> &pos, const vex::vec2<T> End) : shape<T>(pos), end(End) {}

    void translate(vex::vec2<T> by)
    {
        this->position += by;
        end += by;
    }
    void scale(T by) override {
        end = vex::vec2<T>(
        (T)std::floor((double)end[0] / (double)by),
        (T)std::floor((double)end[1] / (double)by));
    }
};

template<vex::arithmetic T>
struct circle : public shape<T>
{
    T radius;
    constexpr circle(T x, T y, T r) : shape<T>(vex::vec2<T>(x, y)), radius(r) {}
    constexpr circle(const vex::vec2<T> &pos, T r) : shape<T>(pos), radius(r) {}

    void scale(T by) override { radius /= by; }
};

template<vex::arithmetic T>
struct ellipse : public shape<T>
{
    T a, b;
    constexpr ellipse(T x, T y, T A, T B) : shape<T>(x, y), a(A), b(B) {}
    constexpr ellipse(const vex::vec2<T> &pos, T A, T B) : shape<T>(pos), a(A), b(B) {}

    void scale(T by) override {a /= by; b /= by; }
};

template<vex::arithmetic T>
struct rectangle : public shape<T>
{
    vex::vec2<T> bounds;
    constexpr rectangle(T x, T y, T w, T h) : shape<T>(vex::vec2<T>(x, y)), bounds(w, h) {}
    constexpr rectangle(const vex::vec2<T> &pos, const vex::vec2<T> &wh) : shape<T>(pos), bounds(wh) {}

    void scale(T by) override { bounds /= by; }
};

template<vex::arithmetic T>
using shapes_variant = std::variant<
    point<T>, line<T>, circle<T>, ellipse<T>, rectangle<T>
>;

template<vex::arithmetic L, vex::arithmetic R>
static bool 
intersects(const circle<L> &lhs, const point<R> &rhs)
{
    vex::vec2<L> dist = rhs.position - lhs.position;
    //std::cout << dist.magnitude() << ' ' << lhs.radius << ' ' << dist.magnitude() - lhs.radius << std::endl;
    return dist.magnitude() < lhs.radius;
}
template<vex::arithmetic L, vex::arithmetic R>
static bool intersects(const point<L> &lhs, const circle<R> &rhs) { return intersects(rhs, lhs); }

template<vex::arithmetic L, vex::arithmetic R>
static bool 
intersects(const ellipse<L> &lhs, const point<R> &rhs)
{
    auto ds = lhs.position - rhs.position;
    auto d2 = ds * ds;
    L a2 = lhs.a * lhs.a;
    L b2 = lhs.b * lhs.b;

    return (((double)d2[0] / a2) + ((double)d2[1] / b2)) <= 1;
}
template<vex::arithmetic L, vex::arithmetic R>
static bool intersects(const point<L> &lhs, const ellipse<R> &rhs) { return intersects(rhs, lhs); }

template<vex::arithmetic L, vex::arithmetic R>
static bool 
intersects(const rectangle<L> &lhs, const point<R> &rhs)
{
    vex::vec2<L> corner = lhs.position + lhs.bounds;
    return (rhs.position[0] > lhs.position[0] && rhs.position[0] < corner[0] &&
            rhs.position[1] > lhs.position[1] && rhs.position[1] < corner[1]);
}
template<vex::arithmetic L, vex::arithmetic R>
static bool intersects(const point<L> &lhs, const rectangle<R> &rhs) { return intersects(rhs, lhs); }

template<vex::arithmetic L, vex::arithmetic R>
static bool 
intersects(const line<L> &lhs, const line<R> &rhs)
{
    double x1 = lhs.position[0], x2 = lhs.end[0], x3 = rhs.position[0], x4 = rhs.end[0];
    double y1 = lhs.position[1], y2 = lhs.end[1], y3 = rhs.position[1], y4 = rhs.end[1];
    double uA = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
    double uB = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));

    if(uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1) return true;
    return false;
}

template<vex::arithmetic L, vex::arithmetic R>
static bool 
intersects(const rectangle<L> &lhs, const line<R> &rhs)
{
    vex::vec2<L> corner = lhs.position + lhs.bounds;
    line<R> l(lhs.position, lhs.position + vex::vec2<R>(0, lhs.bounds[1]));
    line<R> r(lhs.position + vex::vec2<R>(lhs.bounds[0], 0), corner);
    line<R> d(lhs.position, lhs.position + vex::vec2<R>(lhs.bounds[0], 0));
    line<R> u(lhs.position + vex::vec2<R>(0, lhs.bounds[1]), corner);

    if(intersects(rhs, l) || intersects(rhs, r) || intersects(rhs, d) || intersects(rhs, u)) return true;
    return (rhs.position[0] > lhs.position[0] && rhs.position[0] < corner[0] &&
            rhs.position[1] > lhs.position[1] && rhs.position[1] < corner[1]) &&
           (rhs.end[0] > lhs.position[0] && rhs.end[0] < corner[0] &&
            rhs.end[1] > lhs.position[1] && rhs.end[1] < corner[1]);
}
template<vex::arithmetic L, vex::arithmetic R>
static bool intersects(const line<L> &lhs, const rectangle<R> &rhs) { return intersects(rhs, lhs); }

template<vex::arithmetic L, vex::arithmetic R>
static bool
intersects(const rectangle<L> &lhs, const circle<R> &rhs)
{
    vex::vec2<L> dist{
        rhs.position[0] - std::max(lhs.position[0], std::min(rhs.position[0], lhs.position[0] + lhs.bounds[0])),
        rhs.position[1] - std::max(lhs.position[1], std::min(rhs.position[1], lhs.position[1] + lhs.bounds[1])),
    };
    return (dist[0] * dist[0]) + (dist[1] * dist[1]) < (rhs.radius * rhs.radius);
}
template<vex::arithmetic L, vex::arithmetic R>
static bool intersects(const circle<L> &lhs, const rectangle<R> &rhs) { return intersects(rhs, lhs); }

template<vex::arithmetic L, vex::arithmetic R>
static bool
intersects(const rectangle<L> &lhs, const ellipse<R> &rhs)
{
    vex::vec2<L> dist{
        rhs.position[0] - std::max(lhs.position[0], std::min(rhs.position[0], lhs.position[0] + lhs.bounds[0])),
        rhs.position[1] - std::max(lhs.position[1], std::min(rhs.position[1], lhs.position[1] + lhs.bounds[1])),
    };
    vex::vec2<R> ab2 {
        std::max<R>(1, rhs.a * rhs.a),
        std::max<R>(1, rhs.b * rhs.b)
    };
    return ((dist[0] * dist[0]) / ab2[0]) + ((dist[1] * dist[1]) / ab2[1]) <= 1.0;
}
template<vex::arithmetic L, vex::arithmetic R>
static bool intersects(const ellipse<L> &lhs, const rectangle<R> &rhs) { return intersects(rhs, lhs); }

template<vex::arithmetic L, vex::arithmetic R>
static bool
intersects(const rectangle<L> &lhs, const rectangle<R> &rhs)
{
    return (
        (lhs.position[0] < rhs.position[0] + rhs.bounds[0]) &&
        (lhs.position[0] + lhs.bounds[0] > rhs.position[0]) &&
        (lhs.position[1] + lhs.bounds[1] < rhs.position[1]) &&
        (lhs.position[1] > rhs.position[1] + rhs.position[1])
            );
}

}

#endif
