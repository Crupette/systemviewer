#ifndef GAME_HPP
#define GAME_HPP 1

#include "window.hpp"
#include "camera.hpp"
#include "ecs.hpp"
#include "system.hpp"
#include "timeman.hpp"
#include "input.hpp"

#include <memory>

#define WINCTX_GAME "Game"

class Game
{
public:
    enum class State {
        STOPPED, RUNNING, RUNNING_INPUT, PAUSED, PAUSED_INPUT
    };

    static void setup(unsigned w, unsigned h);
    static void cleanup();

    static void turn();
    static void setState(State state) { m_state = state; }
    static void setContext(const std::string &id) { m_currentContext = id; }

    static bool running() { return m_state != State::STOPPED; }
    static bool paused() { return m_state == State::PAUSED || m_state == State::PAUSED_INPUT; }
    static bool inputMode() { return m_state == State::RUNNING_INPUT || m_state == State::PAUSED_INPUT; }

    struct WindowContexts {
        WindowContext &operator[](const std::string &id) { return Game::m_contexts.at(id); }
        WindowContext &operator()() { return Game::m_contexts.at(Game::m_currentContext); }
    };
    static WindowContexts contexts;
private:
    static std::unordered_map<std::string, WindowContext> m_contexts;
    static std::string m_currentContext;
    
    static std::unique_ptr<Camera> m_camera;
    static std::unique_ptr<System> m_system;
    static SystemView m_systemView;
    
    static input::Context m_inputContext;
    
    static double m_delta;
    static State m_state;
};

#endif
