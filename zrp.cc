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

	struct hdr_aodv *ah = HDR_ZRP(p);

	 assert(HDR_IP (p)->sport() == RT_PORT);
	 assert(HDR_IP (p)->dport() == RT_PORT);

	 /*
	  * Incoming Packets.
	  */
	 switch(ah->ah_type) {

	 case ZRPTYPE_RREQ:
	   recvRequest(p);
	   break;

	 case ZRPTYPE_RREP:
	   recvReply(p);
	   break;

	 case ZRPTYPE_REXT:
	   recvExtension(p);
	   break;

	 //case AODVTYPE_HELLO:
	   //recvHello(p);
	   //break;

	 default:
	   fprintf(stderr, "Invalid ZRP type (%x)\n", ah->ah_type);
	   exit(1);
	 }
}

