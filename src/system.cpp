#include "system.hpp"
#include "timeman.hpp"
#include "ecs.hpp"
#include "vex.hpp"
#include "units.hpp"
#include "csv.hpp"
#include "keybind.hpp"
#include "game.hpp"
#include <numbers>
#include <string>

static double G = 6.6743 * std::pow(10, -11);

ecs::Entity &
System::addOrbital(const std::string &name, 
           const std::string &orbitingName, 
           unsigned long a, 
           double e,
           unit::Mass m,
           unsigned r,
           double M,
           double w)
{
    M *= (std::numbers::pi / 180.0);
    w *= (std::numbers::pi / 180.0);

    SystemTreeNode *treeNode = getNode(orbitingName);
    ecs::Entity &newOrbital = m_entityMan.newEntity()
        .addComponent(ecs::PositionComponent{vex::vec2<long>{0, 0}})
        .addComponent(ecs::MassComponent{m})
        .addComponent(ecs::OrbitalComponent{.origin = treeNode->entityId, .a = (long)a, .e = e, .w = w, .M = M, .T = 0, .v = 0})
        .addComponent(ecs::RenderCircleComponent{r})
        .addComponent(ecs::NameComponent{name});
    treeNode->children.push_back({newOrbital.id, {}});
    return newOrbital;
}

System::System()
{
    auto sol = m_entityMan.newEntity()
        .addComponent(ecs::PositionComponent{vex::vec2<long>{0, 0}})
        .addComponent(ecs::MassComponent{unit::solMass})
        .addComponent(ecs::RenderCircleComponent{695700})
        .addComponent(ecs::NameComponent{"Sol"});
    m_systemTree.entityId = sol.id;

    csv::CSVFile<',', std::string, std::string, double, double, double, double, double, double> planetData("data/sol_planets.csv");
    for(auto &planet : planetData.get()) {
        std::string name = std::get<0>(planet);
        std::string orbiting = std::get<1>(planet);
        double sma = std::get<2>(planet) * unit::AU;
        double e = std::get<3>(planet);
        unit::Mass m = unit::earthMass * std::get<4>(planet);
        double r = std::get<5>(planet) * unit::earthRad;
        double M = std::get<6>(planet);
        double w = std::get<7>(planet);
        addOrbital(name, orbiting, sma, e, m, r, M, w);
    }

    csv::CSVFile<',', std::string, std::string, double, double, double, double, double, double> satelliteData("data/sol_satellites.csv");
    for(auto &satellite : satelliteData.get()) {
        std::string name = std::get<0>(satellite);
        std::string orbiting = std::get<1>(satellite);
        double sma = std::get<2>(satellite) * unit::AU;
        double e = std::get<3>(satellite);
        unit::Mass m = unit::earthMass * std::get<4>(satellite);
        double r = std::get<5>(satellite) * unit::earthRad;
        double M = std::get<6>(satellite);
        double w = std::get<7>(satellite);
        addOrbital(name, orbiting, sma, e, m, r, M, w);
    }

    csv::CSVFile<',', std::string, double, double, double, double, double, std::string> asteroidData("data/sol_asteroids.csv");
    for(auto &asteroid : asteroidData.get()) {
        std::string name = std::get<0>(asteroid);
        if(name == "Missing") name = std::get<6>(asteroid);
        addOrbital(name, 
                   "Sol", 
                   (unsigned long)(std::get<1>(asteroid) * unit::AU),
                   std::get<2>(asteroid),
                   unit::Mass(0),
                   (unsigned)std::get<3>(asteroid),
                   std::get<4>(asteroid),
                   std::get<5>(asteroid));
    }
}

constexpr static double tau = std::numbers::pi * 2;

