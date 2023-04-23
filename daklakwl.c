#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>

#include <poll.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>

#include <scfg.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include "input-method-unstable-v2-client-protocol.h"
#include "text-input-unstable-v3-client-protocol.h"
#include "virtual-keyboard-unstable-v1-client-protocol.h"

#include "buffer.h"
#include "config.h"
#include "daklakwl.h"
#include "tray.h"

#define min(a, b)                                                              \
	({                                                                     \
		__typeof__(a) _a = (a);                                        \
		__typeof__(b) _b = (b);                                        \
		_a < _b ? _a : _b;                                             \
	})

#define max(a, b)                                                              \
	({                                                                     \
		__typeof__(a) _a = (a);                                        \
		__typeof__(b) _b = (b);                                        \
		_a > _b ? _a : _b;                                             \
	})

void daklakwl_seat_init(struct daklakwl_seat *seat,
			struct daklakwl_state *state, struct wl_seat *wl_seat)
{
	seat->state = state;
	seat->wl_seat = wl_seat;
	wl_seat_add_listener(wl_seat, &wl_seat_listener, seat);
	seat->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (state->running)
		daklakwl_seat_init_protocols(seat);
	daklakwl_buffer_init(&seat->buffer);
	seat->repeat_timer.callback = daklakwl_seat_repeat_timer_callback;
	seat->is_composing = true;
}

void daklakwl_seat_init_protocols(struct daklakwl_seat *seat)
{
	seat->zwp_input_method_v2
	    = zwp_input_method_manager_v2_get_input_method(
		seat->state->zwp_input_method_manager_v2, seat->wl_seat);
	zwp_input_method_v2_add_listener(seat->zwp_input_method_v2,
					 &zwp_input_method_v2_listener, seat);
	seat->zwp_virtual_keyboard_v1
	    = zwp_virtual_keyboard_manager_v1_create_virtual_keyboard(
		seat->state->zwp_virtual_keyboard_manager_v1, seat->wl_seat);
	seat->zwp_input_method_keyboard_grab_v2
	    = zwp_input_method_v2_grab_keyboard(seat->zwp_input_method_v2);
	zwp_input_method_keyboard_grab_v2_add_listener(
	    seat->zwp_input_method_keyboard_grab_v2,
	    &zwp_input_method_keyboard_grab_v2_listener, seat);
	seat->are_protocols_initted = true;
}

void daklakwl_seat_destroy(struct daklakwl_seat *seat)
{
	daklakwl_buffer_destroy(&seat->buffer);
	free(seat->pending_surrounding_text);
	free(seat->surrounding_text);
	free(seat->name);
	xkb_state_unref(seat->xkb_state);
	xkb_keymap_unref(seat->xkb_keymap);
	xkb_context_unref(seat->xkb_context);
	free(seat->xkb_keymap_string);
	if (seat->are_protocols_initted) {
		zwp_virtual_keyboard_v1_destroy(seat->zwp_virtual_keyboard_v1);
		zwp_input_method_keyboard_grab_v2_destroy(
		    seat->zwp_input_method_keyboard_grab_v2);
		zwp_input_method_v2_destroy(seat->zwp_input_method_v2);
	}
	wl_seat_destroy(seat->wl_seat);
	wl_list_remove(&seat->link);
	free(seat);
}

static void
daklakwl_send_message_to_socket_clients(struct daklakwl_state *state, char *msg,
					int sender)
{
	for (int i = 2; i < state->nfds; i++) {
		if (state->fds[i].fd != sender) {
			int byte_cnt
			    = send(state->fds[i].fd, msg, strlen(msg), 0);
			if (byte_cnt == -1) {
				perror("send message");
			}
		}
	}
}

void daklakwl_seat_composing_update(struct daklakwl_seat *seat)
{
	zwp_input_method_v2_set_preedit_string(
	    seat->zwp_input_method_v2, seat->buffer.text, seat->buffer.pos,
	    seat->buffer.pos);
	zwp_input_method_v2_commit(seat->zwp_input_method_v2,
				   seat->done_events_received);
}

void daklakwl_seat_composing_commit(struct daklakwl_seat *seat)
{
	zwp_input_method_v2_commit_string(seat->zwp_input_method_v2,
					  seat->buffer.text);
	zwp_input_method_v2_commit(seat->zwp_input_method_v2,
				   seat->done_events_received);
	daklakwl_buffer_clear(&seat->buffer);
}

