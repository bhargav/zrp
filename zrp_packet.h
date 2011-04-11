/* zrp_packet.h */
#ifndef __zrp_packet_h__
#define __zrp_packet_h__

#include <zrp/zrp_aux.h>

// Packet Formats 

#define ZRPTYPE_HELLO 	0x01
#define ZRPTYPE_RREQ   	0x02
#define ZRPTYPE_RREP   	0x04
#define ZRPTYPE_REXT   	0x08


//Header Macros

#define HDR_ZRP(p) 					((struct hdr_zrp*)hdr_zrp::access(p))
#define HDR_ZRP_INTRA_LINKST(p)  	((struct hdr_zrp_intra_linkst*)hdr_zrp::access(p))
#define HDR_ZRP_INTER(p)  			((struct hdr_zrp_inter*)hdr_zrp::access(p))
#define HDR_ZRP_BRP(p)				((struct hdr_zrp_brp*)hdr_zrp::access(p))


struct hdr_zrp
{ 
	u_int8_t        ah_type;

	// Header access methods
	static int offset_; // required by PacketHeaderManager
	inline static int& offset() { return offset_; }
	inline static hdr_zrp* access(const Packet* p) 
	{
		return (hdr_zrp*) p->access(offset_);
	}
};

struct hdr_zrp_intra_linkst
{
	u_int8_t 		intra_type;			//Intra Type
	nsaddr_t 		lk_src_addr;		//Link Source Address
	nsaddr_t 		lk_dest_addr;		//Link Destination Address
	nsaddr_t		pkt_src_addr;		//Address of the node that sent the packet
	u_int16_t		state_id;			//Sequence Number
	u_int8_t 		zone_radius;		//Zone Radius
	bool 			flags[8];			//Full Link information
	// Metric??
};


struct hdr_zrp_inter
{
	u_int8_t		inter_type;
	char			type[8];
	u_int8_t		ttl;
	u_int8_t		h_count;
	bool			flags[8];
	u_int16_t       curr_hop_ptr;
	u_int8_t		num_dest;
	u_int8_t		num_nodes;
	u_int8_t		q_id;
	nsaddr_t		lk_src_addr;
//	zrp_node_list	query_dst;
//	zrp_node_list	route;
//	zrp_nodelist	query_dst;
	nsaddr_t		query_dst;
	zrp_nodelist	route;
	zrp_metric_list metric;
};

struct hdr_zrp_brp
{
	u_int8_t			brp_type;
	nsaddr_t			src_addr;
	u_int16_t			msg_id;
	nsaddr_t			prev;
	//	union hdr_all_zrp	packet;		//If any problem , make it struct
};

// for size calculation of header-space reservation
//union hdr_all_zrp
//{
//	hdr_zrp 					ah;
//	hdr_zrp_intra_linkst 		intra;
//	hdr_zrp_inter				inter;
//	hdr_zrp_brp					brp;
//};

#endif /* __zrp_packet_h__ */
