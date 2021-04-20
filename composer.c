#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "mbstrings.h"

static char const *const vowel_a_marks[] = {
	['s'] = "á",
	['f'] = "à",
	['x'] = "ã",
	['r'] = "ả",
	['j'] = "ạ",
};
static char const *const vowel_aw_marks[] = {
	['s'] = "ắ",
	['f'] = "ằ",
	['x'] = "ẵ",
	['r'] = "ẳ",
	['j'] = "ặ",
};
static char const *const vowel_aa_marks[] = {
	['s'] = "ấ",
	['f'] = "ầ",
	['x'] = "ẫ",
	['r'] = "ẩ",
	['j'] = "ậ",
};
static char const *const vowel_e_marks[] = {
	['s'] = "é",
	['f'] = "è",
	['x'] = "ẽ",
	['r'] = "ẻ",
	['j'] = "ẹ",
};
static char const *const vowel_ee_marks[] = {
	['s'] = "ế",
	['f'] = "ề",
	['x'] = "ễ",
	['r'] = "ể",
	['j'] = "ệ",
};
static char const *const vowel_i_marks[] = {
	['s'] = "í",
	['f'] = "ì",
	['x'] = "ĩ",
	['r'] = "ỉ",
	['j'] = "ị",
};
static char const *const vowel_o_marks[] = {
	['s'] = "ó",
	['f'] = "ò",
	['x'] = "õ",
	['r'] = "ỏ",
	['j'] = "ọ",
};
static char const *const vowel_oo_marks[] = {
	['s'] = "ố",
	['f'] = "ồ",
	['x'] = "ỗ",
	['r'] = "ổ",
	['j'] = "ộ",
};
static char const *const vowel_ow_marks[] = {
	['s'] = "ớ",
	['f'] = "ờ",
	['x'] = "ỡ",
	['r'] = "ở",
	['j'] = "ợ",
};
static char const *const vowel_u_marks[] = {
	['s'] = "ú",
	['f'] = "ù",
	['x'] = "ũ",
	['r'] = "ủ",
	['j'] = "ụ",
};
static char const *const vowel_uw_marks[] = {
	['s'] = "ứ",
	['f'] = "ừ",
	['x'] = "ữ",
	['r'] = "ử",
	['j'] = "ự",
};
static char const *const vowel_y_marks[] = {
	['s'] = "ý",
	['f'] = "ỳ",
	['x'] = "ỹ",
	['r'] = "ỷ",
	['j'] = "ỵ",
};

static char const *const *const vowels[7929] = {
	[L'a'] = vowel_a_marks,
	[L'ă'] = vowel_aw_marks,
	[L'â'] = vowel_aa_marks,
	[L'e'] = vowel_e_marks,
	[L'ê'] = vowel_ee_marks,
	[L'i'] = vowel_i_marks,
	[L'o'] = vowel_o_marks,
	[L'ô'] = vowel_oo_marks,
	[L'ơ'] = vowel_ow_marks,
	[L'u'] = vowel_u_marks,
	[L'ư'] = vowel_uw_marks,
	[L'y'] = vowel_y_marks,
};

static int const accents[128] = {
	[L'a'] = 1,
	[L'e'] = 1,
	[L'o'] = 1,
	[L'w'] = 1,
};

static char const *const vowels_accents[7929+128] = {
	[L'a'+L'a'] = "â",
	[L'a'+L'w'] = "ă",
	[L'e'+L'e'] = "ê",
	[L'o'+L'o'] = "ô",
	[L'o'+L'w'] = "ơ",
	[L'u'+L'w'] = "ư",
};

static int const marks[128] = {
	[L's'] = 1,
	[L'f'] = 1,
	[L'x'] = 1,
	[L'r'] = 1,
	[L'j'] = 1,
};

static int const type1[7929*2] = {
	[L'â'] = 1,
	[L'i'+L'ê'] = 1,
	[L'u'+L'â'] = 1,
	[L'u'+L'ô'] = 1,
	[L'ư'+L'ơ'] = 1,
	[L'y'+L'ê'] = 1,
};
static int const type2[7929*3] = {
	[L'ă'] = 1,
	[L'o'+L'ă'] = 1,
	[L'o'+L'o'] = 1,
	[L'u'+L'ă'] = 1,
	[L'u'+L'y'+L'ê'] = 1,
};

struct buffer {
	char *text;
	size_t pos;
	size_t len;
};