int daklakwl_binding_compare(void const *_a, void const *_b)
{
	const struct daklakwl_binding *a = _a;
	const struct daklakwl_binding *b = _b;
	int keysym = a->keysym - b->keysym;
	if (keysym != 0)
		return keysym;
	int modifiers = a->modifiers - b->modifiers;
	if (modifiers != 0)
		return modifiers;
	return a->action - b->action;
}

int daklakwl_seat_binding_compare_without_action(void const *_a, void const *_b)
{
	const struct daklakwl_seat_binding *a = _a;
	const struct daklakwl_seat_binding *b = _b;
	int keycode = a->keycode - b->keycode;
	if (keycode != 0)
		return keycode;
	return a->mod_mask - b->mod_mask;
}

bool daklakwl_seat_handle_key_bindings(struct daklakwl_seat *seat,
				       struct wl_array *bindings,
				       struct daklakwl_seat_binding *press)
{
	struct daklakwl_seat_binding *found
	    = bsearch(press, bindings->data,
		      bindings->size / sizeof(struct daklakwl_seat_binding),
		      sizeof(struct daklakwl_seat_binding),
		      daklakwl_seat_binding_compare_without_action);
	if (found == NULL)
		return false;
	return daklakwl_seat_handle_action(seat, found->action);
}

bool daklakwl_seat_handle_key(struct daklakwl_seat *seat, xkb_keycode_t keycode)
{
	xkb_keysym_t keysym
	    = xkb_state_key_get_one_sym(seat->xkb_state, keycode);
	struct daklakwl_seat_binding press = {
	    .keycode = keycode,
	    .mod_mask = 0,
	    .action = 0,
	};
	xkb_mod_mask_t mod_count = xkb_keymap_num_mods(seat->xkb_keymap);
	for (xkb_mod_index_t i = 0; i < mod_count; i++) {
		bool is_active = xkb_state_mod_index_is_active(
		    seat->xkb_state, i, XKB_STATE_MODS_EFFECTIVE);
		bool is_consumed = xkb_state_mod_index_is_consumed(
		    seat->xkb_state, keycode, i);
		bool is_relevant
		    = (i != seat->mod_indices[DAKLAKWL_CAPS_INDEX])
		      && (i != seat->mod_indices[DAKLAKWL_NUM_INDEX]);
		(void)is_consumed;
		if (is_active && is_relevant)
			press.mod_mask |= 1 << i;
	}

	if (seat->is_composing && seat->buffer.len != 0
	    && daklakwl_seat_handle_key_bindings(
		seat, &seat->composing_bindings, &press)) {
		return true;
	}
	if (keysym == XKB_KEY_Shift_L || keysym == XKB_KEY_Shift_R
	    || keysym == XKB_KEY_Control_L || keysym == XKB_KEY_Control_R
	    || keysym == XKB_KEY_Alt_L || keysym == XKB_KEY_Alt_R
	    || keysym == XKB_KEY_Super_L || keysym == XKB_KEY_Super_R
	    || keysym == XKB_KEY_Hyper_L || keysym == XKB_KEY_Hyper_R
	    || keysym == XKB_KEY_Caps_Lock) {
		return false;
	}
	if (seat->is_composing) {
		bool ctrl_active = xkb_state_mod_name_is_active(
			seat->xkb_state, XKB_MOD_NAME_CTRL, XKB_STATE_EFFECTIVE);
		bool shift_active = xkb_state_mod_name_is_active(
			seat->xkb_state, XKB_MOD_NAME_SHIFT, XKB_STATE_EFFECTIVE);
		if (ctrl_active || (ctrl_active && shift_active)) {
			// like move cursor or select word or select all
			daklakwl_seat_composing_commit(seat);
			return false;
		}
		xkb_keysym_t keysym
		    = xkb_state_key_get_one_sym(seat->xkb_state, keycode);

		if (!((keysym >= XKB_KEY_a && keysym <= XKB_KEY_z)
		      || (keysym >= XKB_KEY_A && keysym <= XKB_KEY_Z))) {
			if (seat->buffer.len == 0)
				return false;
			daklakwl_seat_composing_commit(seat);
			return false;
		}

		uint32_t codepoint
		    = xkb_state_key_get_utf32(seat->xkb_state, keycode);
		if (codepoint != 0 && codepoint < 32)
			return false;

		int utf8_len
		    = xkb_state_key_get_utf8(seat->xkb_state, keycode, NULL, 0);
		if (utf8_len == 0)
			return false;

		char *utf8 = malloc(utf8_len + 1);
		xkb_state_key_get_utf8(seat->xkb_state, keycode, utf8,
				       utf8_len + 1);

		daklakwl_buffer_gi_append(&seat->buffer, utf8);
		if (daklakwl_buffer_should_not_append(&seat->buffer, utf8)) {
			free(utf8);
			return false;
		}
		daklakwl_buffer_raw_append(&seat->buffer, utf8);
		daklakwl_buffer_append(&seat->buffer, utf8);
		daklakwl_buffer_compose(&seat->buffer);
		daklakwl_seat_composing_update(seat);
		free(utf8);
		return true;
	}
	return false;
}

