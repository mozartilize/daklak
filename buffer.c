#include "buffer.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

static char const *const vowel_a_marks[] = {
    ['s'] = "á", ['f'] = "à", ['x'] = "ã", ['r'] = "ả", ['j'] = "ạ",
};

static char const *const vowel_aw_marks[] = {
    ['s'] = "ắ", ['f'] = "ằ", ['x'] = "ẵ", ['r'] = "ẳ", ['j'] = "ặ",
};

static char const *const vowel_aa_marks[] = {
    ['s'] = "ấ", ['f'] = "ầ", ['x'] = "ẫ", ['r'] = "ẩ", ['j'] = "ậ",
};

static char const *const vowel_e_marks[] = {
    ['s'] = "é", ['f'] = "è", ['x'] = "ẽ", ['r'] = "ẻ", ['j'] = "ẹ",
};

static char const *const vowel_ee_marks[] = {
    ['s'] = "ế", ['f'] = "ề", ['x'] = "ễ", ['r'] = "ể", ['j'] = "ệ",
};

static char const *const vowel_i_marks[] = {
    ['s'] = "í", ['f'] = "ì", ['x'] = "ĩ", ['r'] = "ỉ", ['j'] = "ị",
};

static char const *const vowel_o_marks[] = {
    ['s'] = "ó", ['f'] = "ò", ['x'] = "õ", ['r'] = "ỏ", ['j'] = "ọ",
};

static char const *const vowel_oo_marks[] = {
    ['s'] = "ố", ['f'] = "ồ", ['x'] = "ỗ", ['r'] = "ổ", ['j'] = "ộ",
};

static char const *const vowel_ow_marks[] = {
    ['s'] = "ớ", ['f'] = "ờ", ['x'] = "ỡ", ['r'] = "ở", ['j'] = "ợ",
};

static char const *const vowel_u_marks[] = {
    ['s'] = "ú", ['f'] = "ù", ['x'] = "ũ", ['r'] = "ủ", ['j'] = "ụ",
};

static char const *const vowel_uw_marks[] = {
    ['s'] = "ứ", ['f'] = "ừ", ['x'] = "ữ", ['r'] = "ử", ['j'] = "ự",
};

static char const *const vowel_y_marks[] = {
    ['s'] = "ý", ['f'] = "ỳ", ['x'] = "ỹ", ['r'] = "ỷ", ['j'] = "ỵ",
};

static char const *const vowel_upper_a_marks[] = {
    ['s'] = "Á", ['f'] = "À", ['x'] = "Ã", ['r'] = "Ả", ['j'] = "Ạ",
};

static char const *const vowel_upper_aw_marks[] = {
    ['s'] = "Ắ", ['f'] = "Ằ", ['x'] = "Ẵ", ['r'] = "Ẳ", ['j'] = "Ặ",
};

static char const *const vowel_upper_aa_marks[] = {
    ['s'] = "Ấ", ['f'] = "Ầ", ['x'] = "Ẫ", ['r'] = "Ẩ", ['j'] = "Ậ",
};

static char const *const vowel_upper_e_marks[] = {
    ['s'] = "É", ['f'] = "È", ['x'] = "Ẽ", ['r'] = "Ẻ", ['j'] = "Ẹ",
};

static char const *const vowel_upper_ee_marks[] = {
    ['s'] = "Ế", ['f'] = "Ề", ['x'] = "Ễ", ['r'] = "Ể", ['j'] = "Ệ",
};

static char const *const vowel_upper_i_marks[] = {
    ['s'] = "Í", ['f'] = "Ì", ['x'] = "Ĩ", ['r'] = "Ỉ", ['j'] = "Ị",
};

static char const *const vowel_upper_o_marks[] = {
    ['s'] = "Ó", ['f'] = "Ò", ['x'] = "Õ", ['r'] = "Ỏ", ['j'] = "Ọ",
};

static char const *const vowel_upper_oo_marks[] = {
    ['s'] = "Ố", ['f'] = "Ồ", ['x'] = "Ỗ", ['r'] = "Ổ", ['j'] = "Ộ",
};

