/*
 * zrp_iarpdst.h
 */

#ifndef ZRP_IARPDST_H_
#define ZRP_IARPDST_H_

#include <zrp/zrp_rtable.h>

zrp_nodelist pending_lst_list;
zrp_nodelist new_neighbour_list;
zrp_nodelist former_routing_zones;


class zrp_lst_entry
{
	friend class zrp_lstable;
	friend class ZRP;
public:
	zrp_lst_entry();
	~zrp_lst_entry();

protected:

	nsaddr_t link_src;
	int  zone_radius;
	int lst_id;
	zrp_ls_info ls_info;

	LIST_ENTRY(zrp_lst_entry) lst_link;

};

class zrp_lstable
{
public:
	zrp_lstable() { LIST_INIT(&lst_head); }
	zrp_lst_entry* head() { return lst_head.lh_first; }

private:
	LIST_HEAD(zrp_lstable_head, zrp_lst_entry) lst_head;

};

#endif /* ZRP_IARPDST_H_ */