void
System::tickOrbitals(unit::Time time)
{
    for(ecs::Entity &e : m_entityMan.getWith<ecs::OrbitalComponent>()) {
        auto &oc = e.get<ecs::OrbitalComponent>();
        auto &pc = e.get<ecs::PositionComponent>();
        auto &mc = e.get<ecs::MassComponent>();

        ecs::Entity &o = m_entityMan[oc.origin];
        auto &opc = o.get<ecs::PositionComponent>();
        auto &om = o.get<ecs::MassComponent>();

        double e2 = oc.e * oc.e;
        double td = (double)time();
        if(oc.T == 0) {
            double u = G * (om.mass() + mc.mass());
            double am = (double)oc.a * 1000.0;
            oc.T = tau * std::sqrt((am * am * am) / u);
        }
        double n = tau / oc.T;
        double M = oc.M + (n * td);
        double E = M;

        int its = 0;
        while(true) {
            double dE = (E - oc.e * std::sin(E) - M) / (1 - oc.e * std::cos(E));
            E -= dE;
            its++;
            if(std::abs(dE) < 1e-6) break;
        }

        double x = std::cos(E) - oc.e;
        double y = std::sin(E) * std::sqrt(1 - e2);

        double v = std::atan2(y, x) + oc.w;
        oc.v = v;

        double r = std::sqrt(x*x + y*y) * (double)oc.a;

        vex::vec2<double> polar(r, v);
        pc.position = vex::cartesian<long>(polar) + opc.position; 
    }

}

void
System::update() 
{
    tickOrbitals(TimeMan::time());
}

ecs::Entity &
System::getBody(std::size_t id) {
    return m_entityMan[id];
}

System::SystemTreeNode *
System::traverseSystemTree(SystemTreeNode &node, const std::string &name)
{
    SystemTreeNode *found = nullptr;
    ecs::Entity &entity = getBody(node.entityId);
    auto &namecomp = entity.get<ecs::NameComponent>();
    if(namecomp.name == name) found = &node;

    for(SystemTreeNode &child : node.children) {
        if(found != nullptr) break;
        found = traverseSystemTree(child, name);
    }
    return found;
}

System::SystemTreeNode *
System::getNode(const std::string &name)
{
    SystemTreeNode *treeRes = traverseSystemTree(m_systemTree, name);
    if(treeRes == nullptr)
        treeRes = &m_systemTree;
    return treeRes;
}

void 
SystemView::view(System *system) 
{ 
    m_system = system;
    m_focus = &system->m_systemTree;
}

void
SystemView::keypress(Camera *camera, int key)
{
    WindowContext &context = Game::contexts();
    if(context.getFocusedString() == WINDOW_SYSTEMVIEW_SEARCH_ID) {
        m_focusSearch->keypress(key);
        if(!Game::paused()) {
            Search *fp = m_focusSearch.release();
            delete fp;
        }
    }
    if(context.getFocusedString() != WINDOW_SYSTEMVIEW_ID) return;
    if(key == KeyMan::binds[BIND_SYSTEMVIEW_PANUP].code) camera->move(0, camera->getscale());
    if(key == KeyMan::binds[BIND_SYSTEMVIEW_PANDOWN].code) camera->move(0, -(long)camera->getscale());
    if(key == KeyMan::binds[BIND_SYSTEMVIEW_PANRIGHT].code) camera->move(camera->getscale(), 0);
    if(key == KeyMan::binds[BIND_SYSTEMVIEW_PANLEFT].code) camera->move(-(long)camera->getscale(), 0);
    if(key == KeyMan::binds[BIND_SYSTEMVIEW_INCSCALE].code) camera->setscale(camera->getscale() * 2);
    if(key == KeyMan::binds[BIND_SYSTEMVIEW_DECSCALE].code) camera->setscale(camera->getscale() / 2);
    if(key == KeyMan::binds[BIND_SYSTEMVIEW_SEARCH].code) {
        Game::setState(Game::State::PAUSED_INPUT);
        context.focus(WINDOW_SYSTEMVIEW_SEARCH_ID);
        context.setWindowHidden(WINDOW_SYSTEMVIEW_SEARCH_ID, false);
        m_focusSearch = std::make_unique<Search>(this);
    }
}

void 
SystemView::update(Camera *camera)
{
    if(Game::paused()) return;
    auto &efoc = m_system->m_entityMan[m_focus->entityId];
    auto &efocp = efoc.get<ecs::PositionComponent>();
    if(efocp.position != camera->getorigin()) {
        camera->setorigin(efocp.position);
    }
}

