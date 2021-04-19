#include "buffer.h"

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "mbstrings.h"

static wint_t const vowels[512] = {
	[97]  = 1, // a
	[101] = 1, // e
	[105] = 1, // i
	[111] = 1, // o
	[117] = 1, // u
	[121] = 1, // y
	[226] = 1, // â
	[234] = 1, // ê
	[244] = 1, // ô
	[259] = 1, // ă
	[417] = 1, // ơ
	[432] = 1, // ư
};

static char const accents[128] = {
	['f'] = 'f',
	['s'] = 's',
	['x'] = 'x',
	['j'] = 'j',
	['r'] = 'r',
};

static char const marks[128] = {
	['a'] = 'a',
	['e'] = 'e',
	['o'] = 'o',
	['w'] = 'w',
};

static wchar_t vowels_accents[] = {
	[119] = L'à',
};

void daklakwl_buffer_init(struct daklakwl_buffer *buffer)
{
	buffer->text = calloc(1, 1);
	buffer->len = 0;
	buffer->pos = 0;
}

void daklakwl_buffer_destroy(struct daklakwl_buffer *buffer)
{
	free(buffer->text);
}

void daklakwl_buffer_clear(struct daklakwl_buffer *buffer)
{
	buffer->text[0] = '\0';
	buffer->len = 0;
	buffer->pos = 0;
}

void daklakwl_buffer_append(struct daklakwl_buffer *buffer, char const *text)
{
	size_t text_len = strlen(text);
	buffer->text = realloc(buffer->text, buffer->len + text_len + 1);
	if (buffer->pos == 0)
	{
		memmove(
			buffer->text + text_len,
			buffer->text,
			buffer->len + 1);
		memcpy(
			buffer->text,
			text,
			text_len);
	}
	else if (buffer->pos == buffer->len)
	{
		memcpy(
			buffer->text + buffer->len,
			text,
			text_len + 1);
	}
	else
	{
		memmove(
			buffer->text + buffer->pos + text_len,
			buffer->text + buffer->pos,
			buffer->len - buffer->pos + 1);
		memcpy(
			buffer->text + buffer->pos,
			text,
			text_len);
	}
	buffer->len += text_len;
	buffer->pos += text_len;
}

void daklakwl_buffer_delete_backwards(struct daklakwl_buffer *buffer, size_t amt)
{
	if (buffer->pos == 0)
		return;
	size_t end = buffer->pos;
	size_t start = buffer->pos;
	for (size_t i = 0; i < amt; i++)
	{
		start -= 1;
		for (; start != 0; start--)
		{
			if ((buffer->text[start] & 0x80) == 0 || (buffer->text[start] & 0xC0) == 0xC0)
			{
				break;
			}
		}
	}
	memmove(
		buffer->text + start,
		buffer->text + end,
		buffer->len - end + 1);
	buffer->len -= end - start;
	buffer->pos -= end - start;
}

void daklakwl_buffer_delete_forwards(struct daklakwl_buffer *buffer, size_t amt)
{
	if (buffer->pos == buffer->len)
		return;
	size_t start = buffer->pos;
	size_t end = start + 1;
	for (; end != buffer->len; end++)
	{
		if ((buffer->text[end] & 0x80) == 0 || (buffer->text[end] & 0xC0) == 0xC0)
		{
			break;
		}
	}
	memmove(
		buffer->text + start,
		buffer->text + end,
		buffer->len - end + 1);
	buffer->len -= end - start;
}

void daklakwl_buffer_move_left(struct daklakwl_buffer *buffer)
{
	if (buffer->pos == 0)
		return;
	buffer->pos -= 1;
	while ((buffer->text[buffer->pos] & 0x80) != 0 && (buffer->text[buffer->pos] & 0xC0) != 0xC0)
	{
		buffer->pos -= 1;
	}
}

void daklakwl_buffer_move_right(struct daklakwl_buffer *buffer)
{
	if (buffer->pos == buffer->len)
		return;
	buffer->pos += 1;
	while ((buffer->text[buffer->pos] & 0x80) != 0 && (buffer->text[buffer->pos] & 0xC0) != 0xC0)
	{
		buffer->pos += 1;
	}
}

void daklakwl_buffer_convert_romaji(struct daklakwl_buffer *buffer)
{
	// removed
}

void daklakwl_buffer_append_single(struct daklakwl_buffer *buffer, const char *text) {
	char c = (char)*text;
	daklakwl_buffer_append(buffer, text);
	if (accents[(int)c]) {
		buffer->accent = c;
		buffer->accent_pos = buffer->len;
	}

}

void daklakwl_buffer_convert(struct daklakwl_buffer *buffer) {
	wchar_t *text = mbsrdup(buffer->text, 0);
	wchar_t c0 = text[0], c1 = text[1], c2 = text[2], c3 = text[3];
	if (vowels[c0] && !vowels[c1]) {
		if (buffer->accent) {
			text[0] = vowels_accents[(int)c0 + (int)buffer->accent];
		}
	}

}

void daklakwl_buffer_convert_trailing_n(struct daklakwl_buffer *buffer)
{
	if (buffer->pos == 0)
		return;
	if (buffer->text[buffer->pos - 1] == 'n')
	{
		daklakwl_buffer_delete_backwards(buffer, 1);
		daklakwl_buffer_append(buffer, "ん");
	}
}
