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

// for size calculation of header-space reservation

union hdr_all_zrp
{
	hdr_zrp 	ah;
};
#endif /* __zrp_packet_h__ */
