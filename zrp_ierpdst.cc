#include <zrp/zrp_ierpdst.h>

// Query Cache Entry

query_cache_entry::query_cache_entry() {
//	qc_src = 0;
	query_id = 0;
	LIST_INIT(&prev_hop);
	hop_count = 0;
	injection_counter = 0;
}

query_cache_entry::~query_cache_entry() {
	// Nothing to do.
}

// Query Cache




