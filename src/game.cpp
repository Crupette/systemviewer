#include "game.hpp"
#include "keybind.hpp"
#include "input.hpp"
#include <thread>
#include <chrono>

constexpr static double REQUESTED_FPS = 60.0;
constexpr static double REQ_FPS_MSPT = 1000.0 / REQUESTED_FPS;

std::unordered_map<std::string, WindowContext> Game::m_contexts;
std::string Game::m_currentContext;

std::unique_ptr<Camera> Game::m_camera;
std::unique_ptr<System> Game::m_system;
SystemView Game::m_systemView(nullptr);

input::Context Game::m_inputContext;

double Game::m_delta;
Game::State Game::m_state = State::RUNNING;
Game::WindowContexts Game::contexts;

void
Game::setup(unsigned w, unsigned h)
{
    KeyMan::loadKeybindsFrom("keybinds.csv");

    m_inputContext.echo(false);
    m_inputContext.canon(false);
    m_inputContext.cbreak(false);

    unsigned int viewh = h - 1;
    unsigned int infow = 24;
    unsigned int infoh = 12;
    unsigned int timeh = 10;

    TimeMan::init();
    m_contexts.emplace(WINCTX_GAME, WindowContext());
    WindowContext *gameContext = &m_contexts[WINCTX_GAME];

    gameContext->registerWindow(WINDOW_SYSTEMVIEW_ID, "System View", infow, 0, w - infow, viewh);
    gameContext->registerWindow(WINDOW_BODYINFO_ID, "Body Info", 0, 0, infow, infoh);
    gameContext->registerWindow(WINDOW_EVENTS_ID, "Events", 0, infoh, infow, viewh - infoh - timeh);
    gameContext->registerWindow(WINDOW_TIMEMAN_ID, "Time", 0, viewh - timeh, infow, timeh);

    gameContext->registerWindow(WINDOW_SYSTEMVIEW_SEARCH_ID, "Search", infow, 0, (w - infow) / 4, viewh, true);
    m_currentContext = WINCTX_GAME;

    m_camera = std::make_unique<Camera>((*gameContext)[WINDOW_SYSTEMVIEW_ID].screen());
    m_system = std::make_unique<System>();
    m_systemView.view(m_system.get());

    KeyMan::registerBind('\x1B', BIND_G_ESCAPE, CTX_GLOBAL, "Escape from focused searchbox / window");
    KeyMan::registerBind('\n', BIND_G_SELECT, CTX_GLOBAL, "Select something");
    KeyMan::registerBind('x', BIND_G_NEXTWIN, CTX_GLOBAL, "Change the focused window to the next in the stack");
    KeyMan::registerBind('X', BIND_G_PREVWIN, CTX_GLOBAL, "Change the focused window to the previous in the stack");
    KeyMan::registerBind('y', BIND_G_QUIT, CTX_GLOBAL, "Terminate the game");

    KeyMan::registerBind('w', BIND_SYSTEMVIEW_PANUP,    CTX_SYSTEMVIEW, "Moves the camera upwards on the system viewer");
    KeyMan::registerBind('a', BIND_SYSTEMVIEW_PANLEFT,  CTX_SYSTEMVIEW, "Moves the camera to the left on the system viewer");
    KeyMan::registerBind('s', BIND_SYSTEMVIEW_PANDOWN,  CTX_SYSTEMVIEW, "Moves the camera downwards on the system viewer");
    KeyMan::registerBind('d', BIND_SYSTEMVIEW_PANRIGHT, CTX_SYSTEMVIEW, "Moves the camera to the right on the system viewer");
    KeyMan::registerBind('-', BIND_SYSTEMVIEW_INCSCALE, CTX_SYSTEMVIEW, "Decreases zoom from center of screen");
    KeyMan::registerBind('+', BIND_SYSTEMVIEW_DECSCALE, CTX_SYSTEMVIEW, "Increases zoom into center of screen");
    KeyMan::registerBind('/', BIND_SYSTEMVIEW_SEARCH, CTX_SYSTEMVIEW, "Search through bodies in the system");

    KeyMan::registerBind(input::CTRL_KEY_ARROWUP, BIND_SYSTEMVIEW_SEARCH_PREV, CTX_SYSTEMVIEW, "Move the cursor up in the search view");
    KeyMan::registerBind(input::CTRL_KEY_ARROWDOWN, BIND_SYSTEMVIEW_SEARCH_NEXT, CTX_SYSTEMVIEW, "Move the cursor down in the search view");
    KeyMan::registerBind(input::CTRL_KEY_HOME, BIND_SYSTEMVIEW_SEARCH_TOP, CTX_SYSTEMVIEW, "Move to first entry in search view");
    KeyMan::registerBind(input::CTRL_KEY_END, BIND_SYSTEMVIEW_SEARCH_BOTTOM, CTX_SYSTEMVIEW, "Move to last entry in search view");
    KeyMan::registerBind(input::CTRL_KEY_ARROWRIGHT, BIND_SYSTEMVIEW_SEARCH_COLLAPSE, CTX_SYSTEMVIEW, "Toggle collapsed entry in search view");
}

void
Game::cleanup()
{
    KeyMan::writeKeybindsTo("keybinds.csv");
}

void
Game::turn()
{
    auto start = std::chrono::steady_clock::now();
    auto end = start + std::chrono::milliseconds(16);
    int c = input::getcode();
    WindowContext &context = m_contexts.at(m_currentContext);

    /*Global keybinds*/
    if(!inputMode()) {
        if(c == KeyMan::binds[BIND_G_QUIT].code) m_state = State::STOPPED;
        context.update(c);
    }

    if(m_currentContext == WINCTX_GAME) { /*Game Context*/
        m_systemView.keypress(m_camera.get(), c);
        
        m_system->update();
        m_systemView.update(m_camera.get());
        TimeMan::update(c); 
        if(TimeMan::changed()) {
            m_camera->markDirty();
        }

        m_systemView.draw(m_camera.get());
        TimeMan::draw();
        m_camera->draw();
        m_systemView.drawOver(m_camera.get());
        context.draw();
    }

    std::this_thread::sleep_until(end);
    end = std::chrono::steady_clock::now();
    auto diff = end - start;
    m_delta = (double)(std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()) / 1000.0;
}
