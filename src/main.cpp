#include <iostream>
#include "game.hpp"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    fcntl(STDIN_FILENO, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

    Game::setup(w.ws_col, w.ws_row);

    while(Game::running()) {
        Game::turn();
    }

    Game::cleanup();
    return 0;
}
