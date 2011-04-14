#ifndef __zrp_packet_h__
#define __zrp_packet_h__

//#include <config.h>
//#include "zrp.h"
#include <zrp/zrp_aux.h>
#define ZRP_MAX_ERRORS 100
#define ZRP_MAX_DESTINATIONS 5

/* =====================================================================
   Packet Formats...
   ===================================================================== */
#define ZRPTYPE_HELLO  	0x01

#define ZRPTYPE_QUERY 		0x02
#define ZRPTYPE_LINKSTATE	0x04
#define ZRPTYPE_BRP			0x08

/*
 * ZRP Routing Protocol Header Macros
 */
#define HDR_ZRP(p)		((struct hdr_zrp*)hdr_zrp::access(p))

#define HDR_ZRP_QUERY(p) 	((struct hdr_zrp_query*)hdr_zrp::access(p))
#define HDR_ZRP_LINKSTATE(p) 	((struct hdr_zrp_linkstate*)hdr_zrp::access(p))
#define HDR_ZRP_BRP(p) 	((struct hdr_zrp_brp*)hdr_zrp::access(p))

/*
 * General ZRP Header - shared by all formats
 */
struct hdr_zrp {
	u_int8_t        ah_type;
	/*
        u_int8_t        ah_reserved[2];
        u_int8_t        ah_hopcount;
	 */
	// Header access methods
	static int offset_; // required by PacketHeaderManager
	inline static int& offset() { return offset_; }
	inline static hdr_zrp* access(const Packet* p) {
		return (hdr_zrp*) p->access(offset_);
	}
};


struct hdr_zrp_query {
	u_int8_t		query_type;
	u_int8_t		ttl;
	u_int8_t		hop_count;
	bool			flags[8];
	u_int16_t       current_hop_ptr;
	u_int8_t		num_dest;
	u_int8_t		num_nodes;
	u_int16_t		query_id;
	u_int8_t		reserved[2];
	nsaddr_t		query_src_addr;
	nsaddr_t		query_dst[ZRP_MAX_DESTINATIONS];
	nsaddr_t		route[ZRP_MAX_ERRORS];
	// metric

	double			rq_timestamp;

	inline int size() {
			int sz = 0;
			sz = (6 + ZRP_MAX_DESTINATIONS + ZRP_MAX_ERRORS)*sizeof(u_int32_t);
			assert(sz);
			return sz;
		}
};

struct hdr_zrp_linkstate {
	nsaddr_t		dummy_reserved;
	nsaddr_t		link_src;
	nsaddr_t		link_dst;
	nsaddr_t		pkt_src;
	u_int16_t		link_state_id;
	u_int8_t 		zone_radius;		//Zone Radius
	bool 			flags[8];			//Full Link information
	nsaddr_t		link_dst_subnet;
	bool 			link_status;
	// metric

	double			lst_timestamp;

	inline int size() {
			int sz = 0;
			sz = (7)*sizeof(u_int32_t) + sizeof(bool);
			assert(sz);
			return sz;
		}
};

struct hdr_zrp_brp {
	nsaddr_t			des_addr;
	nsaddr_t			src_addr;
	u_int16_t			msg_id;
	u_int8_t			reserved[2];
	nsaddr_t			prev_border_addr;
	hdr_zrp_query		encapsulated_pkt_hdr;

	double				brp_timestamp;



	inline int size() {
				int sz = 0;
				sz = (4)*sizeof(u_int32_t) + sizeof(hdr_zrp_query);
				assert(sz);
				return sz;
			}
};

// for size calculation of header-space reservation
union hdr_all_zrp {
	hdr_zrp		          ah;
	hdr_zrp_linkstate 	  rlst;
	hdr_zrp_query		  rquery;
	hdr_zrp_brp			  rbrp;
};

#endif /* __zrp_packet_h__ */
