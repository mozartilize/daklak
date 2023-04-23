#pragma once

#include <stdbool.h>

#include <sys/poll.h>
#include <sys/un.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include "input-method-unstable-v2-client-protocol.h"
#include "text-input-unstable-v3-client-protocol.h"
#include "virtual-keyboard-unstable-v1-client-protocol.h"

#include "actions.h"
#include "buffer.h"
#include "config.h"

enum daklak_modifier_index {
	DAKLAKWL_SHIFT_INDEX,
	DAKLAKWL_CAPS_INDEX,
	DAKLAKWL_CTRL_INDEX,
	DAKLAKWL_ALT_INDEX,
	DAKLAKWL_NUM_INDEX,
	DAKLAKWL_MOD3_INDEX,
	DAKLAKWL_LOGO_INDEX,
	DAKLAKWL_MOD5_INDEX,
	_DAKLAKWL_MOD_LAST,
};

enum daklakwl_modifier {
	DAKLAKWL_SHIFT = 1 << DAKLAKWL_SHIFT_INDEX,
	DAKLAKWL_CAPS = 1 << DAKLAKWL_CAPS_INDEX,
	DAKLAKWL_CTRL = 1 << DAKLAKWL_CTRL_INDEX,
	DAKLAKWL_ALT = 1 << DAKLAKWL_ALT_INDEX,
	DAKLAKWL_NUM = 1 << DAKLAKWL_NUM_INDEX,
	DAKLAKWL_MOD3 = 1 << DAKLAKWL_MOD3_INDEX,
	DAKLAKWL_LOGO = 1 << DAKLAKWL_LOGO_INDEX,
	DAKLAKWL_MOD5 = 1 << DAKLAKWL_MOD5_INDEX,
};

struct daklakwl_timer {
	struct wl_list link;
	struct timespec time;
	void (*callback)(struct daklakwl_timer *timer);
};

struct daklakwl_state {
	bool running;
	struct wl_display *wl_display;
	struct wl_registry *wl_registry;
	struct wl_compositor *wl_compositor;
	struct wl_shm *wl_shm;
	struct zwp_input_method_manager_v2 *zwp_input_method_manager_v2;
	struct zwp_virtual_keyboard_manager_v1 *zwp_virtual_keyboard_manager_v1;
	struct wl_list seats;
	struct wl_list timers;
	struct daklakwl_config config;
	struct pollfd fds[10];
	struct sockaddr_un sock_server;
	int nfds;
};

struct daklakwl_seat {
	struct wl_list link;
	struct daklakwl_state *state;
	struct wl_seat *wl_seat;

	bool are_protocols_initted;
	struct zwp_text_input_v3 *zwp_text_input_v3;
	struct zwp_input_method_v2 *zwp_input_method_v2;
	struct zwp_input_method_keyboard_grab_v2
	    *zwp_input_method_keyboard_grab_v2;
	struct zwp_virtual_keyboard_v1 *zwp_virtual_keyboard_v1;

	char *xkb_keymap_string;
	struct xkb_context *xkb_context;
	struct xkb_keymap *xkb_keymap;
	struct xkb_state *xkb_state;
	xkb_mod_index_t mod_indices[_DAKLAKWL_MOD_LAST];
	struct wl_array composing_bindings;

	// wl_seat
	char *name;

	// zwp_input_method_v2
	bool pending_activate, active;
	char *pending_surrounding_text, *surrounding_text;
	uint32_t pending_surrounding_text_cursor, surrounding_text_cursor;
	uint32_t pending_surrounding_text_anchor, surrounding_text_anchor;
	uint32_t pending_text_change_cause, text_change_cause;
	uint32_t pending_content_type_hint, content_type_hint;
	uint32_t pending_content_type_purpose, content_type_purpose;
	uint32_t done_events_received;

	// zwp_input_method_keyboard_grab_v2
	uint32_t repeat_rate;
	uint32_t repeat_delay;
	xkb_keycode_t pressed[64];
	xkb_keycode_t repeating_keycode;
	uint32_t repeating_timestamp;
	struct daklakwl_timer repeat_timer;

	struct daklakwl_buffer buffer;

	// composing
	bool is_composing;
};

struct daklakwl_binding {
	xkb_keysym_t keysym;
	enum daklakwl_modifier modifiers;
	enum daklakwl_action action;
};

struct daklakwl_seat_binding {
	xkb_keycode_t keycode;
	xkb_mod_mask_t mod_mask;
	enum daklakwl_action action;
};

void zwp_input_popup_surface_v2_text_input_rectangle(
    void *data, struct zwp_input_popup_surface_v2 *zwp_input_popup_surface_v2,
    int32_t x, int32_t y, int32_t width, int32_t height);

void wl_surface_enter(void *data, struct wl_surface *wl_surface,
		      struct wl_output *wl_output);
void wl_surface_leave(void *data, struct wl_surface *wl_surface,
		      struct wl_output *wl_output);

void zwp_input_method_keyboard_grab_v2_keymap(
    void *data,
    struct zwp_input_method_keyboard_grab_v2 *zwp_input_method_keyboard_grab_v2,
    uint32_t format, int32_t fd, uint32_t size);
