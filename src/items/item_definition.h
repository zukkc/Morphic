#pragma once

#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

namespace morphic {

class ItemDefinition : public Resource {
  GDCLASS(ItemDefinition, Resource)

public:
  enum Hand { LEFT = 0, RIGHT = 1, BOTH = 2 };

protected:
  static void _bind_methods();

public:
  void set_id(int p_id);
  int get_id() const;

  void set_name(String p_name);
  String get_name() const;

  void set_hand(int p_hand);
  int get_hand() const;

  void set_equip_state(String p_state);
  String get_equip_state() const;

  void set_model_scene(const Ref<PackedScene> &p_scene);
  Ref<PackedScene> get_model_scene() const;

  void set_hand_playback(String p_name);
  String get_hand_playback() const;

  void set_actions(Array p_actions);
  Array get_actions() const;
  void set_hand_action(String action, String state);
  String get_hand_action(String action) const;
  void set_full_body_action(String action, String state);
  String get_full_body_action(String action) const;

private:
  int _id = 0;
  String _name;
  int _hand = RIGHT;
  String _equip_state;
  Ref<PackedScene> _model_scene;
  String _hand_playback;
  Array _actions;
};

} // namespace morphic

VARIANT_ENUM_CAST(morphic::ItemDefinition::Hand);
