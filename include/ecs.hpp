#ifndef ECS_HPP
#define ECS_HPP 1

#include "entitycomponents.hpp"
#include "util.hpp"
#include <bitset>
#include <typeindex>
#include <queue>
#include <cassert>
#include <ranges>
#include <span>
#include <iostream>

namespace ecs {

struct ComponentContainer {
    std::vector<component_variant> packed{};
    std::vector<int> sparse{};
    std::queue<unsigned> free{};

    void resize(size_t n) { sparse.resize(n, -1); }

    template<component_type T>
    void insert(unsigned id, const T &c) {
        assert(sparse.size() > id);
        if(free.empty()) {
            sparse[id] = (int)packed.size();
            packed.push_back(c);
        }else {
            sparse[id] = (int)free.front();
            packed[sparse[id]] = c;
            free.pop();
        }
    }

    void remove(unsigned id) {
        assert(sparse.size() > id);
        if(sparse[id] > -1) {
            free.push((unsigned)sparse[id]);
        }
        sparse[id] = -1;
    }

    template<component_type T>
    T &reduce(unsigned id) {
        assert(sparse.size() > id);
        assert(sparse[id] > -1);
        return std::get<T>(packed[sparse[id]]);
    }
};

class EntityMan;
struct Entity {
    EntityMan *man;
    component_sig sig{};
    unsigned id;

    Entity(const Entity &e) = default;
    Entity(EntityMan *Man, unsigned Id) :
        man(Man), sig(0), id(Id) {}


    template<component_type T>
    constexpr bool contains() { return sig.test(get_index<T, component_variant>()); }

    template<component_type T>
    T &get();

    template<component_type T>
    Entity &addComponent(const T &component);
};

class EntityMan {
    std::array<ComponentContainer, std::variant_size_v<component_variant>> m_components;
    std::vector<Entity> m_entities;
    std::queue<unsigned> m_free;
public:
    Entity &operator[](std::size_t i) { return m_entities[i]; }

    Entity newEntity() {
        std::size_t newid = 0;
        if(!m_free.empty()) {
            newid = m_free.front();
            m_free.pop();
        }else{
            newid = m_entities.size();
            m_entities.emplace_back(this, newid);
            for(auto &container : m_components) container.resize(m_entities.size());
        }
        return m_entities[newid];
    }

    void deleteEntity(Entity &e) {
        for(auto &container : m_components) container.remove(e.id);
        e.sig.reset();
        m_free.push(e.id);
    }

    template<component_type T>
    T &get(const Entity &e) {
        constexpr unsigned cid = get_index<T, component_variant>();
        return m_components[cid].reduce<T>(e.id);
    }

    template<component_type T>
    auto getWith() {
        constexpr unsigned cid = get_index<T, component_variant>();
        auto e_hc = [](Entity const e) { return e.sig[cid]; };
        return std::ranges::views::filter(m_entities, e_hc);
    }

    std::span<Entity> all() {
        return std::span<Entity>(m_entities.begin(), m_entities.end());
    }

    std::size_t size() { return m_entities.size(); }

    template<component_type T>
    Entity &addComponent(Entity &entity, const T& component) {
        constexpr unsigned cid = get_index<T, component_variant>();
        m_components[cid].insert(entity.id, component);
        m_entities[entity.id].sig[cid] = 1;
        return entity;
    }
};

template<component_type T>
T &Entity::get() {
    return man->get<T>(*this);
}

template<component_type T>
Entity &Entity::addComponent(const T& component) {
    return man->addComponent(*this, component);
}

}

#endif
