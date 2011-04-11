#include <zrp/zrp.h>
#include <zrp/zrp_packet.h>
#include <random.h>
#include <cmu-trace.h>

#define max(a,b)			( (a) > (b) ? (a) : (b) )
#define CURRENT_TIME		Scheduler::instance().clock()

/*
 * TCL Hooks
 */

int hdr_zrp::offset_;
static class ZRPHeaderClass : public PacketHeaderClass {
public:
	ZRPHeaderClass() : PacketHeaderClass("PacketHeader/ZRP",sizeof(hdr_all_zrp)) {
		bind_offset(&hdr_zrp::offset_);
	}
} class_rtProtoZRP_hdr;

static class ZRPclass : public TclClass {
public:
	ZRPclass() : TclClass("Agent/ZRP") {}

	TclObject* create(int argc, const char*const* argv) {
		assert(argc == 5);
		return (new ZRP((nsaddr_t) Address::instance().str2addr(argv[4])));
	}
} class_rtProtoZRP;

int
ZRP::command(int argc, const char*const* argv) {
	if (argc == 2) {
		Tcl& tcl = Tcl::instance();

		if (strncasecmp(argv[1], "id", 2) == 0) {
			tcl.resultf("%d", index);
			return TCL_OK;
		}
	}
	return Agent::command(argc, argv);
}

/*
 * Constructor for ZRP class
 */
ZRP::ZRP(nsaddr_t id) : Agent(PT_ZRP) {					// Need to assign a number from packet.h
	//	bind_bool("accessible_var_", &accessible_var_);
	index = id;
	seqno = 2;										// From AODV need rationale
}

/*
 * Packet Reception Routines
 */

void
ZRP::recv(Packet *p,Handler*) {
	struct hdr_cmn *commonHeader = HDR_CMN(p);
	struct hdr_ip *ipHeader = HDR_IP(p);

	assert(initialized());

	if (commonHeader->ptype() == PT_ZRP) {
		ipHeader->ttl_ -= 1;
		recvZRP(p);
		return;
	}
}

void ZRP::recvZRP(Packet *p) {

	struct hdr_zrp *ah = HDR_ZRP(p);

	assert(HDR_IP (p)->sport() == RT_PORT);
	assert(HDR_IP (p)->dport() == RT_PORT);

	/*
	 * Incoming Packets.
	 */
	switch(ah->ah_type) {

	case ZRPTYPE_RREQ:
		recvQuery(p);
		break;

	case ZRPTYPE_RREP:
		recvReply(p);
		break;

	case ZRPTYPE_REXT:
		recvExtension(p);
		break;

		//		case ZRPTYPE_HELLO:			//Check if we need Hello packets for NDM
		//		recvHello(p);
		//		break;

	default:
		fprintf(stderr, "Invalid ZRP type (%x)\n", ah->ah_type);
		exit(1);
	}
}

void ZRP::recvReply(Packet *p) {
	struct hdr_cmn *ch = HDR_CMN(p);

#ifdef DEBUG
	fprintf(stderr, "%d - %s: received a REPLY\n", index, __FUNCTION__);
#endif

}

void ZRP::recvExtension(Packet *p) {
	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ip = HDR_IP(p);

#ifdef DEBUG
	fprintf(stderr, "%d - %s: received a QUERY EXTENSION\n", index, __FUNCTION__);
#endif
}

void ZRP::recvQuery(Packet *p)
{
	struct hdr_ip *ih =  HDR_IP(p);
	struct hdr_zrp_inter *rq = HDR_ZRP_INTER(p);
	zrp_rt_entry *rt;

	// Drop, if i am source.
	if(rq->rq_src == index) {
#ifdef DEBUG
		fprintf(stderr, "%s: got my own REQUEST\n", __FUNCTION__);
#endif // DEBUG
		Packet::free(p);
		return;
	}

	/*The packet's accumulated route information is recorded in X's Routing Table and Temporary Query Cache.
	  The accumulated route is replaced by X's address and any accumulated route metrics are updated and compressed*/

	/*If Destination lies in Routing zone of X (check it using rt_isIntra(ns_addr_t id)) ,
	  then we need to generate ROUTE_REPLY and ROUTE_EXTENSION. */

	/*If destination is not in X's routing zone, then Route_Query is forwarded to BRP. */



}