void
SystemView::drawOver(Camera *camera) {
    auto &efoc = m_system->m_entityMan[m_focus->entityId];
    auto &efocp = efoc.get<ecs::PositionComponent>();
    auto &efocm = efoc.get<ecs::MassComponent>();

    WindowContext &context = Game::contexts();
    Window &infoWindow = context[WINDOW_BODYINFO_ID];
    Window &viewWindow = context[WINDOW_SYSTEMVIEW_ID];

    infoWindow << straw::clear(' ');
    infoWindow << straw::move(0, 0) << "Focus: " << efoc.get<ecs::NameComponent>().name << '\n';
    if(efoc.contains<ecs::OrbitalComponent>()) {
        auto &efoco = efoc.get<ecs::OrbitalComponent>();
        ecs::Entity &efoc_origin = m_system->m_entityMan[efoco.origin];

        infoWindow << "Orbiting: " << efoc_origin.get<ecs::NameComponent>().name << '\n';
        infoWindow << "Distance: " << std::abs((double)(efocp.position - efoc_origin.get<ecs::PositionComponent>().position).magnitude()) << "km\n";
        infoWindow << "Period: " << efoco.T / unit::DAY_SECONDS << " days\n";
        infoWindow << "Angle: " << efoco.v * (180.0 / std::numbers::pi) << '\n';
        infoWindow << "Eccentricity: " << efoco.e << '\n';
        infoWindow << "Mass: " << efocm.mass() << '\n';
    }
    vex::vec2<unsigned> viewdims(
                viewWindow.screen()->getwidth(),
                viewWindow.screen()->getheight());

    viewWindow << straw::move(0, 0) << 
                  straw::setcolor(straw::WHITE, straw::BLACK) << 
                  "Press '" << 
                  KeyMan::translateCode(KeyMan::binds[BIND_SYSTEMVIEW_SEARCH].code) << 
                  "' to change focus";

    double scale = camera->getscale() * (viewdims[0] / 2.0);
    if(scale > 1e7) {
        viewWindow << straw::move(0, viewdims[1] - 1) << scale / unit::AU << " AU";
    }else{
        viewWindow << straw::move(0, viewdims[1] - 1) << scale << " km";
    }

}

void
SystemView::draw(Camera *camera)
{
    if(m_focusSearch != nullptr) m_focusSearch->draw();
    if(!camera->dirty()) return;
    auto &efoc = m_system->m_entityMan[m_focus->entityId];
    auto &efocp = efoc.get<ecs::PositionComponent>();
    auto &efocm = efoc.get<ecs::MassComponent>();

    if(efoc.contains<ecs::OrbitalComponent>()) {
        auto &oc = efoc.get<ecs::OrbitalComponent>();
        auto &om = efoc.get<ecs::MassComponent>();
        ecs::Entity &origin = m_system->m_entityMan[oc.origin];
        auto pc = origin.get<ecs::PositionComponent>();
    
        std::vector<vex::vec2<long>> points;
        float i = 0.0;
        while(i < tau){
            double e2 = oc.e * oc.e;
            double M = oc.M + i;
            double E = M;
            
            int its = 0;
            while(its < 512) {
                double dE = (E - oc.e * std::sin(E) - M) / (1 - oc.e * std::cos(E));
                E -= dE;
                its++;
                if(std::abs(dE) < 1e-6) break;
            }
            
            double x = (std::cos(E) - oc.e);
            double y = (std::sin(E) * std::sqrt(1 - e2));

            double v = std::atan2(y, x) + oc.w;
            double r = std::sqrt(x*x + y*y) * (double)oc.a;
            

            vex::vec2<double> polar{r, v};
            vex::vec2<long> cart = vex::cartesian<long>(polar) + pc.position;

            points.push_back(cart);
            i += 0.01;
        }
        for(unsigned i = 0; i < points.size(); i++) {
            if(i == 0) {
                camera->batchShape(shapes::line<long>(points[points.size() - 1], points[i]), straw::color(0, 0, 255), '#');
            }else{
                camera->batchShape(shapes::line<long>(points[i-1], points[i]), straw::color(0, 0, 255), '#');
            }
        }
    }
    
    for(ecs::Entity &e : m_system->m_entityMan.getWith<ecs::RenderCircleComponent>()) {
        auto &pc = e.get<ecs::PositionComponent>();
        auto &cc = e.get<ecs::RenderCircleComponent>();
        unsigned id = e.id;
        
        long cr = cc.radius;
        if(cr < camera->getscale()) cr = camera->getscale();
        shapes::ellipse<long> circle(pc.position, cr, cr); 
        
        straw::color color = id == m_focus->entityId ? straw::color{255, 255, 0} : straw::WHITE;
        if(cc.radius < cr) {
            camera->batchShape(circle, color, '*');
        }else{
            camera->batchShape(circle, color, '#');
        }
    }
}

