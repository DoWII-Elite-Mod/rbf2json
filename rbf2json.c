/*
 * Converts Reric binary files (rbf) to human readable JSON.
 */

#include "rbf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flb.h"

int flb_copy(char *dst, const uint16_t idx)
{
	const uint16_t *flb_idx = (uint16_t*) &flb_data[(idx+1)<<2];
	if ((idx+1) > flb_num_keys || *flb_idx > flb_data_len) {
		fprintf(stderr, "invalid flb index: %d\n", idx+1);
		return -1;
	}
	if (!strcpy(dst, (const char *) (flb_data + *flb_idx + 4)))
		return 1;
	return 0;
}

int rbf_copy_str(char *dst, int *p)
{
	int i, len = *p;
	memcpy(dst, p+1, len);
	for (i = 0; i < len; ++i)
		if (dst[i] == '\\')
			dst[i] = '/';
	return 0;
}

int rbf_load(FILE *rbf, struct rbf_hdr *hdr, struct rbf_buf_ptrs *buf)
{
	int rc;
	fread(hdr, 40, 1, rbf);
	if (memcmp(hdr->version, RBF_VERSION_STR, 8) != 0)
		return -1;

	/* table data */
	buf->tbl = malloc(hdr->tbl_count * sizeof(struct rbf_tbl));
	rc  = fread(buf->tbl, sizeof(struct rbf_tbl), hdr->tbl_count, rbf);
	if (!rc)
		return -1;
	/* index lookup data */
	buf->idx = malloc(hdr->idx_count<<2);
	rc = fread(buf->idx, 4, hdr->idx_count, rbf);
	if (!rc)
		return -1;
	/* data */
	buf->data = malloc(hdr->data_count * 8);
	rc = fread(buf->data, sizeof(struct rbf_data), hdr->data_count, rbf);
	if (!rc)
		return -1;
	/* strings data */
	buf->strblk = malloc(hdr->str_len);
	rc = fread(buf->strblk, 1, hdr->str_len, rbf);
	if (rc != hdr->str_len || fgetc(rbf) != EOF)
		return -1;

	return 0;
}

int rbf_tbl_is_array(char *key_name)
{
	int i;
	for (i = 0; i < ARRAY_KEY_NAMES_SIZE; ++i)
		if (!strcmp(key_name, ARRAY_KEY_NAMES[i]))
			return 1;
	return 0;

}

int rbf_tbl2json(const struct rbf_buf_ptrs *buf, int idx, json_t *root)
{
	const struct rbf_tbl *tbl = &buf->tbl[idx];
	int i, child_idx;

	if(tbl->child_count == 0)
		return 0;

	if (tbl->child_count == 1) {
		if (json_is_array(root)) {
			json_t *tmp = json_object();
			rbf_entry2json(buf, tbl->child_idx, tmp);
			json_array_append(root, tmp);
			json_decref(tmp);
		} else
			rbf_entry2json(buf, tbl->child_idx, root);
		return 0;
	}

	for (i = 0; i < tbl->child_count; ++i) {
		child_idx = buf->idx[tbl->child_idx+i];
		if (json_is_array(root)) {
			json_t *tmp = json_object();
			rbf_entry2json(buf, child_idx, tmp);
			json_array_append(root, tmp);
			json_decref(tmp);
		} else
			rbf_entry2json(buf, child_idx, root);
	}

	return 0;
}

int rbf_entry2json(const struct rbf_buf_ptrs *buf, int idx, json_t *root)
{
	const struct rbf_data *entry = &buf->data[idx];
	char key_name[128];
	char str_buff[128];
	int rc = 0;
	json_t *child = NULL;

	memset(key_name, '\0', 128);
	memset(str_buff, '\0', 128);

	if ((rc = flb_copy(key_name, entry->key))) {
		fprintf(stderr, "error: something messed up in flb_copy\n");
		return rc;
	}

	switch(entry->type) {
	case(0):
		json_object_set_new(root, key_name, json_boolean(entry->ival));
		break;
	case(1):
		json_object_set_new(root, key_name, json_real(entry->fval));
		break;
	case(2):
		json_object_set_new(root, key_name, json_integer(entry->ival));
		break;
	case(3):
		rbf_copy_str(str_buff, (int*) &buf->strblk[entry->ival]);
		json_object_set_new(root, key_name, json_string(str_buff));
		break;
	case(4):
		if (rbf_tbl_is_array(key_name))
			child = json_array();
		else
			child = json_object();
		rbf_tbl2json(buf, entry->ival, child);
		json_object_set_new(root, key_name, child);
		break;
	default:
		fprintf(stderr, "unexpected type: %d\n", entry->type);
		rc = -1;
		break;
	}

	return rc;
}

int main(int argc, char *argv[])
{
	int ec = 0;
	struct rbf_hdr hdr;
	struct rbf_buf_ptrs buf = {0, 0, 0, 0};
	FILE *rbf = NULL;
	json_t *root = NULL;

	if (argc > 2) {
		fprintf(stderr, "usage: %s <input rbf file>\n", argv[0]);
		ec = EXIT_FAILURE;
		goto fail;
	}

	if (argc == 1)
		rbf = stdin; /* read from stdin */
	else
		rbf = fopen(argv[1], "rb");

	if (!rbf) {
		fprintf(stderr, "Unable to open rbf file for reading\n");
		ec = EXIT_FAILURE;
		goto fail;
	}

	if ((ec = rbf_load(rbf, &hdr, &buf))) {
		fprintf(stderr, "Invalid rbf file.\n");
		goto fail;
	}

	root = json_object();
	ec = rbf_tbl2json((const struct rbf_buf_ptrs *) &buf, 0, root);
	if (ec)
		puts("¯\\_(ツ)_/¯");
	else {
		char *json = json_dumps(root, JSON_OPTS);
		puts(json);
		free(json);
	}

fail:
	if(root) json_decref(root);
	if(buf.tbl) free(buf.tbl);
	if(buf.idx) free(buf.idx);
	if(buf.data) free(buf.data);
	if(buf.strblk) free(buf.strblk);
	if(rbf) fclose(rbf);

	exit(ec);
}
