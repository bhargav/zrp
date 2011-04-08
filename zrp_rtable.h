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

//		Routing Table

class zrp_rt_entry {
	friend class zrp_rtable;
	friend class ZRP;

public:
	zrp_rt_entry();
	~zrp_rt_entry();

private:

	ns_addr_t zrp_dst;
	ns_addr_t zrp_subnet;

	bool zrp_intrazone;

};

//		Bordercast Tree Table

class zrp_rtable {
public:
	zrp_rtable() { LIST_INIT(&rthead); }

	zrp_rt_entry*	head() { return rthead.lh_first; }
private:
	LIST_HEAD(zrp_rthead, zrp_rt_entry) rthead;
};




#endif
