#pragma once

#include <stdbool.h>

struct daklakwl_seat;

enum daklakwl_action {
	DAKLAKWL_ACTION_INVALID,
	DAKLAKWL_ACTION_ENABLE,
	DAKLAKWL_ACTION_DISABLE,
	DAKLAKWL_ACTION_TOGGLE,
	DAKLAKWL_ACTION_DELETE_LEFT,
	DAKLAKWL_ACTION_DELETE_RIGHT,
	DAKLAKWL_ACTION_MOVE_LEFT,
	DAKLAKWL_ACTION_MOVE_RIGHT,
	DAKLAKWL_ACTION_COMPOSE,
	DAKLAKWL_ACTION_ACCEPT,
	DAKLAKWL_ACTION_DISCARD,
	_DAKLAKWL_ACTION_LAST,
};

enum daklakwl_action daklakwl_action_from_string(const char *name);
bool daklakwl_seat_handle_action(struct daklakwl_seat *seat,
				 enum daklakwl_action action);
