#include <gtk/gtk.h>

#include "daklakwl.h"
#include "libappindicator/app-indicator.h"

struct indicator_state {
	struct daklakwl_state *state;
	int sock_fd;
};

AppIndicator *gtr_icon_new(struct indicator_state *);
void run_tray(struct daklakwl_state *);
