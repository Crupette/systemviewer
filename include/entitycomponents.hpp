#ifndef ENTITY_COMPONENT_HPP
#define ENTITY_COMPONENT_HPP 1

#include "vex.hpp"
#include "units.hpp"
#include <bitset>

namespace ecs {
struct PositionComponent {
    vex::vec2<long> position{};
};

struct VelocityComponent {
    vex::vec2<long> velocity{};
};

struct MassComponent {
    unit::Mass mass;
};

struct NameComponent {
    std::string name;
};

struct OrbitalComponent {
    unsigned origin;
    long a;
    double e;
    double w;
    double M;
    double T;
    double v;
};

struct RenderCircleComponent {
    unsigned radius;
};

using component_variant = std::variant<
    std::monostate,
    PositionComponent,
    VelocityComponent,
    MassComponent,
    NameComponent,
    OrbitalComponent,
    RenderCircleComponent
>;

using component_sig = 
    std::bitset<std::variant_size_v<component_variant>>; 

template<typename T>
concept component_type =
    is_variant_v<T, component_variant>;

}

#endif
