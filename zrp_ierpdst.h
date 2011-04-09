/*
 * zrp_ierpdst.h
 */

#ifndef ZRP_IERPDST_H_
#define ZRP_IERPDST_H_

#include <zrp/zrp_rtable.h>
#include <zrp/zrp_aux.h>

// Temporary Query Cache Entry

class query_cache_entry {
	friend class query_cache;
	friend class ZRP;
public:
	query_cache_entry();
	~query_cache_entry();

protected:

	LIST_ENTRY(query_cache_entry) qc_link;

	ns_addr_t qc_src;			// Query Source
	u_int16_t query_id;			// Query ID
	zrp_nodelist prev_hop;			// Previous Hop
	u_int8_t hop_count;			// Hop count
	u_int injection_counter;		// Injection counter - don't know use yet
};

// Temporary Query Cache

class query_cache {
public:
	query_cache() { LIST_INIT(&qc_head); }
	query_cache_entry* head() { return qc_head.lh_first; }

private:
	LIST_HEAD(zrp_query_cache_head, query_cache_entry) qc_head;

};

#endif /* ZRP_IERPDST_H_ */