static char const *const vowel_upper_ow_marks[] = {
    ['s'] = "Ớ", ['f'] = "Ờ", ['x'] = "Ỡ", ['r'] = "Ở", ['j'] = "Ợ",
};

static char const *const vowel_upper_u_marks[] = {
    ['s'] = "Ú", ['f'] = "Ù", ['x'] = "Ũ", ['r'] = "Ủ", ['j'] = "Ụ",
};

static char const *const vowel_upper_uw_marks[] = {
    ['s'] = "Ứ", ['f'] = "Ừ", ['x'] = "Ữ", ['r'] = "Ử", ['j'] = "Ự",
};

static char const *const vowel_upper_y_marks[] = {
    ['s'] = "Ý", ['f'] = "Ỳ", ['x'] = "Ỹ", ['r'] = "Ỷ", ['j'] = "Ỵ",
};

static char const *const *const vowels[7929] = {
    [L'a'] = vowel_a_marks,	   [L'ă'] = vowel_aw_marks,
    [L'â'] = vowel_aa_marks,	   [L'e'] = vowel_e_marks,
    [L'ê'] = vowel_ee_marks,	   [L'i'] = vowel_i_marks,
    [L'o'] = vowel_o_marks,	   [L'ô'] = vowel_oo_marks,
    [L'ơ'] = vowel_ow_marks,	   [L'u'] = vowel_u_marks,
    [L'ư'] = vowel_uw_marks,	   [L'y'] = vowel_y_marks,
    [L'A'] = vowel_upper_a_marks,  [L'Ă'] = vowel_upper_aw_marks,
    [L'Â'] = vowel_upper_aa_marks, [L'E'] = vowel_upper_e_marks,
    [L'Ê'] = vowel_upper_ee_marks, [L'I'] = vowel_upper_i_marks,
    [L'O'] = vowel_upper_o_marks,  [L'Ô'] = vowel_upper_oo_marks,
    [L'Ơ'] = vowel_upper_ow_marks, [L'U'] = vowel_upper_u_marks,
    [L'Ư'] = vowel_upper_uw_marks, [L'Y'] = vowel_upper_y_marks,
};

static int const is_accents[128] = {
    [L'a'] = 1,
    [L'e'] = 1,
    [L'o'] = 1,
    [L'w'] = 1,
};

static char const *const vowel_a_accents[] = {
    [L'a'] = "â",
    [L'w'] = "ă",
};

static char const *const vowel_e_accents[] = {
    [L'e'] = "ê",
};

static char const *const vowel_o_accents[] = {
    [L'o'] = "ô",
    [L'w'] = "ơ",
};

static char const *const vowel_u_accents[] = {
    [L'w'] = "ư",
};

static char const *const vowel_upper_a_accents[] = {
    [L'a'] = "Â",
    [L'w'] = "Ă",
};

static char const *const vowel_upper_e_accents[] = {
    [L'e'] = "Ê",
};

static char const *const vowel_upper_o_accents[] = {
    [L'o'] = "Ô",
    [L'w'] = "Ơ",
};

static char const *const vowel_upper_u_accents[] = {
    [L'w'] = "Ư",
};

static char const *const *const vowels_accents[7929] = {
    [L'a'] = vowel_a_accents,	    [L'e'] = vowel_e_accents,
    [L'o'] = vowel_o_accents,	    [L'u'] = vowel_u_accents,
    [L'A'] = vowel_upper_a_accents, [L'E'] = vowel_upper_e_accents,
    [L'O'] = vowel_upper_o_accents, [L'U'] = vowel_upper_u_accents,
};

static int const marks[128] = {
    [L's'] = 1, [L'f'] = 1, [L'x'] = 1, [L'r'] = 1, [L'j'] = 1,
};

static int const type1_c0[7929] = {
    [L'â'] = 1, [L'i'] = 1, [L'u'] = 1, [L'ư'] = 1, [L'y'] = 1,
};

static int const type1_c1[7929] = {
    [L'ê'] = 1,
    [L'â'] = 1,
    [L'ô'] = 1,
    [L'ơ'] = 1,
};

static int const type2_c0[7929] = {
    [L'ă'] = 1,
    [L'o'] = 1,
    [L'u'] = 1,
};