void zwp_input_method_keyboard_grab_v2_key(
    void *data,
    struct zwp_input_method_keyboard_grab_v2 *zwp_input_method_keyboard_grab_v2,
    uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
void zwp_input_method_keyboard_grab_v2_modifiers(
    void *data,
    struct zwp_input_method_keyboard_grab_v2 *zwp_input_method_keyboard_grab_v2,
    uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
    uint32_t mods_locked, uint32_t group);
void zwp_input_method_keyboard_grab_v2_repeat_info(
    void *data,
    struct zwp_input_method_keyboard_grab_v2 *zwp_input_method_keyboard_grab_v2,
    int32_t rate, int32_t delay);

void zwp_input_method_v2_activate(
    void *data, struct zwp_input_method_v2 *zwp_input_method_v2);
void zwp_input_method_v2_deactivate(
    void *data, struct zwp_input_method_v2 *zwp_input_method_v2);
void zwp_input_method_v2_surrounding_text(
    void *data, struct zwp_input_method_v2 *zwp_input_method_v2,
    char const *text, uint32_t cursor, uint32_t anchor);
void zwp_input_method_v2_text_change_cause(
    void *data, struct zwp_input_method_v2 *zwp_input_method_v2,
    uint32_t cause);
void zwp_input_method_v2_content_type(
    void *data, struct zwp_input_method_v2 *zwp_input_method_v2, uint32_t hint,
    uint32_t purpose);
void zwp_input_method_v2_done(void *data,
			      struct zwp_input_method_v2 *zwp_input_method_v2);
void zwp_input_method_v2_unavailable(
    void *data, struct zwp_input_method_v2 *zwp_input_method_v2);

void wl_pointer_enter(void *data, struct wl_pointer *wl_pointer,
		      uint32_t serial, struct wl_surface *surface,
		      wl_fixed_t surface_x, wl_fixed_t surface_y);
void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
		      uint32_t serial, struct wl_surface *surface);
void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time,
		       wl_fixed_t surface_x, wl_fixed_t surface_y);
void wl_pointer_button(void *data, struct wl_pointer *wl_pointer,
		       uint32_t serial, uint32_t time, uint32_t button,
		       uint32_t state);
void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time,
		     uint32_t axis, wl_fixed_t value);
void wl_pointer_frame(void *data, struct wl_pointer *wl_pointer);
void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
			    uint32_t axis_source);
void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
			  uint32_t time, uint32_t axis);
void wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
			      uint32_t axis, int32_t discrete);
void wl_seat_capabilities(void *data, struct wl_seat *wl_seat,
			  uint32_t capabilities);

void wl_seat_name(void *data, struct wl_seat *wl_seat, char const *name);
void wl_seat_global(struct daklakwl_state *state, void *data);

void wl_output_geometry(void *data, struct wl_output *wl_output, int32_t x,
			int32_t y, int32_t physical_width,
			int32_t physical_height, int32_t subpixel,
			const char *make, const char *model, int32_t transform);
void wl_output_mode(void *data, struct wl_output *wl_output, uint32_t flags,
		    int32_t width, int32_t height, int32_t refresh);
void wl_output_done(void *data, struct wl_output *wl_output);
void wl_output_scale(void *data, struct wl_output *wl_output, int32_t factor);
void wl_output_global(struct daklakwl_state *state, void *data);

void wl_registry_global(void *data, struct wl_registry *wl_registry,
			uint32_t name, char const *interface, uint32_t version);
void wl_registry_global_remove(void *data, struct wl_registry *wl_registry,
			       uint32_t name);

void daklakwl_seat_init_protocols(struct daklakwl_seat *seat);
void daklakwl_seat_init(struct daklakwl_seat *seat,
			struct daklakwl_state *state, struct wl_seat *wl_seat);
void daklakwl_seat_destroy(struct daklakwl_seat *seat);
void daklakwl_seat_composing_update(struct daklakwl_seat *seat);
void daklakwl_seat_composing_commit(struct daklakwl_seat *seat);
void daklakwl_seat_selecting_update(struct daklakwl_seat *seat);
void daklakwl_seat_selecting_commit(struct daklakwl_seat *seat);
int daklakwl_binding_compare(void const *_a, void const *_b);
int daklakwl_seat_binding_compare(void const *_a, void const *_b);
int daklakwl_seat_binding_compare_without_action(void const *_a,
						 void const *_b);
bool daklakwl_seat_handle_key_bindings(struct daklakwl_seat *seat,
				       struct wl_array *bindings,
				       struct daklakwl_seat_binding *press);
bool daklakwl_seat_handle_key(struct daklakwl_seat *seat,
			      xkb_keycode_t keycode);
void daklakwl_seat_repeat_timer_callback(struct daklakwl_timer *timer);
void daklakwl_seat_set_up_bindings(struct daklakwl_seat *seat,
				   struct wl_array *state_bindings,
				   struct wl_array *seat_bindings);
void daklakwl_seat_cursor_update(struct daklakwl_seat *seat);
void daklakwl_seat_cursor_timer_callback(struct daklakwl_timer *timer);

bool daklakwl_state_init(struct daklakwl_state *state);
int daklakwl_state_next_timer(struct daklakwl_state *state);
void daklakwl_state_run_timers(struct daklakwl_state *state);
void daklakwl_state_run(struct daklakwl_state *state);
void daklakwl_state_finish(struct daklakwl_state *state);

extern struct zwp_input_popup_surface_v2_listener const
    zwp_input_popup_surface_v2_listener;
extern struct wl_seat_listener const wl_seat_listener;
extern struct wl_surface_listener const wl_surface_listener;
extern struct zwp_input_method_keyboard_grab_v2_listener const
    zwp_input_method_keyboard_grab_v2_listener;
extern struct zwp_input_method_v2_listener const zwp_input_method_v2_listener;
extern struct wl_pointer_listener const wl_pointer_listener;
extern struct wl_output_listener const wl_output_listener;
extern struct wl_registry_listener const wl_registry_listener;
extern struct zwp_input_method_v2_listener const zwp_input_method_v2_listener;
extern struct zwp_text_input_v3_listener const zwp_text_input_v3_listener;
extern struct zwp_input_method_keyboard_grab_v2_listener const
    zwp_input_method_keyboard_grab_v2_listener;
