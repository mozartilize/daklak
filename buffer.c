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

static char const *const *const vowels_marks[] = {
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

static int is_accent(char c)
{
	switch (c) {
	case 'a':
	case 'e':
	case 'o':
	case 'w':
	case 'A':
	case 'E':
	case 'O':
	case 'W':
		return 1;
	default:
		return 0;
	}
};

static char const *const vowel_a_accents[] = {
    [L'a'] = "â",
    [L'w'] = "ă",
};

static char const *const vowel_as_accents[] = {
    [L'a'] = "ấ",
    [L'w'] = "ắ",
};

static char const *const vowel_af_accents[] = {
    [L'a'] = "ầ",
    [L'w'] = "ằ",
};

static char const *const vowel_ar_accents[] = {
    [L'a'] = "ẩ",
    [L'w'] = "ẳ",
};

static char const *const vowel_ax_accents[] = {
    [L'a'] = "ẫ",
    [L'w'] = "ẵ",
};

static char const *const vowel_aj_accents[] = {
    [L'a'] = "ậ",
    [L'w'] = "ặ",
};

static char const *const vowel_e_accents[] = {
    [L'e'] = "ê",
};

static char const *const vowel_es_accents[] = {
    [L'e'] = "ế",
};

static char const *const vowel_ef_accents[] = {
    [L'e'] = "ề",
};

static char const *const vowel_er_accents[] = {
    [L'e'] = "ể",
};

static char const *const vowel_ex_accents[] = {
    [L'e'] = "ễ",
};

static char const *const vowel_ej_accents[] = {
    [L'e'] = "ệ",
};

static char const *const vowel_o_accents[] = {
    [L'o'] = "ô",
    [L'w'] = "ơ",
};

static char const *const vowel_os_accents[] = {
    [L'o'] = "ố",
    [L'w'] = "ớ",
};

static char const *const vowel_of_accents[] = {
    [L'o'] = "ồ",
    [L'w'] = "ờ",
};

static char const *const vowel_or_accents[] = {
    [L'o'] = "ổ",
    [L'w'] = "ở",
};

static char const *const vowel_ox_accents[] = {
    [L'o'] = "ỗ",
    [L'w'] = "ợ",
};

static char const *const vowel_oj_accents[] = {
    [L'o'] = "ộ",
    [L'w'] = "ợ",
};

static char const *const vowel_u_accents[] = {
    [L'w'] = "ư",
};

static char const *const vowel_us_accents[] = {
    [L'w'] = "ứ",
};

static char const *const vowel_uf_accents[] = {
    [L'w'] = "ừ",
};

static char const *const vowel_ur_accents[] = {
    [L'w'] = "ử",
};

static char const *const vowel_ux_accents[] = {
    [L'w'] = "ữ",
};

static char const *const vowel_uj_accents[] = {
    [L'w'] = "ự",
};

static char const *const vowel_upper_a_accents[] = {
    [L'a'] = "Â",
    [L'w'] = "Ă",
};

static char const *const vowel_upper_as_accents[] = {
    [L'a'] = "Ấ",
    [L'w'] = "Ắ",
};

static char const *const vowel_upper_af_accents[] = {
    [L'a'] = "Ầ",
    [L'w'] = "Ằ",
};

static char const *const vowel_upper_ar_accents[] = {
    [L'a'] = "Ẩ",
    [L'w'] = "Ẳ",
};

static char const *const vowel_upper_ax_accents[] = {
    [L'a'] = "Ẫ",
    [L'w'] = "Ẵ",
};

static char const *const vowel_upper_aj_accents[] = {
    [L'a'] = "Ậ",
    [L'w'] = "Ặ",
};

static char const *const vowel_upper_e_accents[] = {
    [L'e'] = "Ê",
};

static char const *const vowel_upper_es_accents[] = {
    [L'e'] = "Ế",
};

static char const *const vowel_upper_ef_accents[] = {
    [L'e'] = "Ề",
};

static char const *const vowel_upper_er_accents[] = {
    [L'e'] = "Ể",
};

static char const *const vowel_upper_ex_accents[] = {
    [L'e'] = "Ễ",
};

static char const *const vowel_upper_ej_accents[] = {
    [L'e'] = "Ệ",
};

static char const *const vowel_upper_o_accents[] = {
    [L'o'] = "Ô",
    [L'w'] = "Ơ",
};

static char const *const vowel_upper_os_accents[] = {
    [L'o'] = "Ố",
    [L'w'] = "Ớ",
};

static char const *const vowel_upper_of_accents[] = {
    [L'o'] = "Ồ",
    [L'w'] = "Ờ",
};

static char const *const vowel_upper_or_accents[] = {
    [L'o'] = "Ổ",
    [L'w'] = "Ở",
};

static char const *const vowel_upper_ox_accents[] = {
    [L'o'] = "Ỗ",
    [L'w'] = "Ợ",
};

static char const *const vowel_upper_oj_accents[] = {
    [L'o'] = "Ộ",
    [L'w'] = "Ợ",
};

static char const *const vowel_upper_u_accents[] = {
    [L'w'] = "Ư",
};

static char const *const vowel_upper_us_accents[] = {
    [L'w'] = "Ứ",
};

static char const *const vowel_upper_uf_accents[] = {
    [L'w'] = "Ừ",
};

static char const *const vowel_upper_ur_accents[] = {
    [L'w'] = "Ử",
};

static char const *const vowel_upper_ux_accents[] = {
    [L'w'] = "Ữ",
};

static char const *const vowel_upper_uj_accents[] = {
    [L'w'] = "Ự",
};

static char const *const *const vowels_accents[] = {
    [L'a'] = vowel_a_accents,	     [L'e'] = vowel_e_accents,
    [L'o'] = vowel_o_accents,	     [L'u'] = vowel_u_accents,
    [L'á'] = vowel_as_accents,	     [L'é'] = vowel_es_accents,
    [L'ó'] = vowel_os_accents,	     [L'ú'] = vowel_us_accents,
    [L'à'] = vowel_af_accents,	     [L'è'] = vowel_ef_accents,
    [L'ò'] = vowel_of_accents,	     [L'ù'] = vowel_uf_accents,
    [L'ả'] = vowel_ar_accents,	     [L'ẻ'] = vowel_er_accents,
    [L'ỏ'] = vowel_or_accents,	     [L'ủ'] = vowel_ur_accents,
    [L'ã'] = vowel_ax_accents,	     [L'ẽ'] = vowel_ex_accents,
    [L'õ'] = vowel_ox_accents,	     [L'ũ'] = vowel_ux_accents,
    [L'ạ'] = vowel_aj_accents,	     [L'ẹ'] = vowel_ej_accents,
    [L'ọ'] = vowel_oj_accents,	     [L'ụ'] = vowel_uj_accents,
    [L'A'] = vowel_upper_a_accents,  [L'E'] = vowel_upper_e_accents,
    [L'O'] = vowel_upper_o_accents,  [L'U'] = vowel_upper_u_accents,
    [L'Á'] = vowel_upper_as_accents, [L'É'] = vowel_upper_es_accents,
    [L'Ó'] = vowel_upper_os_accents, [L'Ú'] = vowel_upper_us_accents,
    [L'À'] = vowel_upper_af_accents, [L'È'] = vowel_upper_ef_accents,
    [L'Ò'] = vowel_upper_of_accents, [L'Ù'] = vowel_upper_uf_accents,
    [L'Ả'] = vowel_upper_ar_accents, [L'Ẻ'] = vowel_upper_er_accents,
    [L'Ỏ'] = vowel_upper_or_accents, [L'Ủ'] = vowel_upper_ur_accents,
    [L'Ã'] = vowel_upper_ax_accents, [L'Ẽ'] = vowel_upper_ex_accents,
    [L'Õ'] = vowel_upper_ox_accents, [L'Ũ'] = vowel_upper_ux_accents,
    [L'Ạ'] = vowel_upper_aj_accents, [L'Ẹ'] = vowel_upper_ej_accents,
    [L'Ọ'] = vowel_upper_oj_accents, [L'Ụ'] = vowel_upper_uj_accents,
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

static int is_vowel(wchar_t c)
{
	static int const vowels[7929] = {
	    [L'a'] = 1, [L'ă'] = 1, [L'â'] = 1, [L'e'] = 1, [L'ê'] = 1,
	    [L'i'] = 1, [L'o'] = 1, [L'ô'] = 1, [L'ơ'] = 1, [L'u'] = 1,
	    [L'ư'] = 1, [L'y'] = 1, [L'A'] = 1, [L'Ă'] = 1, [L'Â'] = 1,
	    [L'E'] = 1, [L'Ê'] = 1, [L'I'] = 1, [L'O'] = 1, [L'Ô'] = 1,
	    [L'Ơ'] = 1, [L'U'] = 1, [L'Ư'] = 1, [L'Y'] = 1, [L'á'] = 1,
	    [L'é'] = 1, [L'ó'] = 1, [L'ú'] = 1, [L'à'] = 1, [L'è'] = 1,
	    [L'ò'] = 1, [L'ù'] = 1, [L'ả'] = 1, [L'ẻ'] = 1, [L'ỏ'] = 1,
	    [L'ủ'] = 1, [L'ã'] = 1, [L'ẽ'] = 1, [L'õ'] = 1, [L'ũ'] = 1,
	    [L'ạ'] = 1, [L'ẹ'] = 1, [L'ọ'] = 1, [L'ụ'] = 1, [L'Á'] = 1,
	    [L'É'] = 1, [L'Ó'] = 1, [L'Ú'] = 1, [L'À'] = 1, [L'È'] = 1,
	    [L'Ò'] = 1, [L'Ù'] = 1, [L'Ả'] = 1, [L'Ẻ'] = 1, [L'Ỏ'] = 1,
	    [L'Ủ'] = 1, [L'Ã'] = 1, [L'Ẽ'] = 1, [L'Õ'] = 1, [L'Ũ'] = 1,
	    [L'Ạ'] = 1, [L'Ẹ'] = 1, [L'Ọ'] = 1, [L'Ụ'] = 1,
	};
	return vowels[(int)c] ? 1 : 0;
}

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
	for (size_t i = 0; i < 4; i++) {
		if (buffer->steps[i]) {
			free(buffer->steps[i]);
			buffer->steps[i] = NULL;
		}
	}
}

void daklakwl_buffer_clear(struct daklakwl_buffer *buffer)
{
	buffer->text[0] = '\0';
	buffer->gi[0] = '\0';
	buffer->raw[0] = '\0';
	buffer->catalyst = '\0';
	buffer->len = 0;
	buffer->pos = 0;
	for (size_t i = 0; i < 4; i++) {
		if (buffer->steps[i]) {
			buffer->steps[i] = '\0';
		}
	}
}

void daklakwl_buffer_append(struct daklakwl_buffer *buffer, char const *text)
{
	size_t text_len = strlen(text);
	if (buffer->root) {
		size_t real_len = buffer->text - buffer->root + buffer->len;
		buffer->root = realloc(buffer->root, real_len + text_len + 1);
	} else {
		buffer->text =
		    realloc(buffer->text, buffer->len + text_len + 1);
	}
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
	size_t text_len = strlen(text);
	size_t raw_len = strlen(buffer->raw);
	size_t raw_pos = buffer->pos;
	buffer->raw = realloc(buffer->raw, raw_len + text_len + 1);
	if (raw_pos == 0) {
		memmove(buffer->raw + text_len, buffer->raw, raw_len + 1);
		memcpy(buffer->raw, text, text_len);
	} else if (raw_pos == raw_len) {
		memcpy(buffer->raw + raw_len, text, text_len + 1);
	} else {
		size_t pos = buffer->pos;
		for (; raw_pos != 0; raw_pos--) {
			if (buffer->raw[raw_pos] == buffer->text[pos] &&
			    buffer->raw[raw_pos - 1] == buffer->text[pos - 1])
				break;
			if (buffer->text[raw_pos] &&
			    (buffer->raw[raw_pos] == buffer->text[pos] ||
			     buffer->raw[raw_pos - 1] == buffer->text[pos - 1]))
				break;
		}
		memmove(buffer->raw + raw_pos + text_len, buffer->raw + raw_pos,
			raw_len - buffer->pos + 1);
		memcpy(buffer->raw + raw_pos, text, text_len);
	}
}

void daklakwl_buffer_delete_backwards(struct daklakwl_buffer *buffer,
				      size_t amt)
{
	if (buffer->pos == 0)
		return;
	size_t end = buffer->pos;
	size_t start = buffer->pos;
	for (size_t i = 0; i < amt; i++) {
		start -= 1;
		for (; start != 0; start--) {
			if ((buffer->text[start] & 0x80) == 0 ||
			    (buffer->text[start] & 0xC0) == 0xC0) {
				break;
			}
		}
	}
	memmove(buffer->text + start, buffer->text + end,
		buffer->len - end + 1);
	buffer->len -= end - start;
	buffer->pos -= end - start;
}

void daklakwl_buffer_delete_backwards_all(struct daklakwl_buffer *buffer,
					  size_t amt)
{
	if (buffer->pos == 0)
		return;
	size_t end = buffer->pos;
	size_t start = buffer->pos;

	for (size_t i = 0; i < amt; i++) {
		start -= 1;
		for (; start != 0; start--) {
			if ((buffer->text[start] & 0x80) == 0 ||
			    (buffer->text[start] & 0xC0) == 0xC0) {
				break;
			}
		}
	}
	memmove(buffer->text + start, buffer->text + end,
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

static void daklakwl_buffer_steps_append(struct daklakwl_buffer *buffer,
					 size_t wc_pos, wchar_t wc)
{
	size_t len = strlen(buffer->steps[wc_pos]);
	buffer->steps[wc_pos] = realloc(buffer->steps[wc_pos], len + 2);
	buffer->steps[wc_pos][len] = wctob(wc);
	// wc get added on init, after composed, we have to remove it
	if (wc_pos + 1 < 4 && buffer->steps[wc_pos + 1]) {
		free(buffer->steps[wc_pos + 1]);
		buffer->steps[wc_pos + 1] = NULL;
	}
}

static void daklakwl_buffer_steps_init(struct daklakwl_buffer *buffer)
{
	wchar_t *wc_text = buffer->wc_text;
	size_t wc_pos = buffer->wc_pos;
	size_t offset = 0;
	if (buffer->root)
		offset = 1;
	for (size_t i = 0; i < wc_pos; i++) {
		wchar_t c = wc_text[i];
		if (offset + i >= 4)
			break;
		if (c && is_vowel(c) && !buffer->steps[offset + i]) {
			buffer->steps[offset + i] = calloc(1, 1);
			buffer->steps[offset + i][0] = wctob(c);
		}
	}
}

static int daklakwl_buffer_compose_vowels(struct daklakwl_buffer *buf)
{
	if (buf->len == 0)
		return 0;
	daklakwl_buffer_steps_init(buf);

	size_t offset = 0;
	if (buf->root)
		offset = 1;
	wchar_t *wc_text = buf->wc_text;
	wchar_t c0 = '\0', c1 = '\0', c2 = '\0', c3 = '\0', c4 = '\0';
	size_t wc_pos = buf->wc_pos;

	c0 = wc_text[0];
	if (wc_pos >= 2)
		c1 = wc_text[1];
	if (wc_pos >= 3)
		c2 = wc_text[2];
	if (wc_pos >= 4)
		c3 = wc_text[3];
	if (wc_pos >= 5)
		c4 = wc_text[4];

	if (is_vowel(c0) && is_vowel(c1) && !is_vowel(c2) && !is_vowel(c3) &&
	    is_mark(towlower(c4)) && wc_pos == 5) {
		char const *const *marks = vowels_marks[c1];
		if (marks && marks[towlower(c4)]) {
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_append(buf, marks[towlower(c4)]);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_steps_append(buf, offset + 1, c4);
			return 1;
		}
	} else if (is_vowel(c0) && is_vowel(c1) && is_vowel(c2) &&
		   !is_vowel(c3) && is_accent(towlower(c4)) && wc_pos == 5) {
		// TODO: shall we?
		char const *const *accents = vowels_accents[c1];
		if (accents && accents[towlower(c4)]) {
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_append(buf, accents[towlower(c4)]);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_steps_append(buf, offset + 1, c4);
			return 1;
		}
	} else if (is_vowel(c0) && is_vowel(c1) && is_vowel(c2) &&
		   !is_vowel(c3) && is_mark(towlower(c4)) && wc_pos == 5) {
		if (is_type2(c0, c1, c2) || strcasecmp(buf->gi, "gi") == 0 ||
		    strcasecmp(buf->gi, "qu") == 0) {
			char const *const *marks = vowels_marks[c2];
			if (marks && marks[towlower(c4)]) {
				daklakwl_buffer_delete_backwards(buf, 1);
				daklakwl_buffer_move_left(buf);
				daklakwl_buffer_delete_backwards(buf, 1);
				daklakwl_buffer_append(buf,
						       marks[towlower(c4)]);
				daklakwl_buffer_move_right(buf);
				daklakwl_buffer_steps_append(buf, offset + 2,
							     c4);
				return 1;
			}
		} else {
			char const *const *marks = vowels_marks[c1];
			if (marks && marks[towlower(c4)]) {
				daklakwl_buffer_delete_backwards(buf, 1);
				daklakwl_buffer_move_left(buf);
				daklakwl_buffer_move_left(buf);
				daklakwl_buffer_delete_backwards(buf, 1);
				daklakwl_buffer_append(buf,
						       marks[towlower(c4)]);
				daklakwl_buffer_move_right(buf);
				daklakwl_buffer_move_right(buf);
				daklakwl_buffer_steps_append(buf, offset + 1,
							     c4);
				return 1;
			}
		}
	} else if (is_vowel(c0) && is_vowel(c1) && is_vowel(c2) &&
		   is_accent(towlower(c3)) && wc_pos == 4) {
		// TODO: handle uouw and uyee
		char const *const *c1_accents = vowels_accents[c1];
		char const *const *c2_accents = vowels_accents[c2];
		if (c1_accents && c1_accents[c3]) {
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_append(buf, c1_accents[towlower(c3)]);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_steps_append(buf, offset + 1, c3);
			return 1;
		} else if (c2_accents && c2_accents[c3]) {
			daklakwl_buffer_delete_backwards(buf, 2);
			daklakwl_buffer_append(buf, c2_accents[towlower(c3)]);
			daklakwl_buffer_steps_append(buf, offset + 2, c3);
			return 1;
		}
	} else if (is_vowel(c0) && is_vowel(c1) && is_vowel(c2) &&
		   is_mark(towlower(c3)) && wc_pos == 4) {
		if (is_type2(c0, c1, c2)) {
			char const *const *marks = vowels_marks[c2];
			if (marks && marks[towlower(c3)]) {
				daklakwl_buffer_delete_backwards(buf, 2);
				daklakwl_buffer_append(buf,
						       marks[towlower(c3)]);
				daklakwl_buffer_steps_append(buf, offset + 2,
							     c3);
				return 1;
			}
		} else {
			char const *const *marks = vowels_marks[c1];
			if (marks && marks[towlower(c3)]) {
				daklakwl_buffer_delete_backwards(buf, 1);
				daklakwl_buffer_move_left(buf);
				daklakwl_buffer_delete_backwards(buf, 1);
				daklakwl_buffer_append(buf,
						       marks[towlower(c3)]);
				daklakwl_buffer_move_right(buf);
				daklakwl_buffer_steps_append(buf, offset + 1,
							     c3);
				return 1;
			}
		}
	} else if (is_vowel(c0) && is_vowel(c1) && !is_vowel(c2) &&
		   is_mark(towlower(c3)) && wc_pos == 4) {
		char const *const *marks = vowels_marks[c1];
		if (marks && marks[towlower(c3)]) {
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_append(buf, marks[towlower(c3)]);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_steps_append(buf, offset + 1, c3);
			return 1;
		}
	} else if (is_vowel(c0) && is_vowel(c1) && !is_vowel(c2) &&
		   is_accent(towlower(c3)) && wc_pos == 4) {
		char const *const *accents = vowels_accents[c1];
		if (accents && accents[towlower(c3)]) {
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_append(buf, accents[towlower(c3)]);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_steps_append(buf, offset + 1, c3);
			return 1;
		}
	} else if (is_vowel(c0) && !is_vowel(c1) && !is_vowel(c2) &&
		   is_mark(towlower(c3)) && wc_pos == 4) {
		char const *const *marks = vowels_marks[c0];
		if (marks && marks[towlower(c3)]) {
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_append(buf, marks[towlower(c3)]);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_steps_append(buf, offset + 0, c3);
			return 1;
		}
	} else if (is_vowel(c0) && !is_vowel(c1) && !is_vowel(c2) &&
		   is_accent(towlower(c3)) && wc_pos == 4) {
		char const *const *accents = vowels_accents[c0];
		if (accents && accents[towlower(c3)]) {
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_append(buf, accents[towlower(c3)]);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_steps_append(buf, offset + 0, c3);
			return 1;
		}
	} else if (is_vowel(c0) && !is_vowel(c1) && is_accent(towlower(c2)) &&
		   wc_pos == 3) {
		char const *const *accents = vowels_accents[c0];
		if (accents && accents[towlower(c2)]) {
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_append(buf, accents[towlower(c2)]);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_steps_append(buf, offset + 0, c2);
			return 1;
		}
	} else if (is_vowel(c0) && !is_vowel(c1) && is_mark(towlower(c2)) &&
		   wc_pos == 3) {
		char const *const *marks = vowels_marks[c0];
		if (marks && marks[towlower(c2)]) {
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_append(buf, marks[towlower(c2)]);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_steps_append(buf, offset + 0, c2);
			return 1;
		}
	} else if (is_vowel(c0) && is_vowel(c1) && is_accent(towlower(c2)) &&
		   wc_pos == 3) {
		char const *const *c0_accents = vowels_accents[c0];
		char const *const *c1_accents = vowels_accents[c1];
		if (c1_accents && c1_accents[towlower(c2)] && c0_accents &&
		    c0_accents[towlower(c2)] && towlower(c1) == L'o') {
			daklakwl_buffer_delete_backwards(buf, 2);
			daklakwl_buffer_append(buf, c1_accents[towlower(c2)]);
			daklakwl_buffer_steps_append(buf, offset + 1, c2);
			return 1;
		} else if (c1_accents && c1_accents[towlower(c2)] &&
			   c0_accents && c0_accents[towlower(c2)] &&
			   towlower(c0) == L'u') {
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_append(buf, c0_accents[towlower(c2)]);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_steps_append(buf, offset + 0, c2);
			return 1;
		} else if (c0_accents && c0_accents[towlower(c2)]) {
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_move_left(buf);
			daklakwl_buffer_delete_backwards(buf, 1);
			daklakwl_buffer_append(buf, c0_accents[towlower(c2)]);
			daklakwl_buffer_move_right(buf);
			daklakwl_buffer_steps_append(buf, offset + 0, c2);
			return 1;
		} else if (c1_accents && c1_accents[towlower(c2)]) {
			daklakwl_buffer_delete_backwards(buf, 2);
			daklakwl_buffer_append(buf, c1_accents[towlower(c2)]);
			daklakwl_buffer_steps_append(buf, offset + 1, c2);
			return 1;
		}
	} else if (is_vowel(c0) && is_vowel(c1) && is_mark(towlower(c2)) &&
		   wc_pos == 3) {
		if (strcasecmp(buf->gi, "gi") == 0 ||
		    strcasecmp(buf->gi, "qu") == 0 || towlower(c1) == L'ơ') {
			char const *const *marks = vowels_marks[c1];
			if (marks && marks[towlower(c2)]) {
				daklakwl_buffer_delete_backwards(buf, 2);
				daklakwl_buffer_append(buf,
						       marks[towlower(c2)]);
				daklakwl_buffer_steps_append(buf, offset + 1,
							     c2);
				return 1;
			}
		} else {
			char const *const *marks = vowels_marks[c0];
			if (marks && marks[towlower(c2)]) {
				daklakwl_buffer_delete_backwards(buf, 1);
				daklakwl_buffer_move_left(buf);
				daklakwl_buffer_delete_backwards(buf, 1);
				daklakwl_buffer_append(buf,
						       marks[towlower(c2)]);
				daklakwl_buffer_move_right(buf);
				daklakwl_buffer_steps_append(buf, offset + 0,
							     c2);
				return 1;
			}
		}
	} else if (is_vowel(c0) && is_accent(towlower(c1)) && wc_pos == 2) {
		char const *const *accents = vowels_accents[c0];
		if (accents && accents[towlower(c1)]) {
			daklakwl_buffer_delete_backwards(buf, 2);
			daklakwl_buffer_append(buf, accents[towlower(c1)]);
			daklakwl_buffer_steps_append(buf, offset + 0, c1);
			return 1;
		}
	} else if (is_vowel(c0) && is_mark(towlower(c1)) && wc_pos == 2) {
		char const *const *marks = vowels_marks[c0];
		if (marks && marks[towlower(c1)]) {
			daklakwl_buffer_delete_backwards(buf, 2);
			daklakwl_buffer_append(buf, marks[towlower(c1)]);
			daklakwl_buffer_steps_append(buf, offset + 0, c1);
			return 1;
		}
	}
	return 0;
}

static char const *const d_accents[] = {
    [L'd'] = "đ",
    [L'D'] = "Đ",
};

static int daklakwl_buffer_compose_dd(struct daklakwl_buffer *buf)
{
	wchar_t *wc_text = buf->wc_text;
	size_t wc_len = buf->wc_len;
	size_t wc_pos = buf->wc_pos;
	if (wc_len <= 1)
		return 0;
	wchar_t c0 = wc_text[0];
	wchar_t cN = wc_text[wc_pos - 1];
	if (towlower(c0) == L'd' && towlower(cN) == L'd') {
		daklakwl_buffer_delete_backwards(buf, 1);
		for (size_t i = wc_pos - 2; i > 0; i--) {
			daklakwl_buffer_move_left(buf);
		}
		daklakwl_buffer_delete_backwards(buf, 1);
		daklakwl_buffer_append(buf, d_accents[c0]);
		for (size_t i = wc_pos - 2; i > 0; i--) {
			daklakwl_buffer_move_right(buf);
		}
		free(buf->gi);
		buf->gi = strdup(d_accents[c0]);
		buf->steps[0] = realloc(buf->steps[0], 3);
		buf->steps[0][1] = wctob(cN);
		return 1;
	}
	return 0;
}

static int daklakwl_buffer_compose_full(struct daklakwl_buffer *buf)
{
	if ((strcasecmp(buf->gi, "d") == 0 || strcasecmp(buf->gi, "đ") == 0) &&
	    buf->len > 1) {
		if (!daklakwl_buffer_compose_dd(buf)) {
			size_t gi_len = strlen(buf->gi);
			struct daklakwl_buffer sub_buf = {
			    .root = buf->text,
			    .text = buf->text + gi_len,
			    .len = buf->len - gi_len,
			    .pos = buf->pos - gi_len,
			    .wc_len = buf->wc_len - 1,
			    .wc_pos = buf->wc_pos - 1,
			    .wc_text = mbsrdup(buf->text + gi_len),
			    .gi = "",
			};
			sub_buf.steps[0] = buf->steps[0];
			sub_buf.steps[1] = buf->steps[1];
			sub_buf.steps[2] = buf->steps[2];
			sub_buf.steps[3] = buf->steps[3];
			int composed = daklakwl_buffer_compose_vowels(&sub_buf);
			buf->steps[1] = sub_buf.steps[1];
			buf->steps[2] = sub_buf.steps[2];
			buf->steps[3] = sub_buf.steps[3];
			buf->len = sub_buf.len + gi_len;
			buf->pos = sub_buf.pos + gi_len;
			free(sub_buf.wc_text);
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
	wchar_t *wc_text = buffer->wc_text = mbsrdup(buffer->text);
	size_t wc_len = buffer->wc_len = mbslen(buffer->text);
	size_t wc_pos = wc_len;
	for (size_t start = buffer->len; wc_pos > 0; wc_pos--, start--) {
		for (; start > 0; start--) {
			if (wc_text[wc_pos] == buffer->text[start] ||
			    wc_text[wc_pos - 1] == buffer->text[start - 1]) {
				break;
			}
		}
		if (start == buffer->pos)
			break;
	}
	buffer->wc_pos = wc_pos;
	char cN = buffer->text[buffer->pos - 1];
	if (buffer->catalyst && buffer->catalyst == cN && is_mark(cN) &&
	    buffer->len == buffer->pos) {
		composed = 0;
	} else {
		if (strcasecmp(buffer->gi, "d") == 0) {
			buffer->steps[0] = calloc(1, 1);
			buffer->steps[0][0] = buffer->gi[0];
		}
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
	free(wc_text);
}

bool daklakwl_buffer_should_not_append(struct daklakwl_buffer *buf,
				       const char *utf8)
{
	return !is_vowel(utf8[0]) && buf->len == 0 && towlower(utf8[0]) != 'd';
}
