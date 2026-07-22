/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define FILE_BIDI_BRACKETS  "data/BidiBrackets.txt"
#define FILE_BIDI_CLASS     "data/DerivedBidiClass.txt"
#define FILE_BIDI_MIRRORING "data/BidiMirroring.txt"
#define FILE_UNICODE_DATA   "data/UnicodeData.txt"

#define NUM_BRACKET_ALIASES 20

static const struct property_spec bidi_property[] = {
	{
		/* default */
		.enumname = "L",
		.file = FILE_BIDI_CLASS,
		.ucdname = "L",
	},
	{
		.enumname = "AL",
		.file = FILE_BIDI_CLASS,
		.ucdname = "AL",
	},
	{
		.enumname = "AN",
		.file = FILE_BIDI_CLASS,
		.ucdname = "AN",
	},
	{
		.enumname = "B",
		.file = FILE_BIDI_CLASS,
		.ucdname = "B",
	},
	{
		.enumname = "BN",
		.file = FILE_BIDI_CLASS,
		.ucdname = "BN",
	},
	{
		.enumname = "CS",
		.file = FILE_BIDI_CLASS,
		.ucdname = "CS",
	},
	{
		.enumname = "EN",
		.file = FILE_BIDI_CLASS,
		.ucdname = "EN",
	},
	{
		.enumname = "ES",
		.file = FILE_BIDI_CLASS,
		.ucdname = "ES",
	},
	{
		.enumname = "ET",
		.file = FILE_BIDI_CLASS,
		.ucdname = "ET",
	},
	{
		.enumname = "FSI",
		.file = FILE_BIDI_CLASS,
		.ucdname = "FSI",
	},
	{
		.enumname = "LRE",
		.file = FILE_BIDI_CLASS,
		.ucdname = "LRE",
	},
	{
		.enumname = "LRI",
		.file = FILE_BIDI_CLASS,
		.ucdname = "LRI",
	},
	{
		.enumname = "LRO",
		.file = FILE_BIDI_CLASS,
		.ucdname = "LRO",
	},
	{
		.enumname = "NSM",
		.file = FILE_BIDI_CLASS,
		.ucdname = "NSM",
	},
	{
		.enumname = "ON",
		.file = FILE_BIDI_CLASS,
		.ucdname = "ON",
	},
	{
		.enumname = "PDF",
		.file = FILE_BIDI_CLASS,
		.ucdname = "PDF",
	},
	{
		.enumname = "PDI",
		.file = FILE_BIDI_CLASS,
		.ucdname = "PDI",
	},
	{
		.enumname = "R",
		.file = FILE_BIDI_CLASS,
		.ucdname = "R",
	},
	{
		.enumname = "RLE",
		.file = FILE_BIDI_CLASS,
		.ucdname = "RLE",
	},
	{
		.enumname = "RLI",
		.file = FILE_BIDI_CLASS,
		.ucdname = "RLI",
	},
	{
		.enumname = "RLO",
		.file = FILE_BIDI_CLASS,
		.ucdname = "RLO",
	},
	{
		.enumname = "S",
		.file = FILE_BIDI_CLASS,
		.ucdname = "S",
	},
	{
		.enumname = "WS",
		.file = FILE_BIDI_CLASS,
		.ucdname = "WS",
	},
};

struct decomposition_payload {
	uint_least32_t cp;
	uint_least32_t decomposition;
};

static int
decomposition_callback(const char *file, char **field, size_t nfields,
                       char *comment, void *payload)
{
	char *p;
	struct decomposition_payload *decomp =
		(struct decomposition_payload *)payload;
	uint_least32_t cp;

	(void)file;
	(void)comment;

	if (nfields < 6) {
		/* we have fewer than 6 fields, discard the line */
		return 0;
	}

	hextocp(field[0], strlen(field[0]), &cp);

	if (decomp->cp == cp) {
		/* we hit the line that contains our decomposition target */
		if (strlen(field[5]) > 0) {
			p = field[5];
			if (*p == '<') {
				/*
				 * the decomposition contains some metadata
				 * <...> we skip
				 */
				for (; *p != '\0'; p++) {
					if (*p == '>') {
						p++;
						while (*p == ' ') {
							p++;
						}
						break;
					}
				}
			}
			hextocp(p, strlen(p), &(decomp->decomposition));
		} else {
			decomp->decomposition = decomp->cp;
		}
	}

	return 0;
}

static struct {
	uint_least32_t base[NUM_BRACKET_ALIASES];
	size_t baselen;
	uint_least32_t pair[NUM_BRACKET_ALIASES];
	size_t pairlen;
	uint_least8_t class;
	char type;
} *b = NULL;

