#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rbf2json.h"

int main(int argc, char *argv[])
{
	int ec = 0;
	FILE *rbf = 0;
    char *json = 0;
    long fsize = 0;
    char *buff = 0;

	if (argc > 2) {
		fprintf(stderr, "usage: %s <input rbf file>\n", argv[0]);
		ec = EXIT_FAILURE;
		goto fail;
	}

    rbf = fopen(argv[1], "rb");

	if (!rbf) {
		fprintf(stderr, "Unable to open rbf file for reading\n");
		ec = EXIT_FAILURE;
		goto fail;
	}

    fseek(rbf, 0, SEEK_END);
    fsize = ftell(rbf);
    fseek(rbf, 0, SEEK_SET);

    buff = malloc(fsize + 1);
    ec = fread(buff, fsize, 1, rbf);
    if(!ec) {
		fprintf(stderr, "Invalid rbf file.\n");
		goto fail;
    }

	if (memcmp(buff, RBF_VERSION_STR, 8) != 0) {
        puts("wrrooooong");
    }

	json = open_rbf(buff);

	if (!json) {
		fprintf(stderr, "Invalid rbf file.\n");
		goto fail;
	}

	if (!json)
		puts("¯\\_(ツ)_/¯");
	else 
		puts(json);

fail:
	if(json) free(json);
    if(buff) free(buff);
    if(rbf) fclose(rbf);

	exit(ec);
}