ecs::Entity &
SystemView::getBody(int id) const
{
    return m_system->m_entityMan[id];
}

int
SystemView::getBodyIdByName(const std::string &name)
{
    for(ecs::Entity &entity : m_system->m_entityMan.all()) {
        int id = entity.id;
        if(entity.contains<ecs::NameComponent>()) {
            auto &namecomp = entity.get<ecs::NameComponent>();
            if(namecomp.name == name) return id;
        }
    }
    return -1;
}

void
SystemView::Search::addNodeToTree(SystemTreeDisplayNode &root, System::SystemTreeNode *node)
{
    m_displayTreeFlat.emplace_back(&root);
    for(auto &child : node->children)
    {
        ecs::NameComponent &namecomp = m_systemView->m_system->m_entityMan[child.entityId].get<ecs::NameComponent>();
        root.children.push_back(
                {&child, 
                    {}, 
                    &root, 
                    (unsigned)m_displayTreeFlat.size(), 
                    m_query.empty(), 
                    m_query.empty() ? false : namecomp.name.find(m_query) == std::string::npos});
        addNodeToTree(root.children.back(), &child);
    }
}

void
SystemView::Search::rebuild()
{
    System::SystemTreeNode *systemRoot = &m_systemView->m_system->m_systemTree;
    m_displayTree = {systemRoot, {}, nullptr, 0, false, false};
    m_displayTreeFlat.clear();
    m_selectionIndex = 0;

    addNodeToTree(m_displayTree, systemRoot);
    for(unsigned i = 0; auto *node : m_displayTreeFlat) {
        if(m_systemView->m_focus == node->node) {
            m_selectionIndex = i;
            break;
        }
        i++;
    }
    SystemTreeDisplayNode *recurse = m_displayTreeFlat[m_selectionIndex];
    while(recurse->parent != nullptr) {
        recurse->parent->collapsed = false;
        recurse->parent->hidden = false;
        recurse = recurse->parent;
    }
}

void
SystemView::Search::drawNode(
        SystemTreeDisplayNode &root,
        Window &searchWindow,
        unsigned indent)
{
    unsigned windowH = searchWindow.screen()->getheight();
    unsigned cursorY = searchWindow.screen()->getcursory();
    if(cursorY == windowH && root.index > m_selectionIndex) return;
    
    ecs::NameComponent &namecomp = m_systemView->m_system->m_entityMan[root.node->entityId].get<ecs::NameComponent>();

    if(root.index == m_selectionIndex) {
        searchWindow << straw::setcolor(straw::BLACK, straw::WHITE);
    }else searchWindow << straw::setcolor(straw::WHITE, straw::BLACK);


    if(!root.hidden) {
        if(m_query.empty()) searchWindow << std::string((indent * 4), ' ');
        if(root.children.size() > 0)
            searchWindow << '[' << (root.collapsed ? '+' : '-') << "] ";
        searchWindow << namecomp.name << straw::setcolor(straw::WHITE, straw::BLACK) << '\n';
    }

    if(!root.collapsed) {
        for(auto &child : root.children) drawNode(child, searchWindow, indent + 1);
    }
}