static int const type2_c1[7929] = {
    [L'ă'] = 1,
    [L'o'] = 1,
    [L'i'] = 1,
    [L'y'] = 1,
};

static int const type2_c2[7929] = {
    [L'ê'] = 1,
};

static bool is_type1(wchar_t c0, wchar_t c1)
{
	return type1_c0[towlower(c0)] && c1 && type1_c1[towlower(c1)];
}

static bool is_type2(wchar_t c0, wchar_t c1, wchar_t c2)
{
	return type2_c0[towlower(c0)] && c1 && type2_c1[towlower(c1)] && c2 &&
	       type2_c2[towlower(c2)];
}

static int is_mark(char c)
{
	switch (c) {
	case 's':
	case 'f':
	case 'x':
	case 'r':
	case 'j':
	case 'S':
	case 'F':
	case 'X':
	case 'R':
	case 'J':
		return 1;
	default:
		return 0;
	}
}

static int is_vowel(char c) { return vowels[(int)c] ? 1 : 0; }

size_t mbslen(const char *s)
{
	mbstate_t state;
	size_t result = 0;
	size_t nbytes;
	memset(&state, '\0', sizeof(state));
	while ((nbytes = mbrlen(s, MB_LEN_MAX, &state)) > 0) {
		if (nbytes >= (size_t)-2)
			return (size_t)-1;
		s += nbytes;
		++result;
	}
	return result;
}

wchar_t *mbsrdup(const char *s)
{
	size_t size = strlen(s) + 1;
	wchar_t *buf = (wchar_t *)malloc(size * sizeof(wchar_t));
	size = mbstowcs(buf, s, size);
	if (size == (size_t)-1)
		return NULL;
	buf = (wchar_t *)realloc(buf, (size + 1) * sizeof(wchar_t));
	return buf;
}

void daklakwl_buffer_init(struct daklakwl_buffer *buffer)
{
	buffer->text = calloc(1, 1);
	buffer->len = 0;
	buffer->pos = 0;
	buffer->gi = calloc(1, 1);
	buffer->raw = calloc(1, 1);
}

void daklakwl_buffer_destroy(struct daklakwl_buffer *buffer)
{
	free(buffer->text);
	free(buffer->gi);
	free(buffer->raw);
}

void daklakwl_buffer_clear(struct daklakwl_buffer *buffer)
{
	buffer->text[0] = '\0';
	buffer->gi[0] = '\0';
	buffer->raw[0] = '\0';
	buffer->catalyst = '\0';
	buffer->len = 0;
	buffer->pos = 0;
}

void daklakwl_buffer_append(struct daklakwl_buffer *buffer, char const *text)
{
	size_t text_len = strlen(text);
	buffer->text = realloc(buffer->text, buffer->len + text_len + 1);
	if (buffer->pos == 0) {
		memmove(buffer->text + text_len, buffer->text, buffer->len + 1);
		memcpy(buffer->text, text, text_len);
	} else if (buffer->pos == buffer->len) {
		memcpy(buffer->text + buffer->len, text, text_len + 1);
	} else {
		memmove(buffer->text + buffer->pos + text_len,
			buffer->text + buffer->pos,
			buffer->len - buffer->pos + 1);
		memcpy(buffer->text + buffer->pos, text, text_len);
	}
	buffer->len += text_len;
	buffer->pos += text_len;
}

void daklakwl_buffer_raw_append(struct daklakwl_buffer *buffer,
				char const *text)
{
	strcat(buffer->raw, text);
}

