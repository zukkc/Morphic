#include "main_menu.h"
#include "utils/bind_methods.h"
#include "utils/debug_utils.h"
#include "utils/network_utils.h"

#include "godot_cpp/classes/engine.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>

using namespace godot;

namespace morphic {

void MainMenu::_ready() {
  if (Engine::get_singleton()->is_editor_hint())
    return;

  _world_loader = get_node<WorldLoader>(_world_loader_path);
  ERR_FAIL_COND_MSG(
      !_world_loader,
      "Cant get WorldLoader in main menu. Check if path is correct");

  _net_manager = NetUtils::get_net_manager(this);
  ERR_FAIL_COND_MSG(!_net_manager, "Cant get NetworkManager in main_menu");
  _save_manager =
      cast_to<SaveManager>(get_node_or_null("/root/GlobalSaveManager"));
  ERR_FAIL_COND_MSG(!_save_manager,
                    "Cant get SaveManager with path: /root/GlobalSaveManager");
  ERR_FAIL_COND_MSG(
      !_session_flow.configure(_net_manager, _world_loader, _save_manager),
      "MainMenu: failed configuring SessionFlow");

  Error err = _net_manager->connect("connection_success",
                                    Callable(this, "on_connection_success"));
  if (err != OK) {
    ERR_PRINT(
        DebugUtils::format_log("connect connection_success failed: %d", err));
  }
  err = _net_manager->connect("connection_failed",
                              Callable(this, "on_connection_failed"));
  if (err != OK) {
    ERR_PRINT(
        DebugUtils::format_log("connect connection_failed failed: %d", err));
  }

  err = _world_loader->connect("loading_finished",
                               Callable(this, "on_world_loading_finished"));
  if (err != OK) {
    ERR_PRINT(
        DebugUtils::format_log("connect loading_finished failed: %d", err));
  }

  err = _world_loader->connect("loading_failed",
                               Callable(this, "on_world_loading_failed"));
  if (err != OK) {
    ERR_PRINT(DebugUtils::format_log("connect loading_failed failed: %d", err));
  }
}

void MainMenu::on_host_pressed() {
  _session_flow.request_host(_world_scene_path, _host_port, _saves_path,
                             _save_name, _seed);
}

void MainMenu::on_join_pressed() {
  String expected_world_id = _saves_path;
  if (!_saves_path.is_empty() && !_save_name.is_empty()) {
    expected_world_id = _saves_path.path_join(_save_name);
  } else if (expected_world_id.is_empty() && !_save_name.is_empty()) {
    expected_world_id = _save_name;
  }

  _session_flow.request_join(_world_scene_path, _join_address, _join_port,
                             expected_world_id);
}

void MainMenu::on_connection_success() { _session_flow.on_connection_success(); }

void MainMenu::on_connection_failed() { _session_flow.on_connection_failed(); }

void MainMenu::on_world_loading_finished() {
  _session_flow.on_world_loading_finished();
}

void MainMenu::on_world_loading_failed(const String &error) {
  _session_flow.on_world_loading_failed(error);
}

String MainMenu::get_world_scene_path() const { return _world_scene_path; }
void MainMenu::set_world_scene_path(String p_path) {
  _world_scene_path = p_path;
}

NodePath MainMenu::get_world_loader_path() const { return _world_loader_path; }
void MainMenu::set_world_loader_path(NodePath p_path) {
  _world_loader_path = p_path;
}

int MainMenu::get_seed() const { return _seed; }
void MainMenu::set_seed(int p_seed) { _seed = p_seed; }

String MainMenu::get_saves_path() const { return _saves_path; }
void MainMenu::set_saves_path(const String &p_path) { _saves_path = p_path; }

String MainMenu::get_save_name() const { return _save_name; }
void MainMenu::set_save_name(const String &p_name) { _save_name = p_name; }

String MainMenu::get_join_address() const { return _join_address; }
void MainMenu::set_join_address(const String &p_address) {
  _join_address = p_address;
}

int MainMenu::get_host_port() const { return _host_port; }
void MainMenu::set_host_port(int p_port) { _host_port = p_port; }

int MainMenu::get_join_port() const { return _join_port; }
void MainMenu::set_join_port(int p_port) { _join_port = p_port; }

void MainMenu::_bind_methods() {
  ClassDB::bind_method(D_METHOD("on_host_pressed"), &MainMenu::on_host_pressed);
  ClassDB::bind_method(D_METHOD("on_join_pressed"), &MainMenu::on_join_pressed);

  ClassDB::bind_method(D_METHOD("on_connection_success"),
                       &MainMenu::on_connection_success);
  ClassDB::bind_method(D_METHOD("on_connection_failed"),
                       &MainMenu::on_connection_failed);
  ClassDB::bind_method(D_METHOD("on_world_loading_finished"),
                       &MainMenu::on_world_loading_finished);
  ClassDB::bind_method(D_METHOD("on_world_loading_failed", "error"),
                       &MainMenu::on_world_loading_failed);

  BIND_PROPERTY(MainMenu, Variant::STRING, "world_scene_path",
                world_scene_path);
  BIND_PROPERTY(MainMenu, Variant::NODE_PATH, "world_loader_path",
                world_loader_path);
  BIND_PROPERTY(MainMenu, Variant::STRING, "saves_path", saves_path);
  BIND_PROPERTY(MainMenu, Variant::STRING, "save_name", save_name);
  BIND_PROPERTY(MainMenu, Variant::STRING, "join_address", join_address);
  BIND_PROPERTY(MainMenu, Variant::INT, "host_port", host_port);
  BIND_PROPERTY(MainMenu, Variant::INT, "join_port", join_port);
  BIND_PROPERTY(MainMenu, Variant::INT, "seed", seed);
}

} // namespace morphic
