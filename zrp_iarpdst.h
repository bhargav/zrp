/*
 * zrp_iarpdst.h
 */

#ifndef ZRP_IARPDST_H_
#define ZRP_IARPDST_H_

#include <zrp/zrp_rtable.h>

#define ZRP_NEW_LINK_INFO 1
#define ZRP_OLD_LINK_UPDATE 0

class ls_info_entry
{
	friend class ZRP;
	friend class zrp_lst_entry;
	friend class zrp_lstable;
	friend class lsinfo_entry;
protected:
	nsaddr_t link_dst;
	nsaddr_t link_subnet;
	zrp_metric_list link_metrics;
	bool link_status;
	bool forwarded;
	LIST_ENTRY(ls_info_entry) ls_info_link;
};

LIST_HEAD(zrp_ls_info, ls_info_entry);

class lsinfo_entry {
	friend class ls_info_entry;
	friend class zrp_lst_entry;
	friend class zrp_lstable;
	friend class ZRP;
public:
	lsinfo_entry() { LIST_INIT(&ls_info); }
	LIST_ENTRY(lsinfo_entry) lsinfo_link;

	int lst_id;
	zrp_ls_info ls_info;

};

LIST_HEAD(zrp_lsinfo, lsinfo_entry);

class zrp_lst_entry
{
	friend class zrp_lstable;
	friend class ZRP;
	friend class lsinfo_entry;
	friend class ls_info_entry;
public:
	zrp_lst_entry();
	lsinfo_entry* lshead() { return lslisthead.lh_first; }
protected:

	nsaddr_t link_src;
	uint8_t  zone_radius;
	zrp_lsinfo lslisthead;

	LIST_ENTRY(zrp_lst_entry) lst_link;

};

class zrp_lstable
{
	friend class ZRP;
public:
	zrp_lstable() { LIST_INIT(&lst_head); }
	zrp_lst_entry* 	head() { return lst_head.lh_first; }
	bool 			lst_insert(nsaddr_t, nsaddr_t, nsaddr_t, u_int8_t, u_int16_t, bool*);
	zrp_lst_entry* 	lst_lookup(nsaddr_t);
	void			lst_delete(nsaddr_t);
	void			lst_dump();
private:
	LIST_HEAD(zrp_lstable_head, zrp_lst_entry) lst_head;
};

#endif /* ZRP_IARPDST_H_ */
