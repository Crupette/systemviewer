/**
 * diargs.cpp
 * Copyright (c) 2022 Jon Santmyer <jon@jonsantmyer.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "diargs.hpp"
#include <string_view>
#include <list>

namespace diargs
{

ArgumentParser::ArgumentParser(
        void (*failfunc)(int), 
        ArgumentList &arguments,
        ArgsPair argcv)
{
    /*Raw c strings passed to string_view for safety*/
    std::vector<std::string_view> argv(argcv.argc);
    for(int arg = 0; arg < argcv.argc; arg++) {
        argv[arg] = std::string_view(argcv.argv[arg]);
    }

    /*Skip the first argument because it will always be the path to the program*/
    for(auto it = argv.begin() + 1; it != argv.end(); it++) {
        if(it->at(0) == '-') { /*Argument could be shortform*/
            if(it->at(1) == '-') { /*Argument is longform*/
                std::string_view parsing = std::string_view(it->data() + 2);
                if(parsing.size() == 0) continue;
                for(auto &argdef : arguments.arguments) {
                    if(argdef->longform != parsing) continue;
                    auto newit = argdef->parse(argv, it);
                    if(newit == std::nullopt) {
                        failfunc(-1);
                        return;
                    }
                    it = newit.value_or(argv.end());
                    break;
                }
                continue;
            }
            std::string_view parsing = (*it);
            for(char c : parsing) {
                for(auto &argdef : arguments.arguments) {
                    if(argdef->shortform != c) continue;
                    auto newit = argdef->parse(argv, it);
                    if(newit == std::nullopt) {
                        failfunc(-1);
                        return;
                    }
                    it = newit.value_or(argv.end());
                }
            }
            continue;
        } /*When argument is unmarked, it's an ordered type*/
        for(auto argit = arguments.arguments.begin(); 
                argit != arguments.arguments.end();
                argit++) {
            if((*argit)->shortform != 0 || !(*argit)->longform.empty()) continue;
            it = (*argit)->parse(argv, it).value_or(argv.end());
            arguments.arguments.erase(argit);
            break;
        }
    }
}

}
