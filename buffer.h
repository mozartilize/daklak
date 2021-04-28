#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <wchar.h>

struct daklakwl_buffer
{
	char *text;
	char *raw;
	char *pending_text;
	size_t len;
	size_t pos;
	char accent;
	char *gi;
	char catalyst;
};

bool daklakwl_buffer_should_not_append(struct daklakwl_buffer *, char const *);
void daklakwl_buffer_init(struct daklakwl_buffer *);
void daklakwl_buffer_destroy(struct daklakwl_buffer *);
void daklakwl_buffer_clear(struct daklakwl_buffer *);
void daklakwl_buffer_append(struct daklakwl_buffer *, char const *);
void daklakwl_buffer_raw_append(struct daklakwl_buffer *, char const *);
void daklakwl_buffer_gi_append(struct daklakwl_buffer *, const char *);
void daklakwl_buffer_delete_backwards(struct daklakwl_buffer *, size_t);
void daklakwl_buffer_delete_forwards(struct daklakwl_buffer *, size_t);
void daklakwl_buffer_move_left(struct daklakwl_buffer *);
void daklakwl_buffer_move_right(struct daklakwl_buffer *);
void daklakwl_buffer_compose(struct daklakwl_buffer *);
