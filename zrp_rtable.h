#ifndef __zrp_rtable_h__
#define __zrp_rtable_h__

#include <assert.h>
#include <sys/types.h>
#include <config.h>
#include <lib/bsd-list.h>
#include <scheduler.h>

#include <zrp/zrp_aux.h>

#define CURRENT_TIME		Scheduler::instance().clock()
#define INFINITY2			0xff

/*
   ZRP Neighbor Table Entry
*/
class ZRP_Neighbor {
        friend class ZRP;
        friend class zrp_rt_entry;
 public:
        ZRP_Neighbor(nsaddr_t a) { nb_addr = a; }

 protected:
        LIST_ENTRY(ZRP_Neighbor) nb_link;
        nsaddr_t        nb_addr;
        double          nb_expire;      // ALLOWED_HELLO_LOSS * HELLO_INTERVAL
};

LIST_HEAD(zrp_ntable, ZRP_Neighbor);

//		Routing Table Entry

class zrp_rt_entry {
	friend class zrp_rtable;
	friend class ZRP;
public:
	zrp_rt_entry();
	~zrp_rt_entry();

private:

	LIST_ENTRY(zrp_rt_entry) rt_link;

	nsaddr_t zrp_dst;
	nsaddr_t zrp_subnet;
	zrp_node_list routes;
	zrp_metric_list route_metrics;
	bool zrp_intrazone;

};

//		Routing Table

class zrp_rtable {
public:
	zrp_rtable() { LIST_INIT(&rthead); }

	zrp_rt_entry*	head() { return rthead.lh_first; }
	zrp_rt_entry* 	rt_add(nsaddr_t id);
	void 			rt_delete(nsaddr_t id);
	zrp_rt_entry*	rt_lookup(nsaddr_t id);
	bool		    rt_isIntra(nsaddr_t id);
private:
	LIST_HEAD(zrp_rthead, zrp_rt_entry) rthead;

};




#endif
