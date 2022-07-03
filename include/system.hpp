#ifndef SYSTEM_HPP
#define SYSTEM_HPP 1

#include "ecs.hpp"
#include "camera.hpp"
#include "window.hpp"

#include <list>

class System {
private:
    friend class SystemView;
    struct SystemTreeNode {
        int entityId;
        std::list<SystemTreeNode> children;
    };
    SystemTreeNode m_systemTree;
    ecs::EntityMan m_entityMan;

    void addOrbital(const std::string &name, const std::string &orbitingName, unsigned long a, double e, unit::Mass m, unsigned r, double M, double w);
    void tickOrbitals(unit::Time time);

    SystemTreeNode *traverseSystemTree(SystemTreeNode &node, const std::string &name);
    SystemTreeNode *getNode(const std::string &name);
public:
    System(const std::string &name);

    void update();

    ecs::Entity &getBody(std::size_t id);
};

class SystemView {
private:
    System *m_system;
    System::SystemTreeNode *m_focus;

    class Search {
    private:
        struct SystemTreeDisplayNode {
            System::SystemTreeNode *node;
            std::list<SystemTreeDisplayNode> children;
            SystemTreeDisplayNode *parent;

            unsigned index;
            bool collapsed;
            bool hidden;
        };
        SystemView *m_systemView;
        SystemTreeDisplayNode m_displayTree;
        std::vector<SystemTreeDisplayNode*> m_displayTreeFlat;

        unsigned m_selectionIndex;
        std::string m_query;
        bool m_dirty;

        void addNodeToTree(SystemTreeDisplayNode &root, System::SystemTreeNode *node);
        void drawNode(SystemTreeDisplayNode &root, Window &searchWindow, unsigned indent);
        void rebuild();
    public:
        Search(SystemView *systemView);

        void finish();
        void keypress(int key);
        void draw();
    };
    std::unique_ptr<Search> m_focusSearch;
public:
    SystemView(System *system) : m_system(system), m_focusSearch(nullptr) {}

    void keypress(Camera *camera, int key);
    void update(Camera *camera);
    void draw(Camera *camera);
    void drawOver(Camera *camera);

    void view(System *system);

    ecs::Entity &getBody(int id) const;
    int getBodyIdByName(const std::string &name);
};

#endif
