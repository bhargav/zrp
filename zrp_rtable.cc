#include <zrp/zrp_rtable.h>

// Routing Table entry

zrp_rt_entry::zrp_rt_entry()
{
//	zrp_dst = 0;
//	zrp_subnet = 0;

	LIST_INIT(&routes);
	LIST_INIT(&route_metrics);
	zrp_intrazone = FALSE;
}

zrp_rt_entry::~zrp_rt_entry()
{
	zrp_nodelist_entry *nlen;

	while((nlen = routes.lh_first)) {
		LIST_REMOVE(nlen, nl_link);
		delete nlen;
	}

	metric_entry *men;

	while((men = route_metrics.lh_first)) {
		LIST_REMOVE(men, metric_link);
		delete men;
	}
}

// Routing Table

zrp_rt_entry *zrp_rtable::rt_add(nsaddr_t id)
{
	zrp_rt_entry *rt;

	//assert(rt_lookup(id) == 0)
	rt = new zrp_rt_entry;
	assert(rt);
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

zrp_rt_entry *zrp_rtable::rt_lookup(nsaddr_t id)
{
	zrp_rt_entry *rt = rthead.lh_first;

	for (; rt ; rt = rt->rt_link.le_next) {
		if (rt->zrp_dst = id)
			break;
	}
	return rt;
}

bool zrp_rtable::rt_isIntra(nsaddr_t id)
{
	zrp_rt_entry *rt = rthead.lh_first;

	for (; rt ; rt = rt->rt_link.le_next) {
		if (rt->zrp_dst = id)
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
