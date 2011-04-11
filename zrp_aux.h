/* zrp_aux.h */
#ifndef __zrp_aux_h__
#define __zrp_aux_h__
#include <lib/bsd-list.h>

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

class ls_info_entry
{
	friend class ZRP;
	friend class zrp_lst_entry;
protected:
	nsaddr_t link_dst;
	nsaddr_t link_subnet;
	zrp_metric_list link_metrics;
	bool forwarded;
	LIST_ENTRY(ls_info_entry) ls_info_link;
};

LIST_HEAD(zrp_ls_info, ls_info_entry);

inline zrp_nodelist_entry *zrp_nodelist::nl_insertNode(nsaddr_t id)
{
	zrp_nodelist_entry *nle = new zrp_nodelist_entry;
	assert(nle);
	nle->node = id;
	LIST_INSERT_AFTER(nl_head.lh_first , nle, nl_link);
	return nle;
}



inline zrp_nodelist_entry *zrp_nodelist::nl_lookup(nsaddr_t id)
{
	zrp_nodelist_entry *nle = nl_head.lh_first;
	for (; nle ; nle = nle->nl_link.le_next) {
		if (nle->node = id)
			break;
	}
	return nle;
}

inline void zrp_nodelist::nl_delete(nsaddr_t id)
{
	zrp_nodelist_entry *nle = nl_lookup(id);
	if (nle) {
		LIST_REMOVE(nle, nl_link);
		delete nle;
	}
}





#endif