void daklakwl_buffer_delete_backwards(struct daklakwl_buffer *buffer,
				      size_t amt)
{
	if (buffer->pos == 0)
		return;
	size_t end = buffer->pos;
	size_t start = buffer->pos;
	size_t raw_end = buffer->pos;
	size_t raw_start = buffer->pos;
	for (size_t i = 0; i < amt; i++) {
		start -= 1;
		raw_start -= 1;
		for (; start != 0; start--) {
			if ((buffer->text[start] & 0x80) == 0 ||
			    (buffer->text[start] & 0xC0) == 0xC0) {
				break;
			}
		}
	}
	size_t raw_len = strlen(buffer->raw);
	for (; raw_start != 0; raw_start--) {
		if (buffer->raw[raw_start] == buffer->text[start] ||
		    buffer->raw[raw_start - 1] == buffer->text[start - 1])
			break;
	}
	for (; raw_end != 0; raw_end--) {
		if (buffer->raw[raw_end] == buffer->text[end] &&
		    buffer->raw[raw_end - 1] == buffer->text[end - 1])
			break;
		if (buffer->text[raw_end] &&
		    (buffer->raw[raw_end] == buffer->text[end] ||
		     buffer->raw[raw_end - 1] == buffer->text[end - 1]))
			break;
	}
	if (raw_end == 0) {
		raw_end = end;
		buffer->catalyst = '\0';
	}
	size_t i = end - start - (raw_end - raw_start);
	size_t match_last_char_pos = raw_len;
	for (; i != 0; i--, match_last_char_pos--) {
		if (buffer->raw[match_last_char_pos - 1] ==
		    buffer->text[buffer->len - 1])
			break;
		buffer->catalyst = '\0';
	}
	memmove(buffer->raw + match_last_char_pos, buffer->raw + raw_len, 1);
	memmove(buffer->text + start, buffer->text + end,
		buffer->len - end + 1);
	buffer->len -= end - start;
	buffer->pos -= end - start;
	memmove(buffer->raw + raw_start, buffer->raw + raw_end,
		raw_len - raw_end + 1);
}

void daklakwl_buffer_delete_forwards(struct daklakwl_buffer *buffer, size_t amt)
{
	if (buffer->pos == buffer->len)
		return;
	size_t start = buffer->pos;
	size_t end = start + 1;
	for (; end != buffer->len; end++) {
		if ((buffer->text[end] & 0x80) == 0 ||
		    (buffer->text[end] & 0xC0) == 0xC0) {
			break;
		}
	}
	memmove(buffer->text + start, buffer->text + end,
		buffer->len - end + 1);
	buffer->len -= end - start;
}

void daklakwl_buffer_move_left(struct daklakwl_buffer *buffer)
{
	if (buffer->pos == 0)
		return;
	buffer->pos -= 1;
	while ((buffer->text[buffer->pos] & 0x80) != 0 &&
	       (buffer->text[buffer->pos] & 0xC0) != 0xC0) {
		buffer->pos -= 1;
	}
}

void daklakwl_buffer_move_right(struct daklakwl_buffer *buffer)
{
	if (buffer->pos == buffer->len)
		return;
	buffer->pos += 1;
	while ((buffer->text[buffer->pos] & 0x80) != 0 &&
	       (buffer->text[buffer->pos] & 0xC0) != 0xC0) {
		buffer->pos += 1;
	}
}

