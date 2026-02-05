#pragma once
#include "session/session_flow.h"
#include "saves/save_manager.h"
#include "world/world_loader.h"
#include <core/network_manager.h>
#include <godot_cpp/classes/control.hpp>

using namespace godot;

namespace morphic {

class MainMenu : public Control {
  GDCLASS(MainMenu, Control)

protected:
  static void _bind_methods();

public:
  void _ready() override;

private:
  NetworkManager *_net_manager = nullptr;
  SaveManager *_save_manager = nullptr;
  WorldLoader *_world_loader = nullptr;
  SessionFlow _session_flow;

  String _world_scene_path = "res://world/world.tscn";
  NodePath _world_loader_path;
  String _saves_path = "user://saves";
  String _save_name = "world";
  String _join_address = "localhost";
  int _host_port = 7777;
  int _join_port = 7777;
  int _seed = 0;

  void on_host_pressed();
  void on_join_pressed();
  void on_connection_success();
  void on_connection_failed();
  void on_world_loading_finished();
  void on_world_loading_failed(const String &error);

  NodePath get_world_loader_path() const;
  void set_world_loader_path(NodePath p_path);
  String get_world_scene_path() const;
  void set_world_scene_path(String p_path);
  int get_seed() const;
  void set_seed(int p_seed);
  String get_saves_path() const;
  void set_saves_path(const String &p_path);
  String get_save_name() const;
  void set_save_name(const String &p_name);
  String get_join_address() const;
  void set_join_address(const String &p_address);
  int get_host_port() const;
  void set_host_port(int p_port);
  int get_join_port() const;
  void set_join_port(int p_port);
};

} // namespace morphic
