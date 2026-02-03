#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/variant.hpp>

// Helper for binding simple properties to the inspector with standard get/set.
#define BIND_PROPERTY(CLASS, TYPE, PROP_NAME, METHOD_BASE)                     \
  do {                                                                         \
    godot::ClassDB::bind_method(godot::D_METHOD("get_" #METHOD_BASE),          \
                                &CLASS::get_##METHOD_BASE);                    \
    godot::ClassDB::bind_method(                                               \
        godot::D_METHOD("set_" #METHOD_BASE, "p_" #METHOD_BASE),               \
        &CLASS::set_##METHOD_BASE);                                            \
    ADD_PROPERTY(godot::PropertyInfo(TYPE, PROP_NAME), "set_" #METHOD_BASE,    \
                 "get_" #METHOD_BASE);                                         \
  } while (0)

// Same as BIND_PROPERTY but allows setting a hint and hint string.
#define BIND_PROPERTY_HINT(CLASS, TYPE, PROP_NAME, METHOD_BASE, HINT)          \
  do {                                                                         \
    godot::ClassDB::bind_method(godot::D_METHOD("get_" #METHOD_BASE),          \
                                &CLASS::get_##METHOD_BASE);                    \
    godot::ClassDB::bind_method(                                               \
        godot::D_METHOD("set_" #METHOD_BASE, "p_" #METHOD_BASE),               \
        &CLASS::set_##METHOD_BASE);                                            \
    ADD_PROPERTY(godot::PropertyInfo(TYPE, PROP_NAME, HINT),                   \
                 "set_" #METHOD_BASE, "get_" #METHOD_BASE);                    \
  } while (0)