static int daklakwl_buffer_compose_vowels(struct daklakwl_buffer *buf)
{
	if (buf->len == 0)
		return 0;
	wchar_t *wc_text = mbsrdup(buf->text);
	size_t wc_len = mbslen(buf->text);
	wchar_t c0 = '\0', c1 = '\0', c2 = '\0', c3 = '\0', c4 = '\0';
	c0 = wc_text[0];
	if (wc_len >= 2)
		c1 = wc_text[1];
	if (wc_len >= 3)
		c2 = wc_text[2];
	if (wc_len >= 4)
		c3 = wc_text[3];
	if (wc_len >= 5)
		c4 = wc_text[4];

	if (vowels[c0] && vowels[c1] && !vowels[c2] && !vowels[c3] &&
	    marks[towlower(c4)] && wc_len == 5) {
		char const *const *vowels_marks = vowels[c1];
		if (vowels_marks[towlower(c4)]) {
			char *c0_override = (char *)calloc(1, sizeof(char));
			char *c2_override = (char *)calloc(1, sizeof(char));
			char *c3_override = (char *)calloc(1, sizeof(char));
			wctomb(c0_override, c0);
			wctomb(c2_override, c2);
			wctomb(c3_override, c3);
			strcat(c0_override, vowels_marks[towlower(c4)]);
			strcat(c0_override, c2_override);
			strcat(c0_override, c3_override);
			buf->text = c0_override;
			free(c2_override);
			free(c3_override);
			buf->len = strlen(buf->text);
			buf->pos = buf->len;
			return 1;
		}
	} else if (vowels[c0] && vowels[c1] && vowels[c2] && !vowels[c3] &&
		   is_accents[towlower(c4)] && wc_len == 5) {
		// TODO: shall we?
		char const *const *accents = vowels_accents[c1];
		if (accents != NULL && accents[towlower(c4)]) {
			char *c0_override = (char *)calloc(1, sizeof(char));
			char *c2_override = (char *)calloc(1, sizeof(char));
			char *c3_override = (char *)calloc(1, sizeof(char));
			wctomb(c0_override, c0);
			wctomb(c2_override, c2);
			wctomb(c3_override, c3);
			strcat(c0_override, accents[towlower(c4)]);
			strcat(c0_override, c2_override);
			strcat(c0_override, c3_override);
			free(c2_override);
			buf->text = c0_override;
			buf->len = strlen(buf->text);
			buf->pos = buf->len;
			return 1;
		}
	} else if (vowels[c0] && vowels[c1] && vowels[c2] && !vowels[c3] &&
		   marks[towlower(c4)] && wc_len == 5) {
		if (is_type2(c0, c1, c2)) {
			char const *const *vowels_marks = vowels[c2];
			if (vowels_marks[towlower(c4)]) {
				char *c0_override =
				    (char *)calloc(1, sizeof(char));
				char *c1_override =
				    (char *)calloc(1, sizeof(char));
				char *c3_override =
				    (char *)calloc(1, sizeof(char));
				wctomb(c0_override, c0);
				wctomb(c1_override, c1);
				wctomb(c3_override, c3);
				strcat(c0_override, c1_override);
				strcat(c0_override, vowels_marks[towlower(c4)]);
				strcat(c0_override, c3_override);
				free(c1_override);
				free(c3_override);
				buf->text = c0_override;
				buf->len = strlen(buf->text);
				buf->pos = buf->len;
				return 1;
			}
		} else {
			char const *const *vowels_marks = vowels[c1];
			if (vowels_marks[towlower(c4)]) {
				char *c0_override =
				    (char *)calloc(1, sizeof(char));
				char *c2_override =
				    (char *)calloc(1, sizeof(char));
				wctomb(c0_override, c0);
				wctomb(c2_override, c2);
				strcat(c0_override, vowels_marks[towlower(c3)]);
				strcat(c0_override, c2_override);
				free(c2_override);
				buf->text = c0_override;
				buf->len = strlen(buf->text);
				buf->pos = buf->len;
				return 1;
			}
		}
	} else if (vowels[c0] && vowels[c1] && vowels[c2] &&
		   is_accents[towlower(c3)] && wc_len == 4) {
		// TODO: handle uouw and uyee
		char const *const *c1_accents = vowels_accents[c1];
		char const *const *c2_accents = vowels_accents[c2];
		if (c1_accents && c1_accents[c3]) {
			char *c0_override = (char *)calloc(1, sizeof(char));
			char *c2_override = (char *)calloc(1, sizeof(char));
			wctomb(c0_override, c0);
			wctomb(c2_override, c2);
			strcat(c0_override, c1_accents[c3]);
			strcat(c0_override, c2_override);
			free(c2_override);
			buf->text = c0_override;
			buf->len = strlen(buf->text);
			buf->pos = buf->len;
			return 1;
		} else if (c2_accents && c2_accents[c3]) {
			char *c0_override = (char *)calloc(1, sizeof(char));
			char *c1_override = (char *)calloc(1, sizeof(char));
			wctomb(c0_override, c0);
			wctomb(c1_override, c1);
			strcat(c0_override, c1_override);
			strcat(c0_override, c2_accents[c3]);
			free(c1_override);
			buf->text = c0_override;
			buf->len = strlen(buf->text);
			buf->pos = buf->len;
			return 1;
		}
	} else if (vowels[c0] && vowels[c1] && vowels[c2] &&
		   marks[towlower(c3)] && wc_len == 4) {
		if (is_type2(c0, c1, c2)) {
			char const *const *vowels_marks = vowels[c2];
			if (vowels_marks[towlower(c3)]) {
				char *c0_override =
				    (char *)calloc(1, sizeof(char));
				char *c1_override =
				    (char *)calloc(1, sizeof(char));
				wctomb(c0_override, c0);
				wctomb(c1_override, c1);
				strcat(c0_override, c1_override);
				strcat(c0_override, vowels_marks[towlower(c3)]);
				free(c1_override);
				buf->text = c0_override;
				buf->len = strlen(buf->text);
				buf->pos = buf->len;
				return 1;
			}
		} else {
			char const *const *vowels_marks = vowels[c1];
			if (vowels_marks[towlower(c3)]) {
				char *c0_override =
				    (char *)calloc(1, sizeof(char));
				char *c2_override =
				    (char *)calloc(1, sizeof(char));
				wctomb(c0_override, c0);
				wctomb(c2_override, c2);
				strcat(c0_override, vowels_marks[towlower(c3)]);
				strcat(c0_override, c2_override);
				free(c2_override);
				buf->text = c0_override;
				buf->len = strlen(buf->text);
				buf->pos = buf->len;
				return 1;
			}
		}
	} else if (vowels[c0] && vowels[c1] && !vowels[c2] &&
		   marks[towlower(c3)] && wc_len == 4) {
		char const *const *vowels_marks = vowels[c1];
		if (vowels_marks[towlower(c3)]) {
			char *c0_override = (char *)calloc(1, sizeof(char));
			char *c2_override = (char *)calloc(1, sizeof(char));
			wctomb(c0_override, c0);
			wctomb(c2_override, c2);
			strcat(c0_override, vowels_marks[towlower(c3)]);
			strcat(c0_override, c2_override);
			buf->text = c0_override;
			free(c2_override);
			buf->len = strlen(buf->text);
			buf->pos = buf->len;
			return 1;
		}
	} else if (vowels[c0] && vowels[c1] && !vowels[c2] &&
		   is_accents[towlower(c3)] && wc_len == 4) {
		char const *const *accents = vowels_accents[c1];
		if (accents != NULL && accents[towlower(c3)]) {
			char *c0_override = (char *)calloc(1, sizeof(char));
			wctomb(c0_override, c0);
			char *c2_override = (char *)calloc(1, sizeof(char));
			wctomb(c2_override, c2);
			strcat(c0_override, accents[towlower(c3)]);
			strcat(c0_override, c2_override);
			buf->text = (char *)c0_override;
			free(c2_override);
			buf->len = strlen(buf->text);
			buf->pos = buf->len;
			return 1;
		}
	} else if (vowels[c0] && !vowels[c1] && !vowels[c2] &&
		   marks[towlower(c3)] && wc_len == 4) {
		char const *const *vowels_marks = vowels[c0];
		if (vowels_marks[towlower(c3)]) {
			buf->text = strdup(vowels_marks[towlower(c3)]);
			char *c1_override = (char *)calloc(1, sizeof(char));
			wctomb(c1_override, c1);
			char *c2_override = (char *)calloc(1, sizeof(char));
			wctomb(c2_override, c2);
			strcat(buf->text, c1_override);
			strcat(buf->text, c2_override);
			free(c1_override);
			free(c2_override);
			buf->len = strlen(buf->text);
			buf->pos = buf->len;
			return 1;
		}
	} else if (vowels[c0] && !vowels[c1] && !vowels[c2] &&
		   is_accents[towlower(c3)] && wc_len == 4) {
		char const *const *accents = vowels_accents[c0];
		if (accents != NULL && accents[towlower(c3)]) {
			buf->text = strdup(accents[towlower(c3)]);
			char *c1_override = (char *)calloc(1, sizeof(char));
			wctomb(c1_override, c1);
			char *c2_override = (char *)calloc(1, sizeof(char));
			wctomb(c2_override, c2);
			strcat(buf->text, c1_override);
			strcat(buf->text, c2_override);
			free(c1_override);
			free(c2_override);
			buf->len = strlen(buf->text);
			buf->pos = buf->len;
			return 1;
		}
	} else if (vowels[c0] && !vowels[c1] && is_accents[towlower(c2)] &&
		   wc_len == 3) {
		char const *const *accents = vowels_accents[c0];
		if (accents != NULL && accents[towlower(c2)]) {
			char *c0_override = (char *)calloc(
			    strlen(accents[towlower(c2)]), sizeof(char));
			char *c1_override = (char *)calloc(1, sizeof(char));
			wctomb(c1_override, c1);
			buf->text =
			    strcat(strcpy(c0_override, accents[towlower(c2)]),
				   c1_override);
			free(c1_override);
			buf->len = strlen(buf->text);
			buf->pos = buf->len;
			return 1;
		}
	} else if (vowels[c0] && !vowels[c1] && marks[towlower(c2)] &&
		   wc_len == 3) {
		char const *const *vowels_marks = vowels[c0];
		if (vowels_marks[towlower(c2)]) {
			char *c0_override = (char *)calloc(
			    strlen(vowels_marks[towlower(c2)]), sizeof(char));
			char *c1_override = (char *)calloc(1, sizeof(char));
			wctomb(c1_override, c1);
			buf->text = strcat(
			    strcpy(c0_override, vowels_marks[towlower(c2)]),
			    c1_override);
			free(c1_override);
			// buf->mark_pos = 1
			buf->len = strlen(buf->text);
			buf->pos = buf->len;
			// buf->mark = c2
			return 1;
		}
	} else if (vowels[c0] && vowels[c1] && is_accents[towlower(c2)] &&
		   wc_len == 3) {
		char const *const *c0_accents = vowels_accents[c0];
		char const *const *c1_accents = vowels_accents[c1];
		if (c1_accents != NULL && c1_accents[towlower(c2)]) {
			char *c0_override = (char *)calloc(1, sizeof(char));
			wctomb(c0_override, c0);
			buf->text =
			    strcat(c0_override, c1_accents[towlower(c2)]);
			buf->len = strlen(buf->text);
			buf->pos = buf->len;
			return 1;
		} else if (c0_accents != NULL && c0_accents[towlower(c2)]) {
			char *c1_override = (char *)calloc(1, sizeof(char));
			wctomb(c1_override, c1);
			buf->text = strdup(c0_accents[towlower(c2)]);
			strcat(buf->text, c1_override);
			buf->len = strlen(buf->text);
			buf->pos = buf->len;
			return 1;
		}
	} else if (vowels[c0] && vowels[c1] && marks[towlower(c2)] &&
		   wc_len == 3) {
		if (strcmp(buf->gi, "gi") == 0 || strcmp(buf->gi, "qu") == 0) {
			char const *const *vowels_marks = vowels[c1];
			if (vowels_marks[towlower(c2)]) {
				char *c0_override =
				    (char *)calloc(1, sizeof(char));
				wctomb(c0_override, c0);
				buf->text = strcat(c0_override,
						   vowels_marks[towlower(c2)]);
				buf->len = strlen(buf->text);
				buf->pos = buf->len;
				return 1;
			}
		} else {
			char const *const *vowels_marks = vowels[c0];
			if (vowels_marks[towlower(c2)]) {
				char *c0_override = (char *)calloc(
				    strlen(vowels_marks[towlower(c2)]),
				    sizeof(char));
				char *c1_override =
				    (char *)calloc(1, sizeof(char));
				wctomb(c1_override, c1);
				buf->text =
				    strcat(strcpy(c0_override,
						  vowels_marks[towlower(c2)]),
					   c1_override);
				buf->len = strlen(buf->text);
				buf->pos = buf->len;
				return 1;
			}
		}
	} else if (vowels[c0] && is_accents[towlower(c1)] && wc_len == 2) {
		char const *const *accents = vowels_accents[c0];
		if (accents != NULL && accents[towlower(c1)]) {
			buf->text = strdup(accents[towlower(c1)]);
			buf->len = strlen(buf->text);
			buf->pos = buf->len;
			return 1;
		}
	} else if (vowels[c0] && marks[towlower(c1)] && wc_len == 2) {
		char const *const *vowels_marks = vowels[c0];
		if (vowels_marks[towlower(c1)]) {
			buf->text = strdup(vowels_marks[towlower(c1)]);
			buf->len = strlen(buf->text);
			buf->pos = buf->len;
			return 1;
		}
	}
	free(wc_text);
	return 0;
}

