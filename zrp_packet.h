/* zrp_packet.h */
#ifndef __zrp_packet_h__
#define __zrp_packet_h__


// Packet Formats 

#define ZRPTYPE_HELLO 0x01

//Header Macros

#define HDR_ZRP(p) 	((struct hdr_zrp*)hdr_zrp::access(p))


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
	ns_addr_t 		lk_src_addr;		//Link Source Address
	ns_addr_t 		lk_dest_addr;		//Link Destination Address
	ns_addr_t		pkt_src_addr;		//Address of the node that sent the packet
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
	u_int8_t		nodes;
	u_int8_t		q_id;
	ns_addr_t		lk_src_addr;
	//Query Destination address ??
	//Route??
	//Metric??
};

struct hdr_zrp_brp
{
	u_int8_t			brp_type;
	ns_addr_t			src_addr;
	u_int16_t			msg_id;
	ns_addr_t			prev;
	union hdr_all_zrp	packet;		//If any problem , make it struct
};

// for size calculation of header-space reservation
union hdr_all_zrp
{
	hdr_zrp 			ah;
	hdr_zrp_intra 		intra;
	hdr_zrp_inter		inter;
	hdr_zrp_brp			brp;
};
#endif /* __zrp_packet_h__ */