static void timespec_correct(struct timespec *ts)
{
	while (ts->tv_nsec >= 1000000000) {
		ts->tv_sec += 1;
		ts->tv_nsec -= 1000000000;
	}
}

void daklakwl_seat_repeat_timer_callback(struct daklakwl_timer *timer)
{
	struct daklakwl_seat *seat = wl_container_of(timer, seat, repeat_timer);
	seat->repeating_timestamp += 1000 / seat->repeat_rate;
	if (!daklakwl_seat_handle_key(seat, seat->repeating_keycode)) {
		wl_list_remove(&timer->link);
		zwp_virtual_keyboard_v1_key(
		    seat->zwp_virtual_keyboard_v1, seat->repeating_timestamp,
		    seat->repeating_keycode - 8, WL_KEYBOARD_KEY_STATE_PRESSED);
		seat->repeating_keycode = 0;
	}
	else {
		clock_gettime(CLOCK_MONOTONIC, &seat->repeat_timer.time);
		timer->time.tv_nsec += 1000000000 / seat->repeat_rate;
		timespec_correct(&timer->time);
	}
}

struct keycode_matches {
	struct daklakwl_seat *seat;
	xkb_keysym_t keysym;
	struct wl_array keycodes;
};

static void find_keycode(struct xkb_keymap *keymap, xkb_keycode_t keycode,
			 void *data)
{
	struct keycode_matches *matches = data;
	xkb_keysym_t keysym
	    = xkb_state_key_get_one_sym(matches->seat->xkb_state, keycode);
	if (keysym == XKB_KEY_NoSymbol)
		return;
	if (matches->keysym == keysym) {
		xkb_keycode_t *keycode_spot
		    = wl_array_add(&matches->keycodes, sizeof(xkb_keycode_t));
		*keycode_spot = keycode;
	}
}

int daklakwl_seat_binding_compare(void const *_a, void const *_b)
{
	const struct daklakwl_seat_binding *a = _a;
	const struct daklakwl_seat_binding *b = _b;
	int keycode = a->keycode - b->keycode;
	if (keycode != 0)
		return keycode;
	int mod_mask = a->mod_mask - b->mod_mask;
	if (mod_mask != 0)
		return mod_mask;
	return a->action - b->action;
}

void daklakwl_seat_set_up_bindings(struct daklakwl_seat *seat,
				   struct wl_array *state_bindings,
				   struct wl_array *seat_bindings)
{
	struct daklakwl_binding *state_binding;
	struct keycode_matches matches = {
	    .seat = seat,
	};
	wl_array_init(&matches.keycodes);
	wl_array_for_each(state_binding, state_bindings)
	{
		matches.keysym = state_binding->keysym;
		matches.keycodes.size = 0;
		xkb_keymap_key_for_each(seat->xkb_keymap, find_keycode,
					&matches);
		assert(matches.keycodes.size);
		xkb_mod_mask_t mod_mask = 0;
		for (int i = 0; i < _DAKLAKWL_MOD_LAST; i++) {
			if (state_binding->modifiers & (1 << i)) {
				mod_mask |= 1 << seat->mod_indices[i];
			}
		}
		xkb_keycode_t *keycode;
		wl_array_for_each(keycode, &matches.keycodes)
		{
			struct daklakwl_seat_binding *seat_binding
			    = wl_array_add(
				seat_bindings,
				sizeof(struct daklakwl_seat_binding));
			seat_binding->keycode = *keycode;
			seat_binding->mod_mask = mod_mask;
			seat_binding->action = state_binding->action;
		}
	}
	qsort(seat_bindings->data,
	      seat_bindings->size / sizeof(struct daklakwl_seat_binding),
	      sizeof(struct daklakwl_seat_binding),
	      daklakwl_seat_binding_compare);
	wl_array_release(&matches.keycodes);
}

