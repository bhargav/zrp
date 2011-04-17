#include <zrp/zrp_rtable.h>

// Routing Table entry

zrp_rt_entry::zrp_rt_entry()
{
	zrp_dst = 0;
	zrp_subnet = 0;

	zrp_intrazone = FALSE;
}

// Routing Table

zrp_rt_entry *zrp_rtable::rt_add(nsaddr_t id)
{
	zrp_rt_entry *rt = rt_lookup(id);
	if (rt != 0) {
		LIST_REMOVE(rt, rt_link);
		delete rt;
	}

	rt = new zrp_rt_entry;
	rt->zrp_dst = id;
	LIST_INSERT_HEAD(&rthead, rt, rt_link);
	return rt;
}

void
zrp_rtable::rt_delete(nsaddr_t id)
{
	zrp_rt_entry *rt = rt_lookup(id);

	if (rt) {
		LIST_REMOVE(rt, rt_link);
		delete rt;
	}

}

zrp_rt_entry* zrp_rtable::rt_lookup(nsaddr_t id)
{
	zrp_rt_entry *rt = rthead.lh_first;

	for (; rt ; rt = rt->rt_link.le_next) {
		if (rt->zrp_dst == id)
			break;
	}
	return rt;
}

bool zrp_rtable::rt_isIntra(nsaddr_t id)
{
	zrp_rt_entry *rt = rthead.lh_first;

	for (; rt ; rt = rt->rt_link.le_next) {
		if (rt->zrp_dst == id)
			return rt->zrp_intrazone;
	}
	return FALSE;
}

bool zrp_rtable::rt_isPeripheral(nsaddr_t id)
{
	zrp_rt_entry *rt = rthead.lh_first;

	for (; rt ; rt = rt->rt_link.le_next) {
		if (rt->zrp_dst = id)
			return rt->zrp_peripheral;
	}
	return FALSE;
}
