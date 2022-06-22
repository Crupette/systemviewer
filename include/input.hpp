#ifndef INPUT_HPP
#define INPUT_HPP 1

namespace input
{

enum extkeys {
    CTRL_RANGE_START = 256,
    CTRL_KEY_ARROWUP = 321,
    CTRL_KEY_ARROWDOWN,
    CTRL_KEY_ARROWRIGHT,
    CTRL_KEY_ARROWLEFT,
    CTRL_KEY_END = 326,
    CTRL_KEY_HOME = 328,
    CTRL_KEY_PAGEUP,
    CTRL_KEY_PAGEDOWN
};

#ifdef __unix__
#include <termios.h>
#include <unistd.h>

class Context
{
    struct termios m_termold;
    struct termios m_termnow;
public:
    Context() {
        tcgetattr(STDIN_FILENO, &m_termold);
        m_termnow = m_termold;
    }
    ~Context() {
        tcsetattr(STDIN_FILENO, TCSADRAIN, &m_termold);
    }

    void echo(bool mode) {
        if(mode){
            m_termnow.c_lflag |= ECHO;
        }else{
            m_termnow.c_lflag &= ~ECHO;
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &m_termnow);
    }

    void canon(bool mode) {
        if(mode) {
            m_termnow.c_lflag |= ICANON;
        }else{
            m_termnow.c_lflag &= ~ICANON;
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &m_termnow);
    }

    void cbreak(bool mode) {
        if(mode){
            m_termnow.c_cc[VMIN] = m_termold.c_cc[VMIN];
            m_termnow.c_cc[VTIME] = m_termold.c_cc[VTIME];
        }else{
            m_termnow.c_cc[VMIN] = 1;
            m_termnow.c_cc[VTIME] = 0;
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &m_termnow);
    }
};

#endif

extern int getcode();
}

#endif
