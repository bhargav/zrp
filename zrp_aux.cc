#include <zrp/zrp_aux.h>


zrp_nodelist_entry *zrp_nodelist::nl_insertNode(nsaddr_t id)
{
	zrp_nodelist_entry *nle = new zrp_nodelist_entry;
	assert(nle);
	nle->node = id;
	LIST_INSERT_HEAD(&nl_head, nle, nl_link);
	return nle;
}



zrp_nodelist_entry *zrp_nodelist::nl_lookup(nsaddr_t id)
{
	zrp_nodelist_entry *nle = nl_head.lh_first;
	for (; nle ; nle = nle->nl_link.le_next) {
		if (nle->node = id)
			break;
	}
	return nle;
}

bool zrp_nodelist::nl_isempty()
{
	zrp_nodelist_entry *nle = nl_head.lh_first;
	if (nle)
		return TRUE;
	return FALSE;
}

void zrp_nodelist::nl_purge()
{
	zrp_nodelist_entry *nle = nl_head.lh_first;
	zrp_nodelist_entry *nbn;

	for(; nle; nle = nbn) {
		nbn = nle->nl_link.le_next;
		nl_delete(nle->node);
	}
}

void zrp_nodelist::nl_delete(nsaddr_t id)
{
	zrp_nodelist_entry *nle = nl_lookup(id);
	if (nle) {
		LIST_REMOVE(nle, nl_link);
		delete nle;
	}
}
