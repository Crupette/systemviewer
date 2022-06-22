#include "timeman.hpp"
#include "keybind.hpp"
#include "window.hpp"
#include "game.hpp"

unit::Time TimeMan::m_time(0);
unit::Time TimeMan::m_step(unit::DAY_SECONDS);
bool TimeMan::m_auto;
bool TimeMan::m_changed;

void
TimeMan::init()
{
    KeyMan::registerBind('.', BIND_TIMEMAN_STEP, CTX_TIMEMAN, "Move time ahead by a step");
    KeyMan::registerBind('+', BIND_TIMEMAN_INCSTEP, CTX_TIMEMAN, "Increase the timestep");
    KeyMan::registerBind('-', BIND_TIMEMAN_DECSTEP, CTX_TIMEMAN, "Decrease the timestep");
    KeyMan::registerBind('a', BIND_TIMEMAN_TOGGLEAUTO, CTX_TIMEMAN, "Toggle if time will move automatically");
    m_changed = true;
}

void 
TimeMan::update(int c)
{
    WindowContext &context = Game::contexts();
    m_changed = false;
    if(!Game::paused()) {
        if(m_auto) {
            m_time += (m_step);
            m_changed = true;
        }
    }
    if(context.getFocusedString() != WINDOW_TIMEMAN_ID) return;
    if(c == KeyMan::binds[BIND_TIMEMAN_INCSTEP].code) m_step = unit::Time(std::max<long>(1, m_step() * 2));
    if(c == KeyMan::binds[BIND_TIMEMAN_DECSTEP].code) m_step = unit::Time(std::max<long>(1, m_step() / 2));
    if(c == KeyMan::binds[BIND_TIMEMAN_TOGGLEAUTO].code) m_auto = !m_auto;
    if(c == KeyMan::binds[BIND_TIMEMAN_STEP].code && !Game::paused()) { m_time += m_step; m_changed = true; }
}

void 
TimeMan::draw()
{
    WindowContext &context = Game::contexts();
    Window &timeWindow = context[WINDOW_TIMEMAN_ID];
    timeWindow << straw::clear(' ');

    timeWindow << straw::move(0, 0) << straw::resetcolor() << m_time.format("%S %D, %C \n%H:%m\n\n");
    timeWindow << m_step.format("Step:\n%Y Years, %M Months\n%D Days, %H Hours\n%m Minutes, %s Seconds\n\n");
    if(m_auto) timeWindow << "Auto";
}
