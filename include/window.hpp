#ifndef WINDOW_HPP
#define WINDOW_HPP 1

#include "util.hpp"
#include "straw.hpp"
#include <memory>
#include <unordered_map>

//Wrapper for screens that act like windows
class Window {
    straw::screen<screenchr> m_border;
    straw::screen<screenchr> m_screen;
    std::string m_title;
    bool m_hidden;
public:
    explicit Window(const std::string Title, unsigned X, unsigned Y, unsigned W, unsigned H, bool hidden = false) :
        m_border(X, Y, W, 1), m_screen(X, Y+1, W, H-1), m_title(Title), m_hidden(hidden) {}
    ~Window() = default;

    template<typename T>
    friend Window &operator<<(Window &o, const T &t) {
        o.m_screen << t;
        return o;
    }
    
    straw::screen<screenchr> *screen() { return &m_screen; }
    std::string title() const { return m_title; }

    void draw(bool focus);

    void setHidden(bool mode) { m_hidden = mode; }
    bool hidden() const { return m_hidden; }
};

#define WINDOW_SYSTEMVIEW_ID "Systemview"
#define WINDOW_SYSTEMVIEW_SEARCH_ID "SystemviewSearch"
#define WINDOW_BODYINFO_ID "Bodyinfo"
#define WINDOW_EVENTS_ID "Events"
#define WINDOW_TIMEMAN_ID "Timeman"

class WindowContext {
    std::unordered_map<std::string, Window> m_windows;
    std::vector<std::string> m_windowOrder;
    unsigned m_focus;
public:
    WindowContext() : m_focus(0) {}
    ~WindowContext() {}

    void registerWindow(const std::string &id, 
                        const std::string &title, 
                        unsigned x, unsigned y, 
                        unsigned w, unsigned h,
                        bool hidden = false);

    Window &operator[](const std::string &id) { return m_windows.at(id); }
    Window &operator[](unsigned id) { return m_windows.at(m_windowOrder[id]); }
    Window &operator()() { return m_windows.at(m_windowOrder[m_focus]); }

    void update(int code);
    void draw();

    void setWindowHidden(const std::string &id, bool mode);
    void focus(const std::string &id);

    unsigned getFocused() { return m_focus; }
    std::string getFocusedString() { return m_windowOrder[m_focus]; }
};

#endif
