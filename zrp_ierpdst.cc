#include <zrp/zrp_ierpdst.h>

// Query Cache Entry

query_cache_entry::query_cache_entry() {
	qc_src = 0;
	query_id = 0;
	hop_count = 0;
	injection_counter = 0;
}

query_cache_entry::~query_cache_entry() {
	// Nothing to do.
}

// Query Cache

query_cache_entry*
query_cache::qc_insert(nsaddr_t query_src) {
	query_cache_entry *qe = qc_lookup(query_src);
	if (qe == 0) {
		qe = new query_cache_entry;
		assert(qe);
		qe->qc_src = query_src;
		LIST_INSERT_HEAD(&qc_head, qe, qc_link);
	}
	return qe;
}

void
query_cache::qc_delete(nsaddr_t query_src) {
	query_cache_entry *qe = qc_lookup(query_src);
	if (qe) {
		LIST_REMOVE(qe, qc_link);
		delete qe;
	}
}

query_cache_entry*
query_cache::qc_lookup(nsaddr_t query_src) {
	query_cache_entry *qe = qc_head.lh_first;

	for(; qe; qe= qe->qc_link.le_next) {
		if(qe->qc_src == query_src)
			break;
	}

	return qe;
}

void
query_cache::qc_dump() {
	query_cache_entry *qe = qc_head.lh_first;

	printf("Dumping Query Cache -- ");
	for(; qe; qe= qe->qc_link.le_next) {
		printf(" { s : %d - id : %d - prevhop %d - hpcnt : %d - ic %d } ", qe->qc_src, qe->query_id, 0,qe->hop_count, qe->injection_counter);
	}
	printf(" -- done \n");
}
