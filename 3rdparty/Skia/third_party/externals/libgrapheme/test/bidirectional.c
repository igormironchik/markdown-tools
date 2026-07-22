/* See LICENSE file for copyright and license details. */
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "../gen/bidirectional-test.h"
#include "../gen/types.h"
#include "../grapheme.h"
#include "util.h"

int
main(int argc, char *argv[])
{
	uint_least32_t data[512]; /* TODO iterate and get max, allocate */
	int_least8_t lev[512];
	size_t i, num_tests, failed, datalen, ret, j, m;

	datalen = LEN(data);

	(void)argc;

	for (i = 0, num_tests = 0; i < LEN(bidirectional_test); i++) {
		num_tests += bidirectional_test[i].modelen;
	}

	for (i = 0, failed = 0; i < LEN(bidirectional_test); i++) {
		for (m = 0; m < bidirectional_test[i].modelen; m++) {
			ret = grapheme_bidirectional_preprocess(
				bidirectional_test[i].cp,
				bidirectional_test[i].cplen,
				bidirectional_test[i].mode[m], data, datalen);
			grapheme_bidirectional_get_line_embedding_levels(
				data, ret, lev);

			if (ret != bidirectional_test[i].cplen ||
			    ret > datalen) {
				goto err;
			}

			for (j = 0; j < ret; j++) {
				if (lev[j] != bidirectional_test[i].level[j]) {
					goto err;
				}
			}
			continue;
err:
			fprintf(stderr,
			        "%s: Failed conformance test %zu (mode %i) [",
			        argv[0], i, bidirectional_test[i].mode[m]);
			for (j = 0; j < bidirectional_test[i].cplen; j++) {
				fprintf(stderr, " 0x%04" PRIXLEAST32,
				        bidirectional_test[i].cp[j]);
			}
			fprintf(stderr, " ],\n\tgot      (");
			for (j = 0; j < ret; j++) {
				fprintf(stderr, " %" PRIdLEAST8,
				        (int_least8_t)lev[j]);
			}
			fprintf(stderr, " ),\n\texpected (");
			for (j = 0; j < ret; j++) {
				fprintf(stderr, " %" PRIdLEAST8,
				        bidirectional_test[i].level[j]);
			}
			fprintf(stderr, " ).\n");
			failed++;
		}
	}
	printf("%s: %zu/%zu conformance tests passed.\n", argv[0],
	       num_tests - failed, num_tests);

	return 0;
}