static char const *const d_accents[] = {
    [L'd'] = "đ",
    [L'D'] = "Đ",
};

static int daklakwl_buffer_compose_dd(struct daklakwl_buffer *buf)
{
	wchar_t *wc_text = mbsrdup(buf->text);
	size_t wc_len = mbslen(buf->text);
	if (wc_len <= 1)
		return 0;
	wchar_t c0 = wc_text[0];
	wchar_t cN = wc_text[wc_len - 1];
	if (towlower(c0) == L'd' && towlower(cN) == L'd') {
		if (wc_len > 2) {
			char *tail = (char *)calloc(buf->len - 2, sizeof(char));
			memcpy(tail, buf->text + 1, buf->len - 2);
			buf->text = strdup(d_accents[c0]);
			strcat(buf->text, tail);
			free(tail);
		} else {
			free(buf->text);
			buf->text = strdup(d_accents[c0]);
		}
		buf->gi = strdup(d_accents[c0]);
		buf->len = strlen(buf->text);
		buf->pos = buf->len;
		return 1;
	}
	free(wc_text);
	return 0;
}

static int daklakwl_buffer_compose_full(struct daklakwl_buffer *buf)
{
	if ((strcmp(buf->gi, "d") == 0 || strcmp(buf->gi, "đ") == 0 ||
	     strcmp(buf->gi, "D") == 0 || strcmp(buf->gi, "Đ") == 0) &&
	    buf->len > 1) {
		if (!daklakwl_buffer_compose_dd(buf)) {
			size_t gi_len = strlen(buf->gi);
			char *t2 = strdup(buf->gi);
			char *t = calloc(buf->len - gi_len, 1);
			memcpy(t, buf->text + gi_len, buf->len - gi_len);
			struct daklakwl_buffer sub_buf = {
			    .text = t,
			    .len = buf->len - gi_len,
			    .pos = buf->pos - gi_len,
			    .gi = calloc(1, 1),
			};
			int composed = daklakwl_buffer_compose_vowels(&sub_buf);
			strcat(t2, sub_buf.text);
			buf->text = t2;
			buf->len = strlen(t2);
			buf->pos = buf->len;
			return composed;
		} else {
			return 1;
		}
	} else {
		return daklakwl_buffer_compose_vowels(buf);
	}
	return 0;
}

