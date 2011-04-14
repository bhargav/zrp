/* zrp_aux.h */
#ifndef __zrp_aux_h__
#define __zrp_aux_h__

#include <assert.h>
#include <sys/types.h>
#include <config.h>
#include <lib/bsd-list.h>
#include <scheduler.h>

class zrp_nodelist_entry {
	friend class ZRP;
	friend class zrp_nodelist;
	friend class zrp_rt_entry;
protected:
	nsaddr_t node;
	LIST_ENTRY(zrp_nodelist_entry) nl_link;
};

LIST_HEAD(zrp_node_list, zrp_nodelist_entry);

class zrp_nodelist {
public:
	zrp_nodelist() { LIST_INIT(&nl_head); }
	zrp_nodelist_entry* head() { return nl_head.lh_first; }
	zrp_nodelist_entry* nl_insertNode (nsaddr_t);
	zrp_nodelist_entry* nl_lookup(nsaddr_t);
	bool nl_isempty();
	void nl_purge();
	void nl_delete(nsaddr_t);


private:
	LIST_HEAD(zrp_nl, zrp_nodelist_entry) nl_head;
};

class metric_entry			// variable used by a dynamic router to calculate its routing table
{
	friend class ZRP;
	friend class zrp_rt_entry;
protected:
	char 			RESERVED[8];
	char 			type[8];
	u_int16_t		value;
	LIST_ENTRY(metric_entry) metric_link;
};

LIST_HEAD(zrp_metric_list, metric_entry);

#endif