void zwp_input_method_keyboard_grab_v2_keymap(
    void *data,
    struct zwp_input_method_keyboard_grab_v2 *zwp_input_method_keyboard_grab_v2,
    uint32_t format, int32_t fd, uint32_t size)
{
	struct daklakwl_seat *seat = data;
	char *map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (seat->xkb_keymap_string == NULL
	    || strcmp(seat->xkb_keymap_string, map) != 0) {
		zwp_virtual_keyboard_v1_keymap(seat->zwp_virtual_keyboard_v1,
					       format, fd, size);
		xkb_keymap_unref(seat->xkb_keymap);
		xkb_state_unref(seat->xkb_state);
		seat->xkb_keymap = xkb_keymap_new_from_string(
		    seat->xkb_context, map, XKB_KEYMAP_FORMAT_TEXT_V1,
		    XKB_KEYMAP_COMPILE_NO_FLAGS);
		seat->mod_indices[DAKLAKWL_SHIFT_INDEX]
		    = xkb_keymap_mod_get_index(seat->xkb_keymap,
					       XKB_MOD_NAME_SHIFT);
		seat->mod_indices[DAKLAKWL_CAPS_INDEX]
		    = xkb_keymap_mod_get_index(seat->xkb_keymap,
					       XKB_MOD_NAME_CAPS);
		seat->mod_indices[DAKLAKWL_CTRL_INDEX]
		    = xkb_keymap_mod_get_index(seat->xkb_keymap,
					       XKB_MOD_NAME_CTRL);
		seat->mod_indices[DAKLAKWL_ALT_INDEX]
		    = xkb_keymap_mod_get_index(seat->xkb_keymap,
					       XKB_MOD_NAME_ALT);
		seat->mod_indices[DAKLAKWL_NUM_INDEX]
		    = xkb_keymap_mod_get_index(seat->xkb_keymap,
					       XKB_MOD_NAME_NUM);
		seat->mod_indices[DAKLAKWL_MOD3_INDEX]
		    = xkb_keymap_mod_get_index(seat->xkb_keymap, "Mod3");
		seat->mod_indices[DAKLAKWL_LOGO_INDEX]
		    = xkb_keymap_mod_get_index(seat->xkb_keymap,
					       XKB_MOD_NAME_LOGO);
		seat->mod_indices[DAKLAKWL_MOD5_INDEX]
		    = xkb_keymap_mod_get_index(seat->xkb_keymap, "Mod5");
		seat->xkb_state = xkb_state_new(seat->xkb_keymap);
		free(seat->xkb_keymap_string);
		seat->xkb_keymap_string = strdup(map);
		daklakwl_seat_set_up_bindings(
		    seat, &seat->state->config.composing_bindings,
		    &seat->composing_bindings);
	}
	close(fd);
	munmap(map, size);
}

void zwp_input_method_keyboard_grab_v2_key(
    void *data,
    struct zwp_input_method_keyboard_grab_v2 *zwp_input_method_keyboard_grab_v2,
    uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	struct daklakwl_seat *seat = data;
	xkb_keycode_t keycode = key + 8;
	bool handled = false;

	if (state == WL_KEYBOARD_KEY_STATE_PRESSED
	    && seat->repeating_keycode != 0
	    && seat->repeating_keycode != keycode) {
		if (!daklakwl_seat_handle_key(seat, keycode)) {
			seat->repeating_keycode = 0;
			wl_list_remove(&seat->repeat_timer.link);
			goto forward;
		}
		if (xkb_keymap_key_repeats(seat->xkb_keymap, keycode)) {
			seat->repeating_keycode = keycode;
			seat->repeating_timestamp = time + seat->repeat_delay;
			clock_gettime(CLOCK_MONOTONIC,
				      &seat->repeat_timer.time);
			seat->repeat_timer.time.tv_nsec
			    += seat->repeat_delay * 1000000;
			timespec_correct(&seat->repeat_timer.time);
		}
		else {
			seat->repeating_keycode = 0;
		}
		return;
	}

	if (state == WL_KEYBOARD_KEY_STATE_RELEASED
	    && seat->repeating_keycode == keycode) {
		seat->repeating_keycode = 0;
		wl_list_remove(&seat->repeat_timer.link);
		return;
	}

	if (seat->active && state == WL_KEYBOARD_KEY_STATE_PRESSED)
		handled |= daklakwl_seat_handle_key(seat, keycode);

	if (state == WL_KEYBOARD_KEY_STATE_PRESSED
	    && xkb_key_repeats(seat->xkb_keymap, keycode) && handled) {
		seat->repeating_keycode = keycode;
		seat->repeating_timestamp = time + seat->repeat_delay;
		clock_gettime(CLOCK_MONOTONIC, &seat->repeat_timer.time);
		seat->repeat_timer.time.tv_nsec += seat->repeat_delay * 1000000;
		timespec_correct(&seat->repeat_timer.time);
		wl_list_insert(&seat->state->timers, &seat->repeat_timer.link);
		return;
	}

	if (state == WL_KEYBOARD_KEY_STATE_PRESSED && handled) {
		for (size_t i = 0;
		     i < sizeof seat->pressed / sizeof seat->pressed[0]; i++) {
			if (seat->pressed[i] == 0) {
				seat->pressed[i] = keycode;
				goto forward;
			}
		}
	}

	if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
		for (size_t i = 0;
		     i < sizeof seat->pressed / sizeof seat->pressed[0]; i++) {
			if (seat->pressed[i] == keycode) {
				seat->pressed[i] = 0;
				return;
			}
		}
	}

	if (handled)
		return;

