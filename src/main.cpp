#include <iostream>
#include "game.hpp"
#include "diargs.hpp"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

void
printusage(int err)
{
    std::cout << "systemviewer " << VERSION << std::endl << 
        "Usage: systemviewer [OPTION]... [FILE]" << std::endl << 
        "With no FILE, FILE is assumed to be data/sol.csv" << std::endl <<
        "-h --help : print this message" << std::endl;

    std::exit(err);
}

int
main(int argc, char **argv)
{
    std::string system = "dat/sol.csv";
    bool helpflag;

    diargs::ArgsPair args{argc, argv};
    diargs::ArgumentList arglist(
        diargs::OrderedArgument<std::string>(system),
        diargs::ToggleArgument<bool>("help", 'h', helpflag, true)
            );
    diargs::ArgumentParser(printusage, arglist, args);

    if(helpflag) printusage(0);

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    fcntl(STDIN_FILENO, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

    Game::setup(w.ws_col, w.ws_row, system);

    while(Game::running()) {
        Game::turn();
    }

    Game::cleanup();
    return 0;
}
