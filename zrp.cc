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
	ZRPHeaderClass() : PacketHeaderClass("PacketHeader/ZRP",sizeof(hdr_zrp_inter)) {
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
	//	if (argc == 2) {
	//		Tcl& tcl = Tcl::instance();
	//
	//		if (strncasecmp(argv[1], "id", 2) == 0) {
	//			tcl.resultf("%d", index);
	//			return TCL_OK;
	//		}
	//	}
	if(argc == 2) {
		Tcl& tcl = Tcl::instance();

		if(strncasecmp(argv[1], "id", 2) == 0) {
			tcl.resultf("%d", index);
			return TCL_OK;
		}

		if(strncasecmp(argv[1], "start", 2) == 0) {
			//	      btimer.handle((Event*) 0);

			//	#ifndef AODV_LINK_LAYER_DETECTION
			htimer.handle((Event*) 0);
			//	      ntimer.handle((Event*) 0);
			//	#endif // LINK LAYER DETECTION

			//	      rtimer.handle((Event*) 0);
			return TCL_OK;
		}
	}
	else if(argc == 3) {
		if(strcmp(argv[1], "index") == 0) {
			index = atoi(argv[2]);
			return TCL_OK;
		}

		else if(strcmp(argv[1], "log-target") == 0 || strcmp(argv[1], "tracetarget") == 0) {
			logtarget = (Trace*) TclObject::lookup(argv[2]);
			if(logtarget == 0)
				return TCL_ERROR;
			return TCL_OK;
		}
		//	    else if(strcmp(argv[1], "drop-target") == 0) {
		//	    int stat = rqueue.command(argc,argv);
		//	      if (stat != TCL_OK) return stat;
		//	      return Agent::command(argc, argv);
		//	    }
//		else if(strcmp(argv[1], "if-queue") == 0) {
			//	    ifqueue = (PriQueue*) TclObject::lookup(argv[2]);

			//	      if(ifqueue == 0)
			//		return TCL_ERROR;
//			return TCL_OK;
//		}
		else if (strcmp(argv[1], "port-dmux") == 0) {
			dmux_ = (PortClassifier *)TclObject::lookup(argv[2]);
			if (dmux_ == 0) {
				fprintf (stderr, "%s: %s lookup of %s failed\n", __FILE__,
						argv[1], argv[2]);
				return TCL_ERROR;
			}
			return TCL_OK;
		}
	}

	return Agent::command(argc, argv);
}

void
ZrpHelloTimer::handle(Event*) {
	agent->sendHello();
	double interval = MinHelloInterval +
			((MaxHelloInterval - MinHelloInterval) * Random::uniform());
	assert(interval >= 0);
	Scheduler::instance().schedule(this, &intr, interval);
}

/*
 * Constructor for ZRP class
 */
ZRP::ZRP(nsaddr_t id) : Agent(PT_ZRP),htimer(this) {					// Need to assign a number from packet.h
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
	struct hdr_ip *ih = HDR_IP(p);
	struct hdr_zrp_inter *interh = HDR_ZRP_INTER(p);

	zrp_rt_entry *rt;

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
	if(rq->lk_src_addr = index) {
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

void
ZRP::forward(zrp_rt_entry *rt, Packet *p, double delay) {
	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ih = HDR_IP(p);

	if (ih->ttl_ == 0) {

#ifdef DEBUG
		fprintf(stderr, "%s: calling drop()\n", __PRETTY_FUNCTION__);
#endif // DEBUG

		drop(p, DROP_RTR_TTL);
		return;
	}

	if (ch->ptype() != PT_ZRP && ch->direction() == hdr_cmn::UP && ((u_int32_t) ih->daddr() == IP_BROADCAST) || (ih->daddr() == here_.addr_)) {
		dmux_->recv(p,0);
		return;
	}

	//	if (rt) {
	//		ch->next_hop_ = rt->
	//	}
	ch->addr_type() = NS_AF_INET;
	ch->direction() = hdr_cmn::DOWN;

	if (delay > 0.0) {
		Scheduler::instance().schedule(target_,p,delay);
	}
	else {
		Scheduler::instance().schedule(target_, p, 0);	// No jitter , No delay
	}
}

void
ZRP::sendQuery(nsaddr_t dst) {
	Packet *p = Packet::alloc();
	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ih = HDR_IP(p);
	struct hdr_zrp_inter *rqh = HDR_ZRP_INTER(p);
	zrp_rt_entry *rt = rtable.rt_lookup(dst);

	// No RTF_UP
	// No rt_req_timeout

#ifdef DEBUG
	fprintf(stderr, "(%2d) - %2d sending Route Request, dst: %d\n",
			++route_request, index, rt->rt_dst);
#endif // DEBUG

	// No checking for request count

	ch->ptype() = PT_ZRP;
	ch->size() = IP_HDR_LEN + sizeof(struct hdr_zrp_inter);
	ch->iface() = -2;
	ch->error() = 0;
	ch->addr_type() = NS_AF_NONE;
	ch->prev_hop_ = index;

	ih->saddr() = index;
	ih->daddr() = IP_BROADCAST;
	ih->saddr() = RT_PORT;
	ih->dport() = RT_PORT;

	rqh->lk_src_addr = index;
	rqh->inter_type = ZRPTYPE_RREQ;
	rqh->h_count = 1;
	rqh->curr_hop_ptr = 1;

	Scheduler::instance().schedule(target_, p, 0.);
}

void
ZRP::sendHello() {
	Packet *p = Packet::alloc();
	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ih = HDR_IP(p);
	struct hdr_zrp_inter *rh = HDR_ZRP_INTER(p);

#ifdef DEBUG
	fprintf(stderr, "sending Hello from %d at %.2f\n", index, Scheduler::instance().clock());
#endif // DEBUG

	rh->inter_type = ZRPTYPE_HELLO;
	rh->h_count = 1;
	rh->query_dst = index;
	rh->q_id = seqno;

	// ch->uid() = 0;
	ch->ptype() = PT_ZRP;
	ch->size() = IP_HDR_LEN + sizeof(hdr_zrp_inter);
	ch->iface() = -2;
	ch->error() = 0;
	ch->addr_type() = NS_AF_NONE;
	ch->prev_hop_ = index;          // AODV hack

	ih->saddr() = index;
	ih->daddr() = IP_BROADCAST;
	ih->sport() = RT_PORT;
	ih->dport() = RT_PORT;
	ih->ttl_ = 1;

	Scheduler::instance().schedule(target_, p, 0.0);
}
