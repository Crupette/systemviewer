#include "input.hpp"
namespace input {

int getcode() {
    int r = 0;
    char c;
    if(read(STDIN_FILENO, &c, 1) != 1) return -1;
    if(c == '\x1b') {
        char seq[3];
        if(read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if(read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
        if(seq[0] == '[') {
            if(seq[1] >= 0 && seq[1] <= '9') {
                if(read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                if(seq[2] == '~') {
                    switch(seq[1]) {
                        case '1':
                        case '7': return CTRL_KEY_HOME;
                        case '4':
                        case '8': return CTRL_KEY_END;
                        default: return CTRL_RANGE_START + seq[1] + 20;
                    }
                }
            }else{
                switch(seq[1]) {
                    default: return CTRL_RANGE_START + seq[1];
                }
            }
        }else if(seq[0] == 'O') {
            return CTRL_RANGE_START + seq[1];
        }
        return '\x1b';
    }else{
        return c;
    }
}
}
