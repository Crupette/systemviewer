#include "window.hpp"
#include "keybind.hpp"

void 
Window::draw(bool focus)
{
    if(m_hidden) return;
    if(focus) {
        m_border << straw::setcolor(straw::BLACK, straw::WHITE) << straw::clear(' ') << straw::move(0, 0) << m_title << straw::resetcolor();
    }else{
        m_border << straw::setcolor(straw::WHITE, straw::BLACK) << straw::clear(' ') << straw::move(0, 0) << m_title;
    }
    m_border.flush();
    m_screen.flush();
}

void 
WindowContext::registerWindow(const std::string &id, 
                        const std::string &title, 
                        unsigned x, unsigned y, 
                        unsigned w, unsigned h,
                        bool hidden)
{
    m_windows.emplace(id, Window(title, x, y, w, h, hidden));
    m_windowOrder.push_back(id);
}

void
WindowContext::update(int code)
{
    if(code == KeyMan::binds[BIND_G_NEXTWIN].code) {
        m_focus = (m_focus + 1) % m_windows.size();
        while(m_windows.at(m_windowOrder[m_focus]).hidden()) {
            m_focus = (m_focus + 1) % m_windows.size();
        }
    }
    if(code == KeyMan::binds[BIND_G_PREVWIN].code) {
        m_focus = (m_focus == 0 ? m_windows.size() - 1 : m_focus - 1);
        while(m_windows.at(m_windowOrder[m_focus]).hidden()) {
            m_focus = (m_focus == 0 ? m_windows.size() - 1 : m_focus - 1);
        }
    }
}

void
WindowContext::draw()
{
    for(unsigned i = 0; i < m_windows.size(); i++) {
        m_windows.at(m_windowOrder[i]).draw(i == m_focus);
    }
}

void 
WindowContext::focus(const std::string &id)
{
    for(unsigned i = 0; i < m_windowOrder.size(); i++) {
        if(m_windowOrder[i] == id) {
            m_focus = i;
            return;
        }
    }
}

void
WindowContext::setWindowHidden(const std::string &id, bool mode)
{
    m_windows.at(id).setHidden(mode);
    for(unsigned i = 0; i < m_windows.size(); i++) {
        if(m_windowOrder[i] == id && mode) continue; 
        m_windows.at(m_windowOrder[i]) << straw::redraw();
    }
}
