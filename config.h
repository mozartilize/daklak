#pragma once

#include <stdbool.h>
#include <wayland-client-core.h>

struct daklakwl_config {
	bool active_at_startup;
	struct wl_array composing_bindings;
};

void daklakwl_config_init(struct daklakwl_config *config);
bool daklakwl_config_load(struct daklakwl_config *config);
void daklakwl_config_finish(struct daklakwl_config *config);
