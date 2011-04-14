#include <zrp/zrp_iarpdst.h>

zrp_lst_entry::zrp_lst_entry() {
	link_src = 0;
	zone_radius = 0;
	LIST_INIT(&lslisthead);
}

bool
zrp_lstable::lst_insert(nsaddr_t link_src, nsaddr_t link_dst, nsaddr_t subnet_mask, u_int8_t radius, u_int16_t state_id, bool* flags) {


	bool new_link = FALSE;
	zrp_lst_entry *le = lst_lookup(link_src);

	if (le == 0)
	{
		le = new zrp_lst_entry;
		le->link_src = link_src;
		le->zone_radius = radius;
		new_link = TRUE;
	}

	assert(le);

	ls_info_entry *lsie = new ls_info_entry;
	lsie->link_dst = link_dst;
	lsie->forwarded = FALSE;
	lsie->link_subnet = subnet_mask;
	lsie->link_status = *(flags + 4);

	lsinfo_entry *lsi = new lsinfo_entry;
	lsi->lst_id = state_id;

	LIST_INSERT_HEAD( &(lsi->ls_info), lsie, ls_info_link);

	LIST_INSERT_HEAD(&(le->lslisthead), lsi, lsinfo_link);

	LIST_INSERT_HEAD(&lst_head, le, lst_link);

//	printf("Here %d and %d", lst_head.lh_first->link_src, lst_head.lh_first->lslisthead.lh_first->ls_info.lh_first->link_dst);
	return new_link;
}

zrp_lst_entry*
zrp_lstable::lst_lookup(nsaddr_t id) {
	zrp_lst_entry *le = lst_head.lh_first;

	for (; le; le = le->lst_link.le_next) {
		if (le->link_src == id)
			break;
	}
	return le;
}

void
zrp_lstable::lst_delete(nsaddr_t id) {
	zrp_lst_entry *le = lst_head.lh_first;

	for (; le; le = le->lst_link.le_next) {
		if (le->link_src == id) {
			LIST_REMOVE(le, lst_link);
			delete le;
			break;
		}
	}

}

void
zrp_lstable::lst_dump() {
//	printf("Dumping LST -- ");
//	zrp_lst_entry *le = lst_head.lh_first;
//	if (le != 0) {
//		printf(" { s : %d - r = %d -", le->link_src, le->zone_radius);
//		lsinfo_entry *lse = le->lslisthead.lh_first;
//		for (; lse; lse=lse->lsinfo_link.le_next) {
//			printf("[ sid : %d -", lse->lst_id);
//			ls_info_entry *lsie = lse->ls_info.lh_first;
//			for (; lsie; lsie = lsie->ls_info_link.le_next) {
//				printf("-( d : %d - msk : %d - frwd : %d )-", lsie->link_dst, lsie->link_subnet, lsie->forwarded);
//			}
//			printf("- ]");
//		}
//		printf(" } ");
//	}
//	printf(" -- done \n");
}
