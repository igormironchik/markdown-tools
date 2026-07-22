/* See LICENSE file for copyright and license details. */
#ifndef GRAPHEME_H
#define GRAPHEME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define GRAPHEME_INVALID_CODEPOINT UINT32_C(0xFFFD)

/* TODO call it simply "direction" without override */
enum grapheme_bidirectional_override {
	GRAPHEME_BIDIRECTIONAL_OVERRIDE_NEUTRAL,
	GRAPHEME_BIDIRECTIONAL_OVERRIDE_LTR,
	GRAPHEME_BIDIRECTIONAL_OVERRIDE_RTL,
};

void grapheme_bidirectional_get_line_embedding_levels(const uint_least32_t *,
                                                      size_t, int_least8_t *);

size_t grapheme_bidirectional_preprocess(const uint_least32_t *, size_t,
                                         enum grapheme_bidirectional_override,
                                         uint_least32_t *, size_t);
size_t
grapheme_bidirectional_preprocess_utf8(const char *, size_t,
                                       enum grapheme_bidirectional_override,
                                       uint_least32_t *, size_t);

size_t grapheme_bidirectional_reorder_line(const uint_least32_t *,
                                           const int_least8_t *, size_t,
                                           uint_least32_t *, size_t);
size_t grapheme_bidirectional_reorder_line_utf8(const char *,
                                                const int_least8_t *, size_t,
                                                char *, size_t);

size_t grapheme_decode_utf8(const char *, size_t, uint_least32_t *);
size_t grapheme_encode_utf8(uint_least32_t, char *, size_t);

bool grapheme_is_character_break(uint_least32_t, uint_least32_t,
                                 uint_least16_t *);

bool grapheme_is_lowercase(const uint_least32_t *, size_t, size_t *);
bool grapheme_is_titlecase(const uint_least32_t *, size_t, size_t *);
bool grapheme_is_uppercase(const uint_least32_t *, size_t, size_t *);

bool grapheme_is_lowercase_utf8(const char *, size_t, size_t *);
bool grapheme_is_titlecase_utf8(const char *, size_t, size_t *);
bool grapheme_is_uppercase_utf8(const char *, size_t, size_t *);

size_t grapheme_next_character_break(const uint_least32_t *, size_t);
size_t grapheme_next_line_break(const uint_least32_t *, size_t);
size_t grapheme_next_sentence_break(const uint_least32_t *, size_t);
size_t grapheme_next_word_break(const uint_least32_t *, size_t);

size_t grapheme_next_character_break_utf8(const char *, size_t);
size_t grapheme_next_line_break_utf8(const char *, size_t);
size_t grapheme_next_sentence_break_utf8(const char *, size_t);
size_t grapheme_next_word_break_utf8(const char *, size_t);

size_t grapheme_to_lowercase(const uint_least32_t *, size_t, uint_least32_t *,
                             size_t);
size_t grapheme_to_titlecase(const uint_least32_t *, size_t, uint_least32_t *,
                             size_t);
size_t grapheme_to_uppercase(const uint_least32_t *, size_t, uint_least32_t *,
                             size_t);

size_t grapheme_to_lowercase_utf8(const char *, size_t, char *, size_t);
size_t grapheme_to_titlecase_utf8(const char *, size_t, char *, size_t);
size_t grapheme_to_uppercase_utf8(const char *, size_t, char *, size_t);

#endif /* GRAPHEME_H */
