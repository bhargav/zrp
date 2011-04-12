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

#define ZRPTYPE_RREQ   	0x02
#define ZRPTYPE_RREP   	0x04
#define ZRPTYPE_RERR   	0x08
#define ZRPTYPE_RREP_ACK  	0x10

#define ZRPTYPE_QUERY 		0x22
#define ZRPTYPE_LINKSTATE	0x24
#define ZRPTYPE_BRP			0x28

/*
 * ZRP Routing Protocol Header Macros
 */
#define HDR_ZRP(p)		((struct hdr_zrp*)hdr_zrp::access(p))

#define HDR_ZRP_QUERY(p) 	((struct hdr_zrp_query*)hdr_zrp::access(p))
#define HDR_ZRP_LINKSTATE(p) 	((struct hdr_zrp_linkstate*)hdr_zrp::access(p))
#define HDR_ZRP_BRP(p) 	((struct hdr_zrp_brp*)hdr_zrp::access(p))

#define HDR_ZRP_REQUEST(p)  	((struct hdr_zrp_request*)hdr_zrp::access(p))
#define HDR_ZRP_REPLY(p)	((struct hdr_zrp_reply*)hdr_zrp::access(p))
#define HDR_ZRP_ERROR(p)	((struct hdr_zrp_error*)hdr_zrp::access(p))
#define HDR_ZRP_RREP_ACK(p)	((struct hdr_zrp_rrep_ack*)hdr_zrp::access(p))

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

struct hdr_zrp_request {
	u_int8_t        rq_type;	// Packet Type
	u_int8_t        reserved[2];
	u_int8_t        rq_hop_count;   // Hop Count
	u_int32_t       rq_bcast_id;    // Broadcast ID

	nsaddr_t        rq_dst;         // Destination IP Address
	u_int32_t       rq_dst_seqno;   // Destination Sequence Number
	nsaddr_t        rq_src;         // Source IP Address
	u_int32_t       rq_src_seqno;   // Source Sequence Number

	double          rq_timestamp;   // when REQUEST sent;
	// used to compute route discovery latency

	// This define turns on gratuitous replies- see zrp.cc for implementation contributed by
	// Anant Utgikar, 09/16/02.
	//#define RREQ_GRAT_RREP	0x80

	inline int size() {
		int sz = 0;
		/*
  	sz = sizeof(u_int8_t)		// rq_type
	     + 2*sizeof(u_int8_t) 	// reserved
	     + sizeof(u_int8_t)		// rq_hop_count
	     + sizeof(double)		// rq_timestamp
	     + sizeof(u_int32_t)	// rq_bcast_id
	     + sizeof(nsaddr_t)		// rq_dst
	     + sizeof(u_int32_t)	// rq_dst_seqno
	     + sizeof(nsaddr_t)		// rq_src
	     + sizeof(u_int32_t);	// rq_src_seqno
		 */
		sz = 7*sizeof(u_int32_t);
		assert (sz >= 0);
		return sz;
	}
};

struct hdr_zrp_reply {
	u_int8_t        rp_type;        // Packet Type
	u_int8_t        reserved[2];
	u_int8_t        rp_hop_count;           // Hop Count
	nsaddr_t        rp_dst;                 // Destination IP Address
	u_int32_t       rp_dst_seqno;           // Destination Sequence Number
	nsaddr_t        rp_src;                 // Source IP Address
	double	        rp_lifetime;            // Lifetime

	double          rp_timestamp;           // when corresponding REQ sent;
	// used to compute route discovery latency

	inline int size() {
		int sz = 0;
		/*
  	sz = sizeof(u_int8_t)		// rp_type
	     + 2*sizeof(u_int8_t) 	// rp_flags + reserved
	     + sizeof(u_int8_t)		// rp_hop_count
	     + sizeof(double)		// rp_timestamp
	     + sizeof(nsaddr_t)		// rp_dst
	     + sizeof(u_int32_t)	// rp_dst_seqno
	     + sizeof(nsaddr_t)		// rp_src
	     + sizeof(u_int32_t);	// rp_lifetime
		 */
		sz = 6*sizeof(u_int32_t);
		assert (sz >= 0);
		return sz;
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
	nsaddr_t		link_src;
	nsaddr_t		link_dst;
	nsaddr_t		pkt_src;
	u_int16_t		link_state_id;
	u_int8_t 		zone_radius;		//Zone Radius
	bool 			flags[8];			//Full Link information
	nsaddr_t		link_dst_subnet;
	// metric

	double			lst_timestamp;

	inline int size() {
			int sz = 0;
			sz = (6)*sizeof(u_int32_t);
			assert(sz);
			return sz;
		}
};

struct hdr_zrp_brp {
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

struct hdr_zrp_error {
	u_int8_t        re_type;                // Type
	u_int8_t        reserved[2];            // Reserved
	u_int8_t        DestCount;                 // DestCount
	// List of Unreachable destination IP addresses and sequence numbers
	nsaddr_t        unreachable_dst[ZRP_MAX_ERRORS];
	u_int32_t       unreachable_dst_seqno[ZRP_MAX_ERRORS];

	inline int size() {
		int sz = 0;
		/*
  	sz = sizeof(u_int8_t)		// type
	     + 2*sizeof(u_int8_t) 	// reserved
	     + sizeof(u_int8_t)		// length
	     + length*sizeof(nsaddr_t); // unreachable destinations
		 */
		sz = (DestCount*2 + 1)*sizeof(u_int32_t);
		assert(sz);
		return sz;
	}

};

struct hdr_zrp_rrep_ack {
	u_int8_t	rpack_type;
	u_int8_t	reserved;
};

// for size calculation of header-space reservation
union hdr_all_zrp {
	hdr_zrp		          ah;
	hdr_zrp_linkstate 	  rlst;
	hdr_zrp_query		  rquery;
	hdr_zrp_brp			  rbrp;

	hdr_zrp_request  	  rreq;
	hdr_zrp_reply    	  rrep;
	hdr_zrp_error    	  rerr;
	hdr_zrp_rrep_ack 	  rrep_ack;
};

#endif /* __zrp_packet_h__ */