forward:
	zwp_virtual_keyboard_v1_key(seat->zwp_virtual_keyboard_v1, time, key,
				    state);
}

void zwp_input_method_keyboard_grab_v2_modifiers(
    void *data,
    struct zwp_input_method_keyboard_grab_v2 *zwp_input_method_keyboard_grab_v2,
    uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
    uint32_t mods_locked, uint32_t group)
{
	struct daklakwl_seat *seat = data;
	xkb_state_update_mask(seat->xkb_state, mods_depressed, mods_latched,
			      mods_locked, 0, 0, group);
	zwp_virtual_keyboard_v1_modifiers(seat->zwp_virtual_keyboard_v1,
					  mods_depressed, mods_latched,
					  mods_locked, group);
}

void zwp_input_method_keyboard_grab_v2_repeat_info(
    void *data,
    struct zwp_input_method_keyboard_grab_v2 *zwp_input_method_keyboard_grab_v2,
    int32_t rate, int32_t delay)
{
	struct daklakwl_seat *seat = data;
	seat->repeat_rate = rate;
	seat->repeat_delay = delay;
}

struct zwp_input_method_keyboard_grab_v2_listener const
    zwp_input_method_keyboard_grab_v2_listener
    = {
	.keymap = zwp_input_method_keyboard_grab_v2_keymap,
	.key = zwp_input_method_keyboard_grab_v2_key,
	.modifiers = zwp_input_method_keyboard_grab_v2_modifiers,
	.repeat_info = zwp_input_method_keyboard_grab_v2_repeat_info,
};

void zwp_input_method_v2_activate(
    void *data, struct zwp_input_method_v2 *zwp_input_method_v2)
{
	struct daklakwl_seat *seat = data;
	seat->pending_activate = true;
	free(seat->pending_surrounding_text);
	seat->pending_surrounding_text = NULL;
	seat->pending_text_change_cause = 0;
	seat->pending_content_type_hint = 0;
	seat->pending_content_type_purpose = 0;
}

void zwp_input_method_v2_deactivate(
    void *data, struct zwp_input_method_v2 *zwp_input_method_v2)
{
	struct daklakwl_seat *seat = data;
	seat->pending_activate = false;
}

void zwp_input_method_v2_surrounding_text(
    void *data, struct zwp_input_method_v2 *zwp_input_method_v2,
    char const *text, uint32_t cursor, uint32_t anchor)
{
	struct daklakwl_seat *seat = data;
	free(seat->pending_surrounding_text);
	seat->pending_surrounding_text = strdup(text);
	seat->pending_surrounding_text_cursor = cursor;
	seat->pending_surrounding_text_anchor = anchor;
}

void zwp_input_method_v2_text_change_cause(
    void *data, struct zwp_input_method_v2 *zwp_input_method_v2, uint32_t cause)
{
	struct daklakwl_seat *seat = data;
	seat->pending_text_change_cause = cause;
}

void zwp_input_method_v2_content_type(
    void *data, struct zwp_input_method_v2 *zwp_input_method_v2, uint32_t hint,
    uint32_t purpose)
{
	struct daklakwl_seat *seat = data;
	seat->pending_content_type_hint = hint;
	seat->pending_content_type_purpose = purpose;
}

