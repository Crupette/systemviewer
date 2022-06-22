#include "keybind.hpp"
#include "csv.hpp"
#include "input.hpp"

#include <algorithm>

std::unordered_map<std::string, KeyMan::Bind> KeyMan::m_keybinds;
std::unordered_map<std::string, std::string> KeyMan::m_keybindContexts;

KeyMan::Binds KeyMan::binds;

static std::unordered_map<int, std::string> CODENAMES = 
{
    { input::CTRL_KEY_ARROWUP, "Up arrow" },
    { input::CTRL_KEY_ARROWDOWN, "Down arrow" },
    { input::CTRL_KEY_ARROWRIGHT, "Right arrow" },
    { input::CTRL_KEY_ARROWLEFT, "Left arrow" }
};

void
KeyMan::registerBind(int def,
        const std::string &name,
        const std::string &context,
        const std::string &desc)
{
    auto find = m_keybinds.find(name);
    if(find != m_keybinds.end()) {
        find->second.ctx = context;
        find->second.desc = desc;
    }else{
    	Bind bind = { def, name, context, desc};
        m_keybinds[name] = bind;
    }
    m_keybindContexts[name] = context;
}

void
KeyMan::loadKeybindsFrom(const std::string &csvPath)
{
    csv::CSVFile<',', int, std::string> keybindData(csvPath);
    for(auto &bind : keybindData.get()) {
        int code = std::get<0>(bind);
        std::string name = std::get<1>(bind);

        m_keybinds[name] = { .code = code, .name = name, .ctx = "", .desc = ""};
    }
}

void 
KeyMan::writeKeybindsTo(const std::string &csvPath)
{
    csv::CSVFile<',', int, std::string> keybindData(csvPath, true);
    for(Bind &bind : KeyMan::binds()) {
        keybindData.put({bind.code, bind.name});
    }
    keybindData.write();
}

std::string 
KeyMan::translateCode(int code)
{
    if(code < 256) return std::string(1, (char)code);
    return CODENAMES[code];
}
