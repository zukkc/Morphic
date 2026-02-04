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

  _net_manager = NetUtils::get_net_manager(this);
  ERR_FAIL_COND_MSG(!_net_manager, "Cant get NetworkManager in main_menu");
  _world_loader =
      cast_to<WorldLoader>(get_node_or_null("/root/GlobalWorldLoader"));
  ERR_FAIL_COND_MSG(!_world_loader,
                    "Cant get WorldLoader with path: /root/GlobalWorldLoader");

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
}

void MainMenu::on_host_pressed() {
  ERR_FAIL_COND_MSG(!_net_manager, "NetworkManager not found");
  ERR_FAIL_COND_MSG(!_world_loader, "WorldLoader not found");

  const bool result = _net_manager->start_host(_host_port);
  if (!result) {
    ERR_PRINT("Failed starting host");
    return;
  }

  String save_path = _saves_path;
  if (!_saves_path.is_empty() && !_save_name.is_empty()) {
    save_path = _saves_path.path_join(_save_name);
  } else if (save_path.is_empty() && !_save_name.is_empty()) {
    save_path = _save_name;
  }

  ERR_FAIL_COND_MSG(save_path.is_empty(),
                    "Save path is empty. Set saves_path/save_name.");
  _world_loader->load_game(save_path, true, true, _seed);
}

void MainMenu::on_join_pressed() {
  ERR_FAIL_COND_MSG(!_net_manager, "NetworkManager not found");
  const bool result = _net_manager->start_client(_join_address, _join_port);
  if (!result) {
    ERR_PRINT("Failed connecting to server");
  }
}

void MainMenu::on_connection_success() {
  if (!_world_loader) {
    ERR_PRINT("WorldLoader not found");
    return;
  }
  _world_loader->load_game("", false);
}

void MainMenu::on_connection_failed() { WARN_PRINT("Connection failed"); }

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

  BIND_PROPERTY(MainMenu, Variant::STRING, "saves_path", saves_path);
  BIND_PROPERTY(MainMenu, Variant::STRING, "save_name", save_name);
  BIND_PROPERTY(MainMenu, Variant::STRING, "join_address", join_address);
  BIND_PROPERTY(MainMenu, Variant::INT, "host_port", host_port);
  BIND_PROPERTY(MainMenu, Variant::INT, "join_port", join_port);
  BIND_PROPERTY(MainMenu, Variant::INT, "seed", seed);
}

} // namespace morphic