static size_t blen;
static uint_least8_t bracket_class_count = 1;

static int
bracket_callback(const char *file, char **field, size_t nfields, char *comment,
                 void *payload)
{
	size_t i, j;
	struct decomposition_payload decomp_base, decomp_pair;
	uint_least32_t cp_base, cp_pair;

	(void)file;
	(void)comment;
	(void)payload;

	if (nfields < 3) {
		/* we have fewer than 3 fields, discard the line */
		return 0;
	}

	/* parse field data */
	hextocp(field[0], strlen(field[0]), &cp_base);
	hextocp(field[1], strlen(field[1]), &cp_pair);

	/* determine decomposition of the base and pair codepoints */
	decomp_base.cp = cp_base;
	decomp_pair.cp = cp_pair;
	parse_file_with_callback(FILE_UNICODE_DATA, decomposition_callback,
	                         &decomp_base);
	parse_file_with_callback(FILE_UNICODE_DATA, decomposition_callback,
	                         &decomp_pair);

	/*
	 * check if we already have the canonical form in the bracket array,
	 * per convention the canonical form is the first element of the alias
	 * array
	 */
	for (i = 0; i < blen; i++) {
		if (decomp_base.decomposition == b[i].base[0]) {
			/* we have a match, check type */
			if (strlen(field[2]) != 1 ||
			    (field[2][0] != 'o' && field[2][0] != 'c')) {
				/* malformed line */
				return 1;
			} else if (b[i].type != field[2][0]) {
				/* mismatching types */
				return 1;
			}

			/*
			 * add our base alias to the base array unless it isn't
			 * already in it
			 */
			for (j = 0; j < b[i].baselen; j++) {
				if (cp_base == b[i].base[j]) {
					/* already in array, do nothing */
					break;
				}
			}
			if (j == b[i].baselen) {
				/*
				 * the base alias is not already in the array,
				 * add it
				 */
				if (b[i].baselen == NUM_BRACKET_ALIASES) {
					fprintf(stderr, "too many aliases\n");
					return 1;
				}
				b[i].baselen++;
				b[i].base[b[i].baselen - 1] = cp_base;
			}

			/*
			 * also add our pair alias to the pair array unless
			 * it isn't already in it
			 */
			for (j = 0; j < b[i].pairlen; j++) {
				if (cp_pair == b[i].pair[j]) {
					/* already in array, do nothing */
					break;
				}
			}
			if (j == b[i].pairlen) {
				/*
				 * the pair alias is not already in the array,
				 * add it
				 */
				if (b[i].pairlen == NUM_BRACKET_ALIASES) {
					fprintf(stderr, "too many aliases\n");
					return 1;
				}
				b[i].pairlen++;
				b[i].pair[b[i].pairlen - 1] = cp_pair;
			}

			return 0;
		}
	}

	/* extend bracket pair array, as this is a new bracket type */
	if (!(b = realloc(b, (++blen) * sizeof(*b)))) {
		fprintf(stderr, "realloc: %s\n", strerror(errno));
		exit(1);
	}

	/* fill field data by adding the canonical form first */
	b[blen - 1].base[0] = decomp_base.decomposition;
	b[blen - 1].baselen = 1;
	b[blen - 1].pair[0] = decomp_pair.decomposition;
	b[blen - 1].pairlen = 1;

	/* add alias if it differs from the canonical form */
	if (cp_base != decomp_base.decomposition) {
		b[blen - 1].base[1] = cp_base;
		b[blen - 1].baselen = 2;
	}
	if (cp_pair != decomp_pair.decomposition) {
		b[blen - 1].pair[1] = cp_pair;
		b[blen - 1].pairlen = 2;
	}

	/* add bracket type */
	if (strlen(field[2]) != 1 ||
	    (field[2][0] != 'o' && field[2][0] != 'c')) {
		/* malformed line */
		return 1;
	} else {
		b[blen - 1].type = field[2][0];
	}

	/*
	 * determine bracket class by iterating over the bracket-array
	 * and seeing if our current canonical cp already has a matching pair.
	 * We only need to check the first entry in each bracket alias
	 * list, as this is, per convention, the canonical form.
	 * If not, add a new class.
	 */
	for (i = 0; i + 1 < blen; i++) {
		if (b[i].pair[0] == b[blen - 1].base[0]) {
			/* matched class */
			b[blen - 1].class = b[i].class;
			break;
		}
	}
	if (i + 1 == blen) {
		/* no match, assign a new class */
		b[blen - 1].class = bracket_class_count++;
	}

	return 0;
}

