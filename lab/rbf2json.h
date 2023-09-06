#ifndef RBF2JSON_H
#define RBF2JSON_H

#include <inttypes.h>
#include <jansson.h>

#define RBF_VERSION_STR "RBF V0.1"
#define JSON_OPTS (JSON_SORT_KEYS | JSON_INDENT(2) | JSON_REAL_PRECISION(6))

typedef uint32_t rbf_idx;
typedef char     rbf_strblk;

struct rbf_hdr {
	char version[8];
	uint32_t tbl_offset,
		 tbl_count,
		 idx_offset,
		 idx_count,
		 data_offset,
		 data_count,
		 str_offset,
		 str_len;
};

struct rbf_tbl {
	uint32_t child_count;
	uint32_t child_idx;
};

struct rbf_data {
	uint16_t type;
	uint16_t key;
	union {
		int32_t  ival;
		uint32_t uval;
		float    fval;
	};
};

struct rbf_buf_ptrs {
	struct rbf_tbl *tbl;
	struct rbf_data *data;
	rbf_idx *idx;
	rbf_strblk *strblk;
};

int rbf_tbl2json(const struct rbf_buf_ptrs *, int, json_t*);
int rbf_tbl_is_array(char *);
int rbf_entry2json(const struct rbf_buf_ptrs *, int, json_t*);
int flb_copy(char *, const uint16_t);
int rbf_copy_str(char *, int *);
int rbf_load(FILE *, struct rbf_hdr *, struct rbf_buf_ptrs *);
int rbf_load_buf(char *, struct rbf_hdr *, struct rbf_buf_ptrs *);
char * open_rbf(char*);

#endif