void zwp_input_method_v2_done(void *data,
			      struct zwp_input_method_v2 *zwp_input_method_v2)
{
	struct daklakwl_seat *seat = data;
	bool was_active = seat->active;
	seat->active = seat->pending_activate;
	free(seat->surrounding_text);
	seat->surrounding_text = seat->pending_surrounding_text;
	seat->pending_surrounding_text = NULL;
	seat->surrounding_text_cursor = seat->pending_surrounding_text_cursor;
	seat->surrounding_text_anchor = seat->pending_surrounding_text_anchor;
	seat->text_change_cause = seat->pending_text_change_cause;
	seat->content_type_hint = seat->pending_content_type_hint;
	seat->content_type_purpose = seat->pending_content_type_purpose;
	seat->done_events_received++;
	if (!was_active && seat->active) {
		daklakwl_buffer_clear(&seat->buffer);
	}
}

void zwp_input_method_v2_unavailable(
    void *data, struct zwp_input_method_v2 *zwp_input_method_v2)
{
	struct daklakwl_seat *seat = data;
	fprintf(stderr, "Input method unavailable on seat \"%s\".\n",
		seat->name);
	daklakwl_seat_destroy(seat);
}

struct zwp_input_method_v2_listener const zwp_input_method_v2_listener = {
    .activate = zwp_input_method_v2_activate,
    .deactivate = zwp_input_method_v2_deactivate,
    .surrounding_text = zwp_input_method_v2_surrounding_text,
    .text_change_cause = zwp_input_method_v2_text_change_cause,
    .content_type = zwp_input_method_v2_content_type,
    .done = zwp_input_method_v2_done,
    .unavailable = zwp_input_method_v2_unavailable,
};

void wl_seat_capabilities(void *data, struct wl_seat *wl_seat,
			  uint32_t capabilities)
{
	// removed
}

void wl_seat_name(void *data, struct wl_seat *wl_seat, char const *name)
{
	struct daklakwl_seat *seat = data;
	free(seat->name);
	seat->name = strdup(name);
}

struct wl_seat_listener const wl_seat_listener = {
    .capabilities = wl_seat_capabilities,
    .name = wl_seat_name,
};

void wl_seat_global(struct daklakwl_state *state, void *data)
{
	struct wl_seat *wl_seat = data;
	struct daklakwl_seat *seat = calloc(1, sizeof *seat);
	daklakwl_seat_init(seat, state, wl_seat);
	wl_list_insert(&state->seats, &seat->link);
}

struct daklakwl_global {
	char const *name;
	struct wl_interface const *interface;
	int version;
	bool is_singleton;
	union {
		ptrdiff_t offset;
		void (*callback)(struct daklakwl_state *state, void *data);
	};
};

static int daklakwl_global_compare(const void *a, const void *b)
{
	return strcmp(((struct daklakwl_global const *)a)->name,
		      ((struct daklakwl_global const *)b)->name);
}

static struct daklakwl_global const globals[] = {
    {
	.name = "wl_compositor",
	.interface = &wl_compositor_interface,
	.version = 4,
	.is_singleton = true,
	.offset = offsetof(struct daklakwl_state, wl_compositor),
    },
    {
	.name = "wl_seat",
	.interface = &wl_seat_interface,
	.version = 7,
	.is_singleton = false,
	.callback = wl_seat_global,
    },
    {
	.name = "wl_shm",
	.interface = &wl_shm_interface,
	.version = 1,
	.is_singleton = true,
	.offset = offsetof(struct daklakwl_state, wl_shm),
    },
    {
	.name = "zwp_input_method_manager_v2",
	.interface = &zwp_input_method_manager_v2_interface,
	.version = 1,
	.is_singleton = true,
	.offset = offsetof(struct daklakwl_state, zwp_input_method_manager_v2),
    },
    {
	.name = "zwp_virtual_keyboard_manager_v1",
	.interface = &zwp_virtual_keyboard_manager_v1_interface,
	.version = 1,
	.is_singleton = true,
	.offset
	= offsetof(struct daklakwl_state, zwp_virtual_keyboard_manager_v1),
    },
};

void wl_registry_global(void *data, struct wl_registry *wl_registry,
			uint32_t name, char const *interface, uint32_t version)
{
	struct daklakwl_global global = {.name = interface};
	struct daklakwl_global *found = bsearch(
	    &global, globals, sizeof globals / sizeof(struct daklakwl_global),
	    sizeof(struct daklakwl_global), daklakwl_global_compare);

	if (found == NULL)
		return;

	if (found->is_singleton) {
		*(void **)((uintptr_t)data + found->offset) = wl_registry_bind(
		    wl_registry, name, found->interface, found->version);
	}
	else {
		found->callback(data, wl_registry_bind(wl_registry, name,
						       found->interface,
						       found->version));
	}
}

void wl_registry_global_remove(void *data, struct wl_registry *wl_registry,
			       uint32_t name)
{
	// TODO
}

