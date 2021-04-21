#pragma once

#include <stddef.h>
#include <wchar.h>

struct daklakwl_buffer
{
	char *text;
	char *pending_text;
	size_t len;
	size_t pos;
	char accent;
};

int daklak_is_vowel(char);
void daklakwl_buffer_init(struct daklakwl_buffer *);
void daklakwl_buffer_destroy(struct daklakwl_buffer *);
void daklakwl_buffer_clear(struct daklakwl_buffer *);
void daklakwl_buffer_append(struct daklakwl_buffer *, char const *);
void daklakwl_buffer_delete_backwards(struct daklakwl_buffer *, size_t);
void daklakwl_buffer_delete_forwards(struct daklakwl_buffer *, size_t);
void daklakwl_buffer_move_left(struct daklakwl_buffer *);
void daklakwl_buffer_move_right(struct daklakwl_buffer *);
int daklakwl_buffer_compose(struct daklakwl_buffer *);