static void
post_process(struct properties *prop)
{
	size_t i, j;

	for (i = 0; i < blen; i++) {
		/*
		 * given the base property fits in 5 bits, we simply
		 * store the bracket-offset in the bits above that.
		 *
		 * All those properties that are not set here implicitly
		 * have offset 0, which we prepared to contain a stub
		 * for a character that is not a bracket.
		 */
		for (j = 0; j < b[i].baselen; j++) {
			prop[b[i].base[j]].property |= (i << 5);
		}
	}
}

static uint_least8_t
fill_missing(uint_least32_t cp)
{
	/* based on the @missing-properties in data/DerivedBidiClass.txt */
	if ((cp >= UINT32_C(0x0590) && cp <= UINT32_C(0x05FF)) ||
	    (cp >= UINT32_C(0x07C0) && cp <= UINT32_C(0x085F)) ||
	    (cp >= UINT32_C(0xFB1D) && cp <= UINT32_C(0xFB4F)) ||
	    (cp >= UINT32_C(0x10800) && cp <= UINT32_C(0x10CFF)) ||
	    (cp >= UINT32_C(0x10D40) && cp <= UINT32_C(0x10EBF)) ||
	    (cp >= UINT32_C(0x10F00) && cp <= UINT32_C(0x10F2F)) ||
	    (cp >= UINT32_C(0x10F70) && cp <= UINT32_C(0x10FFF)) ||
	    (cp >= UINT32_C(0x1E800) && cp <= UINT32_C(0x1EC6F)) ||
	    (cp >= UINT32_C(0x1ECC0) && cp <= UINT32_C(0x1ECFF)) ||
	    (cp >= UINT32_C(0x1ED50) && cp <= UINT32_C(0x1EDFF)) ||
	    (cp >= UINT32_C(0x1EF00) && cp <= UINT32_C(0x1EFFF))) {
		return 17; /* class R */
	} else if ((cp >= UINT32_C(0x0600) && cp <= UINT32_C(0x07BF)) ||
	           (cp >= UINT32_C(0x0860) && cp <= UINT32_C(0x08FF)) ||
	           (cp >= UINT32_C(0xFB50) && cp <= UINT32_C(0xFDCF)) ||
	           (cp >= UINT32_C(0xFDF0) && cp <= UINT32_C(0xFDFF)) ||
	           (cp >= UINT32_C(0xFE70) && cp <= UINT32_C(0xFEFF)) ||
	           (cp >= UINT32_C(0x10D00) && cp <= UINT32_C(0x10D3F)) ||
	           (cp >= UINT32_C(0x10EC0) && cp <= UINT32_C(0x10EFF)) ||
	           (cp >= UINT32_C(0x10F30) && cp <= UINT32_C(0x10F6F)) ||
	           (cp >= UINT32_C(0x1EC70) && cp <= UINT32_C(0x1ECBF)) ||
	           (cp >= UINT32_C(0x1ED00) && cp <= UINT32_C(0x1ED4F)) ||
	           (cp >= UINT32_C(0x1EE00) && cp <= UINT32_C(0x1EEFF))) {
		return 1; /* class AL */
	} else if (cp >= UINT32_C(0x20A0) && cp <= UINT32_C(0x20CF)) {
		return 8; /* class ET */
	} else {
		return 0; /* class L */
	}
}

int
main(int argc, char *argv[])
{
	size_t i;

	(void)argc;

	/*
	 * the first element in the bracket array is initialized to
	 * all-zeros, as we use the implicit 0-offset for all those
	 * codepoints that are not a bracket
	 */
	if (!(b = calloc((blen = 1), sizeof(*b)))) {
		fprintf(stderr, "calloc: %s\n", strerror(errno));
		exit(1);
	}
	parse_file_with_callback(FILE_BIDI_BRACKETS, bracket_callback, NULL);

	properties_generate_break_property(bidi_property, LEN(bidi_property),
	                                   fill_missing, NULL, post_process,
	                                   "bidi", argv[0]);

	printf("\nenum bracket_type {\n\tBIDI_BRACKET_NONE,\n\t"
	       "BIDI_BRACKET_OPEN,\n\tBIDI_BRACKET_CLOSE,\n};\n\n"
	       "static const struct bracket {\n\tenum bracket_type type;\n"
	       "\tuint_least8_t class;\n} bidi_bracket[] = {\n");
	for (i = 0; i < blen; i++) {
		printf("\t{\n\t\t.type = %s,\n\t\t.class = "
		       "%" PRIuLEAST8 ",\n\t},\n",
		       (b[i].type == 'o') ? "BIDI_BRACKET_OPEN" :
		       (b[i].type == 'c') ? "BIDI_BRACKET_CLOSE" :
		                            "BIDI_BRACKET_NONE",
		       b[i].class);
	}
	printf("};\n");

	return 0;
}
