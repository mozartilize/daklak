// gcc `pkg-config appindicator3-0.1 --cflags` tray.c -o build/tray `pkg-config
// appindicator3-0.1 --libs`
#include "tray.h"

#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

#define ICON_NAME_EN "indicator-keyboard-En"
#define ICON_NAME_VI "indicator-keyboard-Vi"

static void handlecommand(gchar *sz, AppIndicator *indicator)
{
	if (g_ascii_strcasecmp(sz, "daklak_off\n") == 0) {
		app_indicator_set_icon(indicator, ICON_NAME_EN);
	}
	else if (g_ascii_strcasecmp(sz, "daklak_on\n") == 0) {
		app_indicator_set_icon(indicator, ICON_NAME_VI);
	}
}

static gboolean mycallback(GIOChannel *channel, GIOCondition cond,
			   gpointer data)
{
	gchar *str_return;
	gsize length;
	gsize terminator_pos;
	GError *error = NULL;
	AppIndicator *indicator = data;
	GIOStatus status = g_io_channel_read_line(channel, &str_return, &length,
						  &terminator_pos, &error);
	if (status != G_IO_STATUS_NORMAL) {
		perror("read");
		return TRUE;
	}
	if (error != NULL) {
		printf("%s\n", error->message);
		return TRUE;
	}
	handlecommand(str_return, indicator);
	g_free(str_return);
	return TRUE;
}

static void quit_activated(GSimpleAction *simple, GVariant *parameter,
			   gpointer user_data)
{
	struct indicator_state *state;
	state = user_data;
	state->state->running = false;
	close(state->sock_fd);
	gtk_main_quit();
}

static GActionEntry app_entries[]
    = {{"quit", quit_activated, NULL, NULL, NULL}};

AppIndicator *gtr_icon_new(struct indicator_state *state)
{
	AppIndicator *indicator
	    = app_indicator_new("daklak-indicator", ICON_NAME_VI,
				APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);

	GtkWidget *menu;
	GMenu *menuModel = g_menu_new();
	GMenuItem *quitMenuItem = g_menu_item_new("Quit", "app.quit");
	g_menu_append_item(menuModel, quitMenuItem);
	menu = gtk_menu_new_from_model(G_MENU_MODEL(menuModel));

	GSimpleActionGroup *group;
	group = g_simple_action_group_new();

	g_action_map_add_action_entries(G_ACTION_MAP(group), app_entries,
					G_N_ELEMENTS(app_entries),
					(gpointer)state);

	gtk_widget_insert_action_group(menu, "app", G_ACTION_GROUP(group));

	app_indicator_set_menu(indicator, GTK_MENU(menu));
	app_indicator_set_title(indicator, "daklak");
	return indicator;
}

void run_tray(struct daklakwl_state *state)
{
	gtk_init(NULL, NULL);
	int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_fd == -1) {
		perror("socket");
		exit(1);
	}
	struct sockaddr_un server;

	memset(&server, 0, sizeof(server));
	server.sun_family = AF_UNIX;
	strncpy(server.sun_path, "/tmp/daklak.sock",
		sizeof(server.sun_path) - 1);

	if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server))
	    == -1) {
		perror("connect");
		exit(1);
	}
	struct indicator_state istate = {
	    .state = state,
	    .sock_fd = sock_fd,
	};
	AppIndicator *indicator = gtr_icon_new(&istate);

	GIOChannel *gio_channel = g_io_channel_unix_new(sock_fd);
	g_io_add_watch(gio_channel, G_IO_IN, mycallback, (gpointer)indicator);

	gtk_main();
}
