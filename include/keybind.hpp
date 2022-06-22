#ifndef KEYBIND_HPP
#define KEYBIND_HPP 1

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

#define CTX_GLOBAL "Global"
#define CTX_SYSTEMVIEW "System View"
#define CTX_TIMEMAN "Time Manager"

#define BIND_G_QUIT "G_Quit"
#define BIND_G_NEXTWIN "G_NextWindow"
#define BIND_G_PREVWIN "G_PrevWindow"
#define BIND_G_EDITBINDS "G_EditBinds"
#define BIND_G_ESCAPE "G_Escape"
#define BIND_G_SELECT "G_Select"

#define BIND_SYSTEMVIEW_PANUP "Systemview_PanUp"
#define BIND_SYSTEMVIEW_PANDOWN "Systemview_PanDown"
#define BIND_SYSTEMVIEW_PANLEFT "Systemview_PanLeft"
#define BIND_SYSTEMVIEW_PANRIGHT "Systemview_PanRight"
#define BIND_SYSTEMVIEW_DECSCALE "Systemview_DecScale"
#define BIND_SYSTEMVIEW_INCSCALE "Systemview_IncScale"
#define BIND_SYSTEMVIEW_SEARCH "Systemview_Search"

#define BIND_SYSTEMVIEW_SEARCH_PREV "Systemview_Search_Prev"
#define BIND_SYSTEMVIEW_SEARCH_NEXT "Systemview_Search_Next"
#define BIND_SYSTEMVIEW_SEARCH_TOP "Systemview_Search_Top"
#define BIND_SYSTEMVIEW_SEARCH_BOTTOM "Systemview_Search_Bottom"
#define BIND_SYSTEMVIEW_SEARCH_COLLAPSE "Systemview_Search_Collapse"

#define BIND_TIMEMAN_STEP "Timeman_Step"
#define BIND_TIMEMAN_INCSTEP "Timeman_IncStep"
#define BIND_TIMEMAN_DECSTEP "Timeman_DecStep"
#define BIND_TIMEMAN_TOGGLEAUTO "Timeman_ToggleAuto"

class KeyMan {
public:
    struct Bind {
        int code;
        std::string name;
        std::string ctx;
        std::string desc;
    };

    struct Binds {
        std::vector<Bind> operator()() {
            std::vector<Bind> bindList;
            std::transform(
                    KeyMan::m_keybinds.begin(), 
                    KeyMan::m_keybinds.end(), 
                    std::back_inserter(bindList), [](auto &pair){return pair.second;});
            return bindList;
        }
        Bind &operator[](const std::string &name) { return KeyMan::m_keybinds[name]; }
    };
    static Binds binds;

    static void registerBind(int def, const std::string &name, const std::string &context, const std::string &desc);
    static void loadKeybindsFrom(const std::string &csvPath);
    static void writeKeybindsTo(const std::string &csvPath);
    static std::string translateCode(int code);

private:
    static std::unordered_map<std::string, Bind> m_keybinds;
    static std::unordered_map<std::string, std::string> m_keybindContexts;
};

#endif
