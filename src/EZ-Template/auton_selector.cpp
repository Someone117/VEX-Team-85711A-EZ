/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "EZ-Template/auton_selector.hpp"

#include "api.h"
#include "lemlib/api.hpp"

AutonSelector auton_selector;

AutonSelector::AutonSelector() {
  auton_count = 0;
  current_auton_page = 0;
  Autons = {};
}

AutonSelector::AutonSelector(std::vector<Auton> autons) {
  auton_count = autons.size();
  current_auton_page = 0;
  Autons = {};
  Autons.assign(autons.begin(), autons.end());
}

void AutonSelector::print_selected_auton() {
  if (auton_count == 0) return;
  for (int i = 0; i < 8; i++)
    pros::lcd::clear_line(i);
  lemlib::print_to_screen("Page " + std::to_string(current_auton_page + 1) + "\n" + Autons[current_auton_page].Name);
}

void AutonSelector::call_selected_auton() {
  if (auton_count == 0) return;
  Autons[current_auton_page].auton_call();
}

void AutonSelector::add_autons(std::vector<Auton> autons) {
  auton_count += autons.size();
  current_auton_page = 0;
  Autons.assign(autons.begin(), autons.end());
}

void AutonSelector::init_auton_selector() {
  if (auton_selector.current_auton_page > auton_selector.auton_count - 1 || auton_selector.current_auton_page < 0) {
    auton_selector.current_auton_page = 0;
  }
}

void page_up() {
  if (auton_selector.current_auton_page == auton_selector.auton_count - 1)
    auton_selector.current_auton_page = 0;
  else
    auton_selector.current_auton_page++;
  auton_selector.print_selected_auton();
}

void page_down() {
  if (auton_selector.current_auton_page == 0)
    auton_selector.current_auton_page = auton_selector.auton_count - 1;
  else
    auton_selector.current_auton_page--;
  auton_selector.print_selected_auton();
}

void AutonSelector::initialize() {
  // Initialize auto selector and LLEMU
  pros::lcd::initialize();
  init_auton_selector();

  // Callbacks for auto selector
  auton_selector.print_selected_auton();
  pros::lcd::register_btn0_cb(page_down);
  pros::lcd::register_btn2_cb(page_up);
}