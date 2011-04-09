/* zrp_aux.h */
#ifndef __zrp_aux_h__
#define __zrp_aux_h__
#include <lib/bsd-list.h>
#include <lib/bsd-list.h>

class nodelist_entry {
	friend class ZRP;
	friend class zrp_rt_entry;
protected:
	ns_addr_t node;
	LIST_ENTRY(nodelist_entry) nl_link;
};

LIST_HEAD(zrp_nodelist, nodelist_entry);

class nodelist {
public:
	nodelist() { LIST_INIT(&nl_head); }

private:
	LIST_HEAD(nodelist_head, nodelist_entry) nl_head;
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