int compose(struct buffer *buf) {
	if (buf->len == 0)
		return 0;
	wchar_t *wc_text = mbsrdup(buf->text, NULL);
	wchar_t c0 = '\0', c1 = '\0', c2 = '\0', c3 = '\0', c4 = '\0';
	c0 = wc_text[0];
	if (buf->len >= 2) c1 = wc_text[1];
	if (buf->len >= 3) c2 = wc_text[2];
	if (buf->len >= 4) c3 = wc_text[3];
	if (buf->len >= 5) c4 = wc_text[4];

	if (vowels[c0] && vowels[c1] && vowels[c2] && !vowels[c3] && accents[c4]) {
		// TODO: shall we?
		if (vowels_accents[c1 + c3]) {
			char *c0_override = (char *) calloc(1, sizeof(char));
			char *c2_override = (char *) calloc(1, sizeof(char));
			char *c3_override = (char *) calloc(1, sizeof(char));
			wctomb(c0_override, c0);
			wctomb(c2_override, c2);
			wctomb(c3_override, c3);
			strcat(c0_override, vowels_accents[c1 + c3]);
			strcat(c0_override, c2_override);
			strcat(c0_override, c3_override);
			free(c2_override);
			buf->text = c0_override;
			return 1;
		}
	}
	else if (vowels[c0] && vowels[c1] && vowels[c2] && !vowels[c3] && marks[c4]) {
		if (type2[c0 + c1 + c2]) {
			char const *const *vowels_marks = vowels[c2];
			if (vowels_marks[c4]) {
				char *c0_override = (char *) calloc(1, sizeof(char));
				char *c1_override = (char *) calloc(1, sizeof(char));
				char *c3_override = (char *) calloc(1, sizeof(char));
				wctomb(c0_override, c0);
				wctomb(c1_override, c1);
				wctomb(c3_override, c3);
				strcat(c0_override, c1_override);
				strcat(c0_override, vowels_marks[c3]);
				strcat(c0_override, c3_override);
				free(c1_override);
				free(c3_override);
				buf->text = c0_override;
				return 1;
			}
		}
		else {
			char const *const *vowels_marks = vowels[c1];
			if (vowels_marks[c4]) {
				char *c0_override = (char *) calloc(1, sizeof(char));
				char *c2_override = (char *) calloc(1, sizeof(char));
				wctomb(c0_override, c0);
				wctomb(c2_override, c2);
				strcat(c0_override, vowels_marks[c3]);
				strcat(c0_override, c2_override);
				free(c2_override);
				buf->text = c0_override;
				return 1;
			}

		}
	}
	else if (vowels[c0] && vowels[c1] && vowels[c2] && accents[c3]) {
		// TODO: handle uouw and uyee
		if (vowels_accents[c1 + c3]) {
			char *c0_override = (char *) calloc(1, sizeof(char));
			char *c2_override = (char *) calloc(1, sizeof(char));
			wctomb(c0_override, c0);
			wctomb(c2_override, c2);
			strcat(c0_override, vowels_accents[c1 + c3]);
			strcat(c0_override, c2_override);
			free(c2_override);
			buf->text = c0_override;
			return 1;
		}
	}
	else if (vowels[c0] && vowels[c1] && vowels[c2] && marks[c3]) {
		if (type2[c0 + c1 + c2]) {
			char const *const *vowels_marks = vowels[c2];
			if (vowels_marks[c3]) {
				char *c0_override = (char *) calloc(1, sizeof(char));
				char *c1_override = (char *) calloc(1, sizeof(char));
				wctomb(c0_override, c0);
				wctomb(c1_override, c1);
				strcat(c0_override, c1_override);
				strcat(c0_override, vowels_marks[c3]);
				free(c1_override);
				buf->text = c0_override;
				return 1;
			}
		}
		else {
			char const *const *vowels_marks = vowels[c1];
			if (vowels_marks[c3]) {
				char *c0_override = (char *) calloc(1, sizeof(char));
				char *c2_override = (char *) calloc(1, sizeof(char));
				wctomb(c0_override, c0);
				wctomb(c2_override, c2);
				strcat(c0_override, vowels_marks[c3]);
				strcat(c0_override, c2_override);
				free(c2_override);
				buf->text = c0_override;
				return 1;
			}

		}
	}
	else if (vowels[c0] && vowels[c1] && !vowels[c2] && marks[c3]) {
		if (type1[c0 + c1] || type2[c0 + c1]) {
			char const *const *vowels_marks = vowels[c1];
			if (vowels_marks[c2]) {
				char *c0_override = (char *) calloc(1, sizeof(char));
				char *c2_override = (char *) calloc(1, sizeof(char));
				wctomb(c0_override, c0);
				wctomb(c2_override, c2);
				strcat(c0_override, vowels_marks[c3]);
				strcat(c0_override, c2_override);
				buf->text = c0_override;
				free(c2_override);
				return 1;
			}
		}
		else {
			char const *const *vowels_marks = vowels[c0];
			if (vowels_marks[c2]) {
				char *c0_override = (char *) calloc(strlen(vowels_marks[c2]), sizeof(char));
				char *c1_override = (char *) calloc(1, sizeof(char));
				char *c2_override = (char *) calloc(1, sizeof(char));
				wctomb(c1_override, c1);
				wctomb(c2_override, c2);
				strcat(strcpy(c0_override, vowels_marks[c2]), c1_override);
				strcat(c0_override, c2_override);
				buf->text = c0_override;
				free(c2_override);
				return 1;
			}
		}
	}
	else if (vowels[c0] && vowels[c1] && !vowels[c2] && accents[c3]) {
		if (vowels_accents[c1 + c3]) {
			char *c0_override = (char *) calloc(1, sizeof(char));
			wctomb(c0_override, c0);
			char *c2_override = (char *) calloc(1, sizeof(char));
			wctomb(c2_override, c2);
			strcat(c0_override, vowels_accents[c1 + c3]);
			strcat(c0_override, c2_override);
			buf->text = (char *)c0_override;
			return 1;
		}
	}
	else if (vowels[c0] && !vowels[c1] && accents[c2]) {
		if (vowels_accents[c0 + c2 ]) {
			char *c0_override = (char *) calloc(strlen(vowels_accents[c0 + c2]), sizeof(char));
			char *c1_override = (char *) calloc(1, sizeof(char));
			wctomb(c1_override, c1);
			buf->text = strcat(strcpy(c0_override, vowels_accents[c0 + c2]), c1_override);
			free(c1_override);
			return 1;
		}
	}
	else if (vowels[c0] && !vowels[c1] && marks[c2]) {
		char const *const *vowels_marks = vowels[c0];
		if (vowels_marks[c2]) {
			char *c0_override = (char *) calloc(strlen(vowels_marks[c2]), sizeof(char));
			char *c1_override = (char *) calloc(1, sizeof(char));
			wctomb(c1_override, c1);
			buf->text = strcat(strcpy(c0_override, vowels_marks[c2]), c1_override);
			free(c1_override);
			//buf->mark_pos = 1
			//buf->mark = c2
			return 1;
		}
	}
	else if (vowels[c0] && vowels[c1] && accents[c2]) {
		if (vowels_accents[c1+c2]) {
			char *c0_override = (char *) calloc(1, sizeof(char));
			wctomb(c0_override, c0);
			buf->text = strcat(c0_override, vowels_accents[c1 + c2]);
			return 1;
		}
	}
	else if (vowels[c0] && vowels[c1] && marks[c2]) {
		if (type1[c0 + c1] || type2[c0 + c1]) {
			char const *const *vowels_marks = vowels[c1];
			if (vowels_marks[c2]) {
				char *c0_override = (char *) calloc(1, sizeof(char));
				wctomb(c0_override, c0);
				buf->text = strcat(c0_override, vowels_marks[c2]);
				return 1;
			}
		}
		else {
			char const *const *vowels_marks = vowels[c0];
			if (vowels_marks[c2]) {
				char *c0_override = (char *) calloc(strlen(vowels_marks[c2]), sizeof(char));
				char *c1_override = (char *) calloc(1, sizeof(char));
				wctomb(c1_override, c1);
				buf->text = strcat(strcpy(c0_override, vowels_marks[c2]), c1_override);
				return 1;
			}
		}
	}
	else if (vowels[c0] && accents[c1]) {
		if (vowels_accents[c0 + c1]) {
			buf->text = strdup(vowels_accents[c0 + c1]);
			return 1;
		}
	}
	else if (vowels[c0] && marks[c1]) {
		char const *const *vowels_marks = vowels[c0];
		if (vowels_marks[c1]) {
			buf->text = strdup(vowels_marks[c1]);
			buf->len -= 1;
			return 1;
		}
	}
	free(wc_text);
	return 0;
}
