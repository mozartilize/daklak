#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "actions.h"
#include "daklakwl.h"

static bool daklakwl_seat_handle_enable(struct daklakwl_seat *seat)
{
	seat->is_composing = true;
	return true;
}

static bool daklakwl_seat_handle_disable(struct daklakwl_seat *seat)
{
	seat->is_composing = false;
	daklakwl_buffer_clear(&seat->buffer);
	daklakwl_seat_composing_update(seat);
	return true;
}

static bool daklakwl_seat_handle_toggle(struct daklakwl_seat *seat)
{
	if (seat->is_composing)
		return daklakwl_seat_handle_disable(seat);
	else
		return daklakwl_seat_handle_enable(seat);
}

static bool daklakwl_seat_handle_delete_left(struct daklakwl_seat *seat)
{
	if (!seat->is_composing)
		return true;
	if (seat->buffer.len == 0)
		return true;
	daklakwl_buffer_delete_backwards_all(&seat->buffer, 1);
	daklakwl_seat_composing_update(seat);
	if (seat->buffer.len == 0)
		daklakwl_seat_composing_commit(seat);
	return true;
}

static bool daklakwl_seat_handle_delete_right(struct daklakwl_seat *seat)
{
	if (!seat->is_composing)
		return true;
	if (seat->buffer.len == 0)
		return true;
	daklakwl_buffer_delete_forwards_all(&seat->buffer, 1);
	daklakwl_seat_composing_update(seat);
	if (seat->buffer.len == 0)
		daklakwl_seat_composing_commit(seat);
	return true;
}

static bool daklakwl_seat_handle_move_left(struct daklakwl_seat *seat)
{
	if (!seat->is_composing)
		return true;
	if (seat->buffer.len == 0)
		return true;
	daklakwl_buffer_move_left(&seat->buffer);
	daklakwl_seat_composing_update(seat);
	return true;
}

static bool daklakwl_seat_handle_move_right(struct daklakwl_seat *seat)
{
	if (!seat->is_composing)
		return true;
	if (seat->buffer.len == 0)
		return true;
	daklakwl_buffer_move_right(&seat->buffer);
	daklakwl_seat_composing_update(seat);
	return true;
}

static bool daklakwl_seat_handle_compose(struct daklakwl_seat *seat)
{
	if (!seat->is_composing)
		return daklakwl_seat_handle_enable(seat);
	daklakwl_seat_composing_update(seat);
	return true;
}

static bool daklakwl_seat_handle_accept(struct daklakwl_seat *seat)
{
	if (!seat->is_composing)
		return true;
	if (seat->buffer.len == 0)
		return true;
	daklakwl_seat_composing_commit(seat);
	return true;
}

static bool daklakwl_seat_handle_discard(struct daklakwl_seat *seat)
{
	if (!seat->is_composing)
		return true;
	daklakwl_buffer_clear(&seat->buffer);
	daklakwl_seat_composing_commit(seat);
	return true;
}

enum daklakwl_action daklakwl_action_from_string(const char *name)
{
	if (strcmp(name, "enable") == 0)
		return DAKLAKWL_ACTION_ENABLE;
	if (strcmp(name, "disable") == 0)
		return DAKLAKWL_ACTION_DISABLE;
	if (strcmp(name, "toggle") == 0)
		return DAKLAKWL_ACTION_TOGGLE;
	if (strcmp(name, "delete-left") == 0)
		return DAKLAKWL_ACTION_DELETE_LEFT;
	if (strcmp(name, "delete-right") == 0)
		return DAKLAKWL_ACTION_DELETE_RIGHT;
	if (strcmp(name, "move-left") == 0)
		return DAKLAKWL_ACTION_MOVE_LEFT;
	if (strcmp(name, "move-right") == 0)
		return DAKLAKWL_ACTION_MOVE_RIGHT;
	if (strcmp(name, "compose") == 0)
		return DAKLAKWL_ACTION_COMPOSE;
	if (strcmp(name, "accept") == 0)
		return DAKLAKWL_ACTION_ACCEPT;
	if (strcmp(name, "discard") == 0)
		return DAKLAKWL_ACTION_DISCARD;
	return DAKLAKWL_ACTION_INVALID;
}

static bool (*daklakwl_seat_action_handlers[_DAKLAKWL_ACTION_LAST])(
    struct daklakwl_seat *)
    = {
	[DAKLAKWL_ACTION_ENABLE] = daklakwl_seat_handle_enable,
	[DAKLAKWL_ACTION_DISABLE] = daklakwl_seat_handle_disable,
	[DAKLAKWL_ACTION_TOGGLE] = daklakwl_seat_handle_toggle,
	[DAKLAKWL_ACTION_DELETE_LEFT] = daklakwl_seat_handle_delete_left,
	[DAKLAKWL_ACTION_DELETE_RIGHT] = daklakwl_seat_handle_delete_right,
	[DAKLAKWL_ACTION_MOVE_LEFT] = daklakwl_seat_handle_move_left,
	[DAKLAKWL_ACTION_MOVE_RIGHT] = daklakwl_seat_handle_move_right,
	[DAKLAKWL_ACTION_COMPOSE] = daklakwl_seat_handle_compose,
	[DAKLAKWL_ACTION_ACCEPT] = daklakwl_seat_handle_accept,
	[DAKLAKWL_ACTION_DISCARD] = daklakwl_seat_handle_discard,
};

bool daklakwl_seat_handle_action(struct daklakwl_seat *seat,
				 enum daklakwl_action action)
{
	if (action <= DAKLAKWL_ACTION_INVALID
	    || action >= _DAKLAKWL_ACTION_LAST)
		return false;
	bool (*handler)(struct daklakwl_seat *)
	    = daklakwl_seat_action_handlers[action];
	if (!handler)
		return false;
	return handler(seat);
}
