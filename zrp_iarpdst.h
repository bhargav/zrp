/*
 * zrp_iarpdst.h
 */

#ifndef ZRP_IARPDST_H_
#define ZRP_IARPDST_H_

#include <zrp/zrp_rtable.h>
#include <vector>

#define ZRP_NEW_LINK_INFO 1
#define ZRP_OLD_LINK_UPDATE 0

class ls_subentry
{
public:
	nsaddr_t link_dst;
	nsaddr_t link_subnet;
	bool link_status;
	bool forwarded;
	ls_subentry() { link_dst = 0; link_status = 0; link_status = 0; forwarded = 0; }
};

struct lsinfo_entry {
	int lst_id;
	std::vector<ls_subentry> ls_info;

};

class zrp_lst_entry
{
	friend class zrp_lstable;
	friend class ZRP;
	friend class lsinfo_entry;
	friend class ls_info_entry;
public:
	zrp_lst_entry();
protected:
	nsaddr_t link_src;
	uint8_t  zone_radius;
	std::vector<lsinfo_entry> lsinfo;

	LIST_ENTRY(zrp_lst_entry) lst_link;
};

class zrp_lstable
{
	friend class ZRP;
public:
	bool 			lst_insert(nsaddr_t, nsaddr_t, nsaddr_t, u_int8_t, u_int16_t, bool*);
	zrp_lst_entry* 	lst_lookup(nsaddr_t);
	void			lst_delete(nsaddr_t);
	void			lst_dump();
	std::vector<zrp_lst_entry*> lshead;
};

#endif /* ZRP_IARPDST_H_ */
