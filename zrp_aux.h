/* zrp_aux.h */
#ifndef __zrp_aux_h__
#define __zrp_aux_h__

class nodelist_entry {
protected:
	ns_addr_t node;
	LIST_ENTRY(nodelist_entry) nl_link;
};

class nodelist {
public:
	nodelist() { LIST_INIT(&nl_head); }

private:
	LIST_HEAD(nodelist_head, nodelist_entry) nl_head;
};

struct	metric			// variable used by a dynamic router to calculate its routing table
{
	char 			type;
	u_int16_t		value;
};

#endif