void daklakwl_buffer_gi_append(struct daklakwl_buffer *buffer, const char *utf8)
{
	char c = tolower(utf8[0]);
	if (buffer->len == 0 && buffer->gi[0] == '\0' &&
	    (c == 'g' || c == 'q' || c == 'd')) {
		strcat(buffer->gi, utf8);
	} else if (strlen(buffer->gi) == 1 &&
		   (buffer->gi[0] == 'g' || buffer->gi[0] == 'q' ||
		    buffer->gi[0] == 'G' || buffer->gi[0] == 'Q')) {
		if (c == 'i' || c == 'u') {
			strcat(buffer->gi, utf8);
		} else if (buffer->len == 0) {
			buffer->gi[0] = '\0';
		}
	}
}

void daklakwl_buffer_compose(struct daklakwl_buffer *buffer)
{
	int composed;
	char cN = buffer->text[buffer->len - 1];
	if (buffer->catalyst && buffer->catalyst == cN && is_mark(cN)) {
		composed = 0;
	} else {
		composed = daklakwl_buffer_compose_full(buffer);
	}
	if (composed) {
		buffer->catalyst = cN;
	} else {
		if (buffer->catalyst == cN) {
			size_t raw_len = strlen(buffer->raw) - 1;
			free(buffer->text);
			char *temp_raw = buffer->raw;
			buffer->text = calloc(raw_len, 1);
			strncpy(buffer->text, temp_raw, raw_len);
			buffer->raw = calloc(raw_len, 1);
			strncpy(buffer->raw, temp_raw, raw_len);
			free(temp_raw);
			buffer->len = raw_len;
			buffer->pos = raw_len;
			buffer->catalyst = '\0';
		}
	}
}

bool daklakwl_buffer_should_not_append(struct daklakwl_buffer *buf,
				       const char *utf8)
{
	return !is_vowel(utf8[0]) && buf->len == 0 && towlower(utf8[0]) != 'd';
}
