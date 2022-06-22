#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H 1

#include "util.hpp"
#include "units.hpp"
#include "straw.hpp"
#include "window.hpp"

class TimeMan {
    static unit::Time m_time;
    static unit::Time m_step;
    static bool m_auto;
    static bool m_changed;
public:

    static void init();

    static void update(int c);
    static void draw();

    static unit::Time time() { return m_time; }
    static bool automatic() { return m_auto; }
    static void interrupt() { m_auto = false; }
    static bool changed() { return m_changed; }
};

#endif
