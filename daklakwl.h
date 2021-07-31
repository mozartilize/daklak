#include <stdbool.h>

#include <sys/poll.h>
#include <sys/un.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

#include "input-method-unstable-v2-client-protocol.h"
#include "text-input-unstable-v3-client-protocol.h"
#include "virtual-keyboard-unstable-v1-client-protocol.h"

struct daklakwl_state {
	bool running;
	struct wl_display *wl_display;
	struct wl_registry *wl_registry;
	struct wl_compositor *wl_compositor;
	struct wl_shm *wl_shm;
	struct zwp_text_input_manager_v3 *zwp_text_input_manager_v3;
	struct zwp_input_method_manager_v2 *zwp_input_method_manager_v2;
	struct zwp_virtual_keyboard_manager_v1 *zwp_virtual_keyboard_manager_v1;
	struct wl_list seats;
	struct wl_list timers;
	int max_scale;
	struct pollfd fds[10];
	struct sockaddr_un sock_server;
	int nfds;
};