struct wl_registry_listener const wl_registry_listener = {
    .global = wl_registry_global,
    .global_remove = wl_registry_global_remove,
};

bool daklakwl_state_init(struct daklakwl_state *state)
{
	wl_list_init(&state->seats);
	wl_list_init(&state->timers);
	daklakwl_config_init(&state->config);

	if (!daklakwl_config_load(&state->config))
		return false;

	state->wl_display = wl_display_connect(NULL);
	if (state->wl_display == NULL) {
		perror("wl_display_connect");
		return false;
	}

	state->wl_registry = wl_display_get_registry(state->wl_display);
	wl_registry_add_listener(state->wl_registry, &wl_registry_listener,
				 state);
	wl_display_roundtrip(state->wl_display);

	for (size_t i = 0; i < sizeof globals / sizeof globals[0]; i++) {
		const struct daklakwl_global *global = &globals[i];
		if (!global->is_singleton)
			continue;
		struct wl_proxy **location
		    = (struct wl_proxy **)((uintptr_t)state + global->offset);
		if (*location == NULL) {
			fprintf(stderr,
				"required interface unsupported by compositor: "
				"%s\n",
				global->name);
			return false;
		}
	}

	struct daklakwl_seat *seat;
	wl_list_for_each(seat, &state->seats, link)
	    daklakwl_seat_init_protocols(seat);

	wl_display_flush(state->wl_display);

	int rc, on = 1;
	int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_fd == -1) {
		perror("socket creation");
		return false;
	}
	rc = ioctl(sock_fd, FIONBIO, (char *)&on);
	if (rc == -1) {
		perror("ioctl()");
		return false;
	}

	struct sockaddr_un server;
	memset(&server, 0, sizeof(server));
	server.sun_family = AF_UNIX;
	strncpy(server.sun_path, "/tmp/daklak.sock",
		sizeof(server.sun_path) - 1);

	unlink(server.sun_path);
	if (bind(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
		perror("bind");
		return false;
	}
	listen(sock_fd, SOMAXCONN);
	state->sock_server = server;
	memset(state->fds, 0, sizeof(state->fds));
	state->fds[0].fd = wl_display_get_fd(state->wl_display);
	state->fds[0].events = POLLIN;

	state->fds[1].fd = sock_fd;
	state->fds[1].events = POLLIN;
	state->nfds = 2;
	return true;
}

static bool interrupted;
static void sigint(int signal) { interrupted = true; }

int daklakwl_state_next_timer(struct daklakwl_state *state)
{
	int timeout = INT_MAX;
	if (wl_list_empty(&state->timers))
		return -1;
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	struct daklakwl_timer *timer;
	wl_list_for_each(timer, &state->timers, link)
	{
		int time = (timer->time.tv_sec - now.tv_sec) * 1000
			   + (timer->time.tv_nsec - now.tv_nsec) / 1000000;
		if (time < timeout)
			timeout = time;
	}
	return timeout;
}

void daklakwl_state_run_timers(struct daklakwl_state *state)
{
	if (wl_list_empty(&state->timers))
		return;
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	struct daklakwl_timer *timer, *tmp;
	wl_list_for_each_safe(timer, tmp, &state->timers, link)
	{
		bool expired = timer->time.tv_sec < now.tv_sec
			       || (timer->time.tv_sec == now.tv_sec
				   && timer->time.tv_nsec < now.tv_nsec);
		if (expired)
			timer->callback(timer);
	}
}

