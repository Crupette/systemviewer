/**
 * diargs.hpp
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
 *
 * diargs - An alternative to GNU getopt for modern C++
 * Provides tools to parse command line arguments for C++17 programs
 * 
 * Usage: Instantiate an ArgumentParser class, passing in a fallback
 *        error function, an ArgumentList struct, and an argc,argv pair.
 *
 * Example:
 *
 * void printusage(int err) {
 *     std::cout << "-h --help : Print this message" << std::endl;
 *     std::exit(err);
 * }
 *
 * int main(int argc, char **argv) {
 *    bool helpflag;
 *
 *    ArgsPair args{argc, argv};
 *    ArgumentList argslist(
 *          ToggleArgument("help", 'h', helpflag, true)
 *    );
 *    ArgumentParser parser(printusage, argslist, args);
 *    return 0;
 * }
 */
#ifndef DIARGS_HPP
#define DIARGS_HPP 1

#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <optional>
#include <algorithm>

namespace diargs {

    /*Passed to ArgumentParser*/
    struct ArgsPair {
        int argc {};
        char **argv {};
    };

    /*Interface for argument types. Makes the whole system tick*/
    struct IArgument
    {
        std::string longform {}; /*Name parsed when passed through with '--'*/
        char shortform {}; /*Character parsed when passed through with '-'*/
        
        IArgument() = default;
        IArgument(const std::string &Longform, char Shortform) : 
            longform(Longform), shortform(Shortform) {}
        virtual ~IArgument() = default;

        virtual std::optional<std::vector<std::string_view>::iterator> parse(
                std::vector<std::string_view> &argv,
                std::vector<std::string_view>::iterator arg) = 0;
    };

    template<typename T>
    struct MultiArgument : public IArgument
    {
        T *flag; /*Value set when encountered. Taken from next argument
                   i.e. --arg [VALUE]*/
        
        MultiArgument(T &Flag) : flag(&Flag) {}
        MultiArgument(const std::string &Longform, T &Flag) :
            IArgument(Longform, 0), flag(&Flag) {}
        MultiArgument(const std::string &Longform, char Shortform, T &Flag) :
            IArgument(Longform, Shortform), flag(&Flag) {}
        ~MultiArgument() {}

        std::optional<std::vector<std::string_view>::iterator> parse(
                std::vector<std::string_view> &argv,
                std::vector<std::string_view>::iterator arg) {
            if(++arg == argv.end()) return std::nullopt; /*Not having the
            value is an error case*/
            std::stringstream ss((*arg).data());
            ss >> std::noskipws >> *flag;
            return arg;
        }
    };

    template<typename T>
    struct ToggleArgument : public IArgument
    {
        T *flag;    /*Value set to [set] when encountered*/
        T set;      /*Value that [flag] is set to when parsed*/

        ToggleArgument(T &Flag, const T Set) : 
            flag(&Flag), set(Set) {}
        ToggleArgument(const std::string &Longform, T &Flag, const T Set) :
            IArgument(Longform, 0), flag(&Flag), set(&Set) {}
        ToggleArgument(const std::string &Longform, char Shortform, 
                       T &Flag, const T Set) :
            IArgument(Longform, Shortform), flag(&Flag), set(&Set) {}
        ~ToggleArgument() {}

        std::optional<std::vector<std::string_view>::iterator> parse(
                std::vector<std::string_view> &argv,
                std::vector<std::string_view>::iterator arg) {
            *flag = set;
            return arg;
        }
    };

    /*Type for values not denoted with '-' or '--'*/
    template<typename T>
    struct OrderedArgument : public IArgument
    {
        T *flag; /*Value set when passed arg is not any other type*/
        std::optional<T> dflt;
        OrderedArgument(T &Flag) :
            flag(&Flag), dflt(std::nullopt) {}
        OrderedArgument(T &Flag, T Dflt) :
            flag(&Flag), dflt(Dflt) {}
        ~OrderedArgument() {}
        std::optional<std::vector<std::string_view>::iterator> parse(
                std::vector<std::string_view> &argv,
                std::vector<std::string_view>::iterator arg) {
            std::stringstream ss((*arg).data());
            ss >> std::noskipws >> *flag;
            return arg;
        }       
    };

    /*Container for argument checking*/
    struct ArgumentList
    {
        std::vector<std::unique_ptr<IArgument>> arguments;

        void addArgument() {}
        template<class T, class... ArgumentPack>
        void addArgument(T arg, ArgumentPack... args) {
            arguments.push_back(std::make_unique<T>(arg));
            addArgument(args...);
        }

        template<class... ArgumentPack>
        ArgumentList(ArgumentPack... arguments) {
            addArgument(arguments...);
        }
    };

    /*Instantiation results in parsed arguments*/
    class ArgumentParser
    {
    public:
        ArgumentParser(
                void (*failfunc)(int),
                ArgumentList &arguments,
                ArgsPair argcv);
    };
}

#endif
