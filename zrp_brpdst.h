/*
 * zrp_brpdst.h
 */

#ifndef ZRP_BRPDST_H_
#define ZRP_BRPDST_H_

#include <assert.h>
#include <sys/types.h>
#include <config.h>
#include <lib/bsd-list.h>
#include <scheduler.h>

#include <zrp/zrp_aux.h>

//		Bordercast Tree table Entry

class zrp_btt_entry {
	friend class zrp_bttable;
	friend class ZRP;

public:
	zrp_btt_entry();
	~zrp_btt_entry();

private:

	ns_addr_t node_id;
	bool member;

	LIST_ENTRY(nodelist_entry) dwnstrm_n;				 // Downstream Neighbor List
	LIST_ENTRY(nodelist_entry) dwnstrm_pnodes;			//  Downstream Peripheral Nodes List
};

//		Bordercast Tree Table

class zrp_bttable {
public:
	zrp_bttable() { LIST_INIT(&btthead); }

	zrp_btt_entry*	head() { return btthead.lh_first; }
private:
	LIST_HEAD(zrp_btthead, zrp_btt_entry) btthead;
};


#endif /* ZRP_BRPDST_H_ */