SystemView::Search::Search(SystemView *systemView) :
    m_systemView(systemView), m_selectionIndex(0), m_dirty(true)
{
    rebuild();
}

void
SystemView::Search::finish()
{
    WindowContext &context = Game::contexts();
    Game::setState(Game::State::RUNNING);
    context.setWindowHidden(WINDOW_SYSTEMVIEW_SEARCH_ID, true);
    context.focus(WINDOW_SYSTEMVIEW_ID);
}

void 
SystemView::Search::keypress(int key)
{
    if(key == KeyMan::binds[BIND_G_ESCAPE].code) {
        finish();
    }else
    if(key == KeyMan::binds[BIND_SYSTEMVIEW_SEARCH_NEXT].code) {
        if(m_selectionIndex == m_displayTreeFlat.size() - 1) return;
        for(m_selectionIndex++; m_selectionIndex < m_displayTreeFlat.size() - 1; ++m_selectionIndex) {
            if(!m_displayTreeFlat[m_selectionIndex]->hidden) break;
        }
        if(m_displayTreeFlat[m_selectionIndex]->hidden) m_selectionIndex = 0;
        SystemTreeDisplayNode *node = m_displayTreeFlat[m_selectionIndex];
        while(node->parent != nullptr) {
            if(!node->parent->collapsed) break;
            m_selectionIndex += node->parent->children.size();
            node = node->parent;
        }

        m_dirty = true;
    }else
    if(key == KeyMan::binds[BIND_SYSTEMVIEW_SEARCH_PREV].code) {
        if(m_selectionIndex == 0) return;
        for(m_selectionIndex--; m_selectionIndex > 0; --m_selectionIndex) {
            if(!m_displayTreeFlat[m_selectionIndex]->hidden) break;
        }
        SystemTreeDisplayNode *node = m_displayTreeFlat[m_selectionIndex];
        while(node->parent != nullptr) {
            if(!node->parent->collapsed) break;
            m_selectionIndex -= node->parent->children.size();
            node = node->parent;
        }

        m_dirty = true;
    }else
    if(key == KeyMan::binds[BIND_SYSTEMVIEW_SEARCH_TOP].code) {
        m_selectionIndex = 0;
        for(; m_selectionIndex < m_displayTreeFlat.size(); ++m_selectionIndex) {
            if(!m_displayTreeFlat[m_selectionIndex]->hidden) break;
        }
        m_dirty = true;
    }else
    if(key == KeyMan::binds[BIND_SYSTEMVIEW_SEARCH_BOTTOM].code) {
        m_selectionIndex = m_displayTreeFlat.size() - 1;
        for(; m_selectionIndex > 0; --m_selectionIndex) {
            if(!m_displayTreeFlat[m_selectionIndex]->hidden) break;
        }
        m_dirty = true;
    }else
    if(key == KeyMan::binds[BIND_SYSTEMVIEW_SEARCH_COLLAPSE].code) {
        m_displayTreeFlat[m_selectionIndex]->collapsed = !m_displayTreeFlat[m_selectionIndex]->collapsed;
        m_dirty = true;
    }else
    if(key == KeyMan::binds[BIND_G_SELECT].code) {
        m_systemView->m_focus = m_displayTreeFlat[m_selectionIndex]->node;
        finish();
    }else
    {
        if(key < 0) return;
        switch(key) {
            case 127:
            case '\b': {
                            if(!m_query.empty()) m_query.pop_back(); 
                       } break;
            default: m_query.push_back((char)key); break;
        }
        rebuild();
        m_dirty = true;
    }
}

void
SystemView::Search::draw()
{
    if(!m_dirty) return;
    
    WindowContext &windowContext = Game::contexts();
    Window &searchWindow = windowContext[WINDOW_SYSTEMVIEW_SEARCH_ID];

    searchWindow << straw::clear(' ') << straw::move(0, 0);
    searchWindow << "Query: " << m_query << '\n';
    
    drawNode(m_displayTree, searchWindow, 0);

    searchWindow << straw::flush();
    m_dirty = false;
}