void daklakwl_state_run(struct daklakwl_state *state)
{
	state->running = true;
	signal(SIGINT, sigint);
	int current_size, on = 1;
	bool error = false, compressed_fds = false;
	char buffer[1024];
	while (state->running && !interrupted) {
		if (poll(state->fds, state->nfds,
			 daklakwl_state_next_timer(state))
		    == -1) {
			if (errno == EINTR)
				continue;
			perror("poll");
			break;
		}
		current_size = state->nfds;
		for (int i = 0; i < current_size; i++) {
			if (state->fds[i].revents == 0) {
				continue;
			}
			else if (state->fds[i].revents != POLLIN) {
				close(state->fds[i].fd);
				state->fds[i].fd = -1;
				compressed_fds = true;
				perror("revents");
				continue;
			}
			if (i == 0) {
				while (
				    wl_display_prepare_read(state->wl_display)
				    != 0)
					wl_display_dispatch_pending(
					    state->wl_display);
				if (state->fds[i].events & POLLIN)
					wl_display_read_events(
					    state->wl_display);
				else
					wl_display_cancel_read(
					    state->wl_display);

				if (wl_display_dispatch_pending(
					state->wl_display)
				    == -1) {
					perror("wl_display_dispatch");
					error = true;
					break;
				}
			}
			else if (i == 1) {
				int new_fd
				    = accept(state->fds[1].fd, NULL, NULL);
				if (new_fd == -1) {
					perror("connect");
					continue;
				}

				printf("New incomming connection: %d\n",
				       new_fd);
				ioctl(new_fd, FIONBIO, (char *)&on);
				state->fds[state->nfds].fd = new_fd;
				state->fds[state->nfds].events = POLLIN;
				state->nfds++;
			}
			else {
				memset(buffer, 0, sizeof(buffer));
				int rc = recv(state->fds[i].fd, buffer,
					      sizeof(buffer), 0);
				if (rc == -1) {
					perror("recv");
					continue;
				}
				else if (rc == 0) {
					close(state->fds[i].fd);
					state->fds[i].fd = -1;
					compressed_fds = true;
				}
				printf("recveiced: %s\n", buffer);
				if (strcmp(buffer, "daklak_toggle\n") == 0) {
					struct daklakwl_seat *seat, *tmp_seat;
					wl_list_for_each_safe(
					    seat, tmp_seat, &state->seats, link)
					{
						seat->is_composing
						    = !seat->is_composing;
						if (seat->is_composing) {
							daklakwl_send_message_to_socket_clients(
							    state,
							    "daklak_on\n", -1);
						}
						else {
							daklakwl_seat_composing_commit(seat);
							daklakwl_send_message_to_socket_clients(
							    state,
							    "daklak_off\n", -1);
						}
					}
				}
				else if (strcmp(buffer, "daklak_on\n") == 0) {
					struct daklakwl_seat *seat, *tmp_seat;
					wl_list_for_each_safe(
					    seat, tmp_seat, &state->seats, link)
					{
						seat->is_composing = true;
						daklakwl_send_message_to_socket_clients(
						    state, "daklak_on\n", -1);
					}
				}
				else if (strcmp(buffer, "daklak_off\n") == 0) {
					struct daklakwl_seat *seat, *tmp_seat;
					wl_list_for_each_safe(
					    seat, tmp_seat, &state->seats, link)
					{
						daklakwl_seat_composing_commit(seat);
						seat->is_composing = false;
						daklakwl_send_message_to_socket_clients(
						    state, "daklak_off\n", -1);
					}
				}
			}
		}

		if (error) {
			break;
		}
		if (compressed_fds) {
			compressed_fds = false;
			for (int i = 0; i < state->nfds; i++) {
				if (state->fds[i].fd == -1) {
					for (int j = i; j < state->nfds; j++) {
						state->fds[j].fd
						    = state->fds[j + 1].fd;
					}
					i--;
					state->nfds--;
				}
			}
		}

		daklakwl_state_run_timers(state);

		wl_display_flush(state->wl_display);

		if (wl_list_empty(&state->seats)) {
			fprintf(stderr, "No seats with input-method "
					"available.\n");
			error = true;
			break;
		}
	}
	signal(SIGINT, SIG_DFL);
	state->running = false;
}

void daklakwl_state_finish(struct daklakwl_state *state)
{
	struct daklakwl_seat *seat, *tmp_seat;
	wl_list_for_each_safe(seat, tmp_seat, &state->seats, link)
	    daklakwl_seat_destroy(seat);
	if (state->zwp_virtual_keyboard_manager_v1 != NULL) {
		zwp_virtual_keyboard_manager_v1_destroy(
		    state->zwp_virtual_keyboard_manager_v1);
	}
	if (state->zwp_input_method_manager_v2 != NULL)
		zwp_input_method_manager_v2_destroy(
		    state->zwp_input_method_manager_v2);
	if (state->wl_compositor != NULL)
		wl_compositor_destroy(state->wl_compositor);
	if (state->wl_registry != NULL)
		wl_registry_destroy(state->wl_registry);
	if (state->wl_display != NULL)
		wl_display_disconnect(state->wl_display);
	daklakwl_config_finish(&state->config);
}

int main(void)
{
	setlocale(LC_CTYPE, "en_US.utf8");
	pthread_t indicator_thread;
	struct daklakwl_state state = {0};
	if (!daklakwl_state_init(&state))
		return 1;
	pthread_create(&indicator_thread, NULL, (void *)&run_tray, &state);
	daklakwl_state_run(&state);
	daklakwl_state_finish(&state);
}
