#include <zrp/zrp_iarpdst.h>

zrp_lst_entry::zrp_lst_entry() {
	link_src = 0;
	zone_radius = 0;
}

bool
zrp_lstable::lst_insert(nsaddr_t link_src, nsaddr_t link_dst, nsaddr_t subnet_mask, u_int8_t radius, u_int16_t state_id, bool* flags) {


	bool new_link = FALSE;
	bool isPresent = TRUE;
	zrp_lst_entry *le = lst_lookup(link_src);

	if (le == 0)
	{
		le = new zrp_lst_entry;
		le->link_src = link_src;
		le->zone_radius = radius;
		new_link = TRUE;
		isPresent = FALSE;
	}

	assert(le);

	std::vector<lsinfo_entry>::iterator it;
	bool inserted = FALSE;
	for (it=le->lsinfo.begin(); it!=le->lsinfo.end(); ++it) {
		if ((*it).lst_id = state_id) {
			inserted = TRUE;
			break;
		}
	}

	ls_subentry *sb = new ls_subentry;
	sb->forwarded = FALSE;
	sb->link_dst = link_dst;
	sb->link_status = *(flags + 4);
	sb->link_subnet = 0;
	if (inserted) {

		(*it).ls_info.push_back(*sb);
	}
	else {
		lsinfo_entry *lse = new lsinfo_entry;
		lse->lst_id = link_src;
		lse->ls_info.push_back(*sb);
		le->lsinfo.push_back(*lse);
	}

	if(isPresent == FALSE)
		lshead.push_back(le);

	return new_link;
}

zrp_lst_entry*
zrp_lstable::lst_lookup(nsaddr_t id) {
	std::vector<zrp_lst_entry*>::iterator itr;
	for(itr = lshead.begin(); itr != lshead.end(); ++itr) {
		if ((*itr)->link_src == id) {
			return (*itr);
		}
	}
	return NULL;
}

/*
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
*/
void
zrp_lstable::lst_dump() {
	printf("Dumping LST -- ");
	std::vector<zrp_lst_entry*>::iterator mit;
	for (mit = lshead.begin(); mit != lshead.end(); ++mit) {
		printf(" { s : %d - r = %d -", (*mit)->link_src, (*mit)->zone_radius);
		std::vector<lsinfo_entry>::iterator it;
		std::vector<ls_subentry>::iterator suit;
		for(it = (*mit)->lsinfo.begin(); it != (*mit)->lsinfo.end(); ++it) {
			printf("[ sid : %d -", (*it).lst_id);
			for(suit = (*it).ls_info.begin(); suit != (*it).ls_info.end(); ++suit) {
				printf("-( d : %d - msk : %d - frwd : %d st : %s)-", (*suit).link_dst, (*suit).link_subnet, (*suit).forwarded, ((*suit).link_status ? "UP ": "DOWN" ));
			}
		}
		printf(" } ");
	}
	printf(" -- done \n");
}

