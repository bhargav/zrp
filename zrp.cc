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
public:	LIST_HEAD(zrp_rthead, zrp_rt_entry) rthead;

ZRPclass() : TclClass("Agent/ZRP") {}

TclObject* create(int argc, const char*const* argv) {
	assert(argc == 5);
	return (new ZRP((nsaddr_t) Address::instance().str2addr(argv[4])));
}
} class_rtProtoZRP;

int
ZRP::command(int argc, const char*const* argv) {
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
			qtimer.handle((Event*) 0);
			ntimer.handle((Event*) 0);
			//	#endif // LINK LAYER DETECTION

			//	      rtimer.handle((Event*) 0);
			return TCL_OK;
		}
		if(strcasecmp(argv[1], "print_rtable") == 0) {
			rt_dump();
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
		else if(strcmp(argv[1], "if-queue") == 0) {
			ifqueue = (PriQueue*) TclObject::lookup(argv[2]);

			if(ifqueue == 0)
				return TCL_ERROR;
			return TCL_OK;
		}
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
	//	double interval = 2;
	assert(interval >= 0);
	Scheduler::instance().schedule(this, &intr, interval);
}

void
ZrpQueryTimer::handle(Event*) {
	if (agent->index == 40)
		agent->sendQuery(50);
	double interval = 50.0;
	assert(interval >= 0);
	Scheduler::instance().schedule(this, &intr, interval);
}

void
ZrpNeighborTimer::handle(Event*) {
	agent->nb_purge();
	Scheduler::instance().schedule(this, &intr, 5 * HELLO_INTERVAL * Random::uniform());
}

/*
 * Constructor for ZRP class
 */
ZRP::ZRP(nsaddr_t id) : Agent(PT_ZRP),ntimer(this),htimer(this),qtimer(this),rtable(),lktable(),pending_lst_list(),new_neighbour_list(),former_routing_zones(),tempqc() {					// Need to assign a number from packet.h
	//	bind("zone_radius", (int64_t *) &zone_radius);
	index = id;
	seqno = 2;										// From AODV need rationale
	zone_radius = 2;
	my_state_id = 1;
	my_reply_id = 1;
	cum_status = ZRP_UPDATE_COMPLETE;
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

	case ZRPTYPE_QUERY:
		recvQuery(p);
		break;

	case ZRPTYPE_LINKSTATE:
		recvLinkState(p);
		break;

	case ZRPTYPE_HELLO:			//Check if we need Hello packets for NDM
		recvHello(p);
		break;

	default:
		fprintf(stderr, "Invalid ZRP type (%x)\n", ah->ah_type);
		exit(1);
	}
}

void ZRP::recvLinkState(Packet *p) {
	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ih = HDR_IP(p);
	struct hdr_zrp *zh = HDR_ZRP(p);
	struct hdr_zrp_linkstate *lh = HDR_ZRP_LINKSTATE(p);

	updateIntraRoutingTable(p, lh->link_dst, FALSE, lh->link_state_id);
	rt_dump();
}

void ZRP::recvExtension(Packet *p) {
	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ip = HDR_IP(p);
	struct hdr_zrp_query *rq = HDR_ZRP_QUERY(p);
	zrp_rt_entry *rt;

#ifdef DEBUG
	fprintf(stderr, "%d - %s: received a QUERY EXTENSION\n", index, __FUNCTION__);
#endif

	// Drop, if i am source.
	if(rq->query_src_addr == index) {
		Packet::free(p);
		return;
	}

	// ih->ttl_ --; Done in recv();
	rq->ttl--;
	rq->hop_count++;
	/*
	// Extract route from packet and record to previous hops
	int count;

	zrp_nodelist prev_hops;
	printf(" current %d ", rq->current_hop_ptr);
	for (int i = 0; i <= rq->current_hop_ptr - 1; i++) {	// current_hop_ptr
		prev_hops.nl_insertNode(rq->route[i]);
	}

	query_cache_entry *qce = tempqc.qc_insert(rq->query_src_addr);
	qce->query_id = rq->query_id;
	qce->hop_count = rq->hop_count;

	tempqc.qc_dump();

	zrp_rt_entry *rten;
	//	printf(" qsc %d ", rq->query_src_addr);

	rten = rtable.rt_lookup(rq->query_src_addr);
	if (rten != 0)
		rtable.rt_delete(rq->query_src_addr);
	rten = rtable.rt_add(rq->query_src_addr);
	//	printf("Inserting to rtable entry %d", rq->query_src_addr);

	rten->zrp_intrazone = FALSE;

	zrp_nodelist_entry *nle = prev_hops.head();
	for (count = 0; nle; nle = nle->nl_link.le_next, count++) {
		qce->prev_hop[count] = nle->node;
	}

	for (int j = count - 1; j > 0; j ++) {
		//			rten->routes.nl_insertNode(qce->prev_hop[count]);
		zrp_nodelist_entry *nlen = new zrp_nodelist_entry;
		nlen->node = qce->prev_hop[count];
		LIST_INSERT_HEAD(&(rten->routes), nlen, nl_link);
		//
	}

	// Extraction and updating routing table complete

	if (rq->query_dst != index) {
		zrp_rt_entry *r = rtable.rt_lookup(rq->query_dst);
		if (r) {
			nsaddr_t nexthop = r->routes.lh_first->node;
			ch->next_hop_ = nexthop;
			//			rq->nexthop = nexthop;
			ip->daddr() = nexthop;

			Scheduler::instance().schedule(target_, p, 0.);
		}
	}*/
}

void ZRP::recvQuery(Packet *p)
{
	struct hdr_ip *ih =  HDR_IP(p);
	struct hdr_zrp_query *rq = HDR_ZRP_QUERY(p);
	zrp_rt_entry *rt;

	printf("\nReceived query from %d by %d", rq->query_src_addr, index);

	// Drop, if i am source.
	if(rq->query_src_addr == index) {
		Packet::free(p);
		return;
	}

	// ih->ttl_ --; Done in recv();
	rq->ttl--;
	rq->hop_count++;
	/*
	// Extract route from packet and record to previous hops
	int count;

	zrp_nodelist prev_hops;
	printf(" current %d ", rq->current_hop_ptr);
	for (int i = 0; i <= rq->current_hop_ptr - 1; i++) {	// current_hop_ptr
		prev_hops.nl_insertNode(rq->route[i]);
	}

	query_cache_entry *qce = tempqc.qc_insert(rq->query_src_addr);
	qce->query_id = rq->query_id;
	qce->hop_count = rq->hop_count;

	tempqc.qc_dump();

	zrp_rt_entry *rten;
	//	printf(" qsc %d ", rq->query_src_addr);

	rten = rtable.rt_lookup(rq->query_src_addr);
	if (rten != 0)
		rtable.rt_delete(rq->query_src_addr);
	rten = rtable.rt_add(rq->query_src_addr);
	//	printf("Inserting to rtable entry %d", rq->query_src_addr);

	rten->zrp_intrazone = FALSE;

	zrp_nodelist_entry *nle = prev_hops.head();
	for (count = 0; nle; nle = nle->nl_link.le_next, count++) {
		qce->prev_hop[count] = nle->node;
	}

	for (int j = count - 1; j > 0; j ++) {
		//			rten->routes.nl_insertNode(qce->prev_hop[count]);
		zrp_nodelist_entry *nlen = new zrp_nodelist_entry;
		nlen->node = qce->prev_hop[count];
		LIST_INSERT_HEAD(&(rten->routes), nlen, nl_link);
		//
	}

	// Extraction and updating routing table complete

	if (rtable.rt_lookup(rq->query_dst)) {
		if (rtable.rt_isIntra(rq->query_dst)) {
			// destination is within this node's routing zone
			// send query extension TODO
			sendQueryExtension(rq->query_dst);

			zrp_rt_entry *rten = rtable.rt_lookup(rq->query_src_addr);
			if (rten)
				sendReply(index, rq->hop_count, rq->query_src_addr, my_reply_id, CURRENT_TIME, rq->route);	// Check Not COrrect
		}
		else {
			// destination is outside this node's routing zone
			// Send packet to BRP
		}
	}
	rt_dump(); */
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
	struct hdr_zrp_query *rqh = HDR_ZRP_QUERY(p);

	zrp_rt_entry *rt = rtable.rt_lookup(dst);

	// No RTF_UP
	// No rt_req_timeout

#ifdef DEBUG
	fprintf(stderr, "(%2d) - %2d sending Route Request, dst: %d\n",
			++route_request, index, rt->rt_dst);
#endif // DEBUG

	// No checking for request count

	ch->ptype() = PT_ZRP;
	ch->size() = IP_HDR_LEN + rqh->size();
	ch->iface() = -2;
	ch->error() = 0;
	ch->addr_type() = NS_AF_NONE;
	ch->prev_hop_ = index;

	ih->saddr() = index;
	ih->daddr() = IP_BROADCAST;		// Query is bordercasted
	ih->sport() = RT_PORT;
	ih->dport() = RT_PORT;

	rqh->query_src_addr = index;
	rqh->query_type = ZRPTYPE_QUERY;
	rqh->hop_count = 0;
	rqh->current_hop_ptr = 1;

	Scheduler::instance().schedule(target_, p, 0.);
}

void
ZRP::sendLinkState(nsaddr_t link_src, nsaddr_t link_dst, u_int16_t state_id, u_int8_t radius, bool* flags, nsaddr_t subnet_mask, bool link_status, nsaddr_t broadcast) {
	Packet *p = Packet::alloc();
	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ih = HDR_IP(p);
	struct hdr_zrp *zrph = HDR_ZRP(p);
	struct hdr_zrp_linkstate *lsh = HDR_ZRP_LINKSTATE(p);

	// No RTF_UP
	// No rt_req_timeout
	// No checking for request count

	ch->ptype() = PT_ZRP;
	ch->size() = IP_HDR_LEN + lsh->size();
	ch->iface() = -2;
	ch->error() = 0;
	ch->addr_type() = NS_AF_NONE;
	ch->prev_hop_ = index;

	ih->saddr() = index;
	ih->ttl_ = 1;
	ih->daddr() = broadcast;
	ih->sport() = RT_PORT;
	ih->dport() = RT_PORT;

	lsh->link_src = link_src;
	lsh->link_dst = link_dst;
	lsh->pkt_src = index;
	lsh->flags[0] = *(flags++);
	lsh->flags[1] = *(flags++);
	lsh->flags[2] = *(flags++);
	lsh->flags[3] = *(flags++);
	lsh->flags[4] = *(flags++);
	lsh->link_state_id = state_id;
	lsh->zone_radius = radius;
	lsh->link_status = link_status;

	if (lsh->flags[3] == 0)
		lsh->link_dst_subnet = subnet_mask;

	zrph->ah_type = ZRPTYPE_LINKSTATE;

	Scheduler::instance().schedule(target_, p, 0.);

}

void
ZRP::sendQueryExtension(nsaddr_t dst) {
	Packet *p = Packet::alloc();
	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ih = HDR_IP(p);
	struct hdr_zrp *zh = HDR_ZRP(p);
	struct hdr_zrp_query *rqh = HDR_ZRP_QUERY(p);

	zrp_rt_entry *rt = rtable.rt_lookup(dst);

	// No RTF_UP
	// No rt_req_timeout
	// No checking for request count

#ifdef DEBUG
	fprintf(stderr, "(%2d) - %2d sending Query Extension , dst: %d\n",
			++route_request, index, rt->rt_dst);
#endif // DEBUG

	if (rt) {
		zh->ah_type = ZRPTYPE_QUERYEXTENSION;

		ch->ptype() = PT_ZRP;
		ch->size() = IP_HDR_LEN + rqh->size();
		ch->iface() = -2;
		ch->error() = 0;
		ch->addr_type() = NS_AF_NONE;
		ch->prev_hop_ = index;
		//		ch->next_hop_ = rt->routes.lh_first->node; TODO

		ih->saddr() = index;
		//		ih->daddr() = rt->routes.lh_first->node; TODO
		ih->sport() = RT_PORT;
		ih->dport() = RT_PORT;

		rqh->query_src_addr = index;
		rqh->query_id = seqno;
		rqh->query_dst = dst;
		rqh->query_type = ZRPTYPE_QUERYEXTENSION;
		rqh->hop_count = 0;
		rqh->current_hop_ptr = 1;

		rqh->route[0] = index;

		int i;	ih->daddr() = IP_BROADCAST;

		//		zrp_nodelist_entry *rtentry = rt->routes.lh_first;
		//		for (i= 1; rtentry; rtentry = rtentry->nl_link.le_next ) {
		//			rqh->route[i] = rtentry->node;
		//			i++;
		//		}TODO

		rqh->num_nodes = i;
		ih->ttl_ = i + 1;
		rqh->ttl = i + 1;
		rqh->num_dest = 1;

		Scheduler::instance().schedule(target_, p, 0.);
	}
}

void
ZRP::sendReply(nsaddr_t ipdst, u_int32_t hop_count, nsaddr_t rpdst, u_int32_t rpseq, double timestamp,nsaddr_t* route_)
{
	Packet *p = Packet::alloc();
	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ih = HDR_IP(p);
	struct hdr_zrp *zh = HDR_ZRP(p);
	struct hdr_zrp_query *rp = HDR_ZRP_QUERY(p);

	//	zrp_rt_entry *rt = rtable.rt_lookup(rpdst); // Inject path from temporary cache

#ifdef DEBUG
	fprintf(stderr, "sending Reply from %d at %.2f\n", index, Scheduler::instance().clock());
#endif // DEBUG
	//	assert(rt);

	zh->ah_type = ZRPTYPE_REPLY;

	rp->hop_count = hop_count;
	rp->query_dst = rpdst;
	rp->query_id = rpseq;
	rp->query_src_addr = index;
	rp->rq_timestamp = timestamp;
	for (int j = 0; j < ZRP_MAX_ERRORS; j++)
		rp->route[j] = *(route_ + j);

	// ch->uid() = 0;
	ch->ptype() = PT_ZRP;
	ch->size() = IP_HDR_LEN + rp->size();
	ch->iface() = -2;
	ch->error() = 0;
	ch->addr_type() = NS_AF_INET;
	ch->next_hop_ = ipdst;							// Need rationale TODO
	ch->prev_hop_ = index;
	ch->direction() = hdr_cmn::DOWN;

	ih->saddr() = index;
	ih->daddr() = ipdst;
	ih->sport() = RT_PORT;
	ih->dport() = RT_PORT;

	Scheduler::instance().schedule(target_, p, 0.);

}

void
ZRP::sendHello() {
	Packet *p = Packet::alloc();
	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ih = HDR_IP(p);
	struct hdr_zrp *zrph = HDR_ZRP(p);
	struct hdr_zrp_query *rh = HDR_ZRP_QUERY(p);

#ifdef DEBUG
	fprintf(stderr, "sending Hello from %d at %.2f\n", index, Scheduler::instance().clock());
#endif // DEBUG

	zrph->ah_type = ZRPTYPE_HELLO;

	rh->query_type = ZRPTYPE_HELLO;
	rh->hop_count = 1;
	//	rh->query_dst[0] = index;
	rh->query_id = seqno;
	rh->query_src_addr = index;

	// ch->uid() = 0;
	ch->ptype() = PT_ZRP;
	ch->size() = IP_HDR_LEN + rh->size();
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

void
ZRP::recvHello(Packet *p) {
	struct hdr_zrp_query *qh = HDR_ZRP_QUERY(p);

	//	printf("Received hello %d with %d radius \n", index, zone_radius);

	ZRP_Neighbor *nb;
	nb = nb_lookup(qh->query_src_addr);
	if(nb == 0) {
		nb_insert(qh->query_src_addr);
		neighborFound(qh->query_src_addr);
	}
	else {
		nb->nb_expire = CURRENT_TIME + ALLOWED_NEIGHBOR_LOSS;
	}

	Packet::free(p);
}

void
ZRP::nb_dump() {
	ZRP_Neighbor *nb = nbhead.lh_first;
	printf("Nb cache entries of %d -- ", index);
	for(; nb; nb = nb->nb_link.le_next) {
		printf(" %d ", nb->nb_addr);
	}
	printf(" complete dumping \n");
}

void
ZRP::nb_insert(nsaddr_t id) {
	ZRP_Neighbor *nb = new ZRP_Neighbor(id);

	//	nb_dump();
	assert(nb);
	nb->nb_expire = CURRENT_TIME + ALLOWED_NEIGHBOR_LOSS;
	LIST_INSERT_HEAD(&nbhead, nb, nb_link);
	seqno += 2;            	 // set of neighbors changed -- TODO
	//	assert ((seqno%2) == 0);
	//	nb_dump();
	// Neighbor Found

}


ZRP_Neighbor*
ZRP::nb_lookup(nsaddr_t id) {
	ZRP_Neighbor *nb = nbhead.lh_first;

	for(; nb; nb = nb->nb_link.le_next) {
		if(nb->nb_addr == id) break;
	}
	return nb;
}


/*
 * Called when we receive *explicit* notification that a Neighbor
 * is no longer reachable.
 */
void
ZRP::nb_delete(nsaddr_t id) {
	ZRP_Neighbor *nb = nbhead.lh_first;

	// log_link_del(id);
	seqno += 2;     // Set of neighbors changed
	assert ((seqno%2) == 0);

	for(; nb; nb = nb->nb_link.le_next) {
		if(nb->nb_addr == id) {
			LIST_REMOVE(nb,nb_link);
			delete nb;
			break;
		}
	}

	neighborLost(id);
	//	nb_dump();
	// Neighbor Lost

	// handle_link_failure(id);

}


/*
 * Purges all timed-out Neighbor Entries - runs every
 * HELLO_INTERVAL * 1.5 seconds.
 */
void
ZRP::nb_purge() {
	ZRP_Neighbor *nb = nbhead.lh_first;
	ZRP_Neighbor *nbn;
	double now = CURRENT_TIME;

	for(; nb; nb = nbn) {
		nbn = nb->nb_link.le_next;
		if(nb->nb_expire <= now) {
			nb_delete(nb->nb_addr);
		}
	}
}


void ZRP::neighborFound(nsaddr_t id)
{
	printf("Neighbor Found at time %.17f by %d : %d \n", CURRENT_TIME,index, id);
	zrp_nodelist_entry *nh = new_neighbour_list.nl_lookup(id);

	if (nh == 0)
		new_neighbour_list.nl_insertNode(id);

	zrp_rt_entry *nben = rtable.rt_lookup(id);
	if (!nben)
		rtable.rt_delete(id);

	nben = rtable.rt_add(id);
	if (zone_radius >=1 )
		nben->zrp_intrazone = TRUE;
	nben->route.push_back(id);

	rt_dump();

	//	bool fl[8] = {0,0,0,0,1,0,0,0};
	//	sendLinkState(index,id,my_state_id,zone_radius, fl, NULL, ZRP_UP,IP_BROADCAST);


	updateIntraRoutingTable(NULL, id, 0, ZRP_UP);

	lktable.lst_dump();
}

void ZRP::neighborLost(nsaddr_t id)
{
	printf("Neighbor Lost at %.17f from %d : %d \n", CURRENT_TIME, index, id);

	zrp_nodelist_entry *nh = new_neighbour_list.nl_lookup(id);
	if (nh != 0)
		new_neighbour_list.nl_delete(id);

	// Routing Table Cleanup
	rtable.rt_delete(id);
	zrp_rt_entry *rt = rtable.head();

	std::vector<nsaddr_t> temp;
	std::vector<nsaddr_t>::iterator ii;
	for (; rt; rt=rt->rt_link.le_next) {
		for(ii = rt->route.begin(); ii!= rt->route.end(); ++ii) {
			if (*ii == id) {
				temp.push_back(*ii);
				break;
			}
		}
	}

	for(ii = temp.begin(); ii!=temp.end(); ++ii) {
		rtable.rt_delete(*ii);
	}
	// Routing Table Cleanup Complete

	rt_dump();
	lktable.lst_dump();

	updateIntraRoutingTable(NULL, id, 0, ZRP_DOWN);
}

void
ZRP::rt_dump()
{
	printf("Dumping Routing Table for %d at %.17f -- ", index, CURRENT_TIME);
	zrp_rt_entry *rt = rtable.head();
	for (; rt; rt=rt->rt_link.le_next) {
		printf(" [ d:%d inzone:%d r: ", rt->zrp_dst, rt->zrp_intrazone);
		std::vector<nsaddr_t>::reverse_iterator rii;
		for(rii=rt->route.rbegin(); rii!=rt->route.rend(); ++rii)  {
			printf("%d - ", *rii);
		}
		printf(" ]");
	}
	printf(" -- done \n");
}

void
ZRP::updateIntraRoutingTable(Packet *pkt, nsaddr_t link_dest, bool mask, bool ifneigh_status) {
	printf("Updating Routing Table Case %d", (pkt == 0));
	bool my_link_changed;
	nsaddr_t link_source;
	nsaddr_t pk_source;
	u_int16_t state_id;
	u_int8_t	radius;
	bool full_link_state,current_update,all_updates,mask_;
	bool link_status;

	if (pkt) {
		struct hdr_cmn *ch = HDR_CMN(pkt);
		struct hdr_ip *ih = HDR_IP(pkt);
		struct hdr_zrp_linkstate *lsh = HDR_ZRP_LINKSTATE(pkt);

		my_link_changed = FALSE;
		link_source = lsh->link_src;
		pk_source = lsh->pkt_src;
		state_id = lsh->link_state_id;
		radius = lsh->zone_radius;

		full_link_state = lsh->flags[0];
		current_update = lsh->flags[1];
		all_updates = lsh->flags[2];
		mask_ = lsh->flags[3];
		link_status = lsh->flags[4];
	}
	else {
		link_source = index;
		pk_source = index;
		state_id = my_state_id;
		radius = zone_radius;

		full_link_state = 0;
		current_update = ZRP_COMPLETE;
		all_updates = ZRP_INCOMPLETE;
		link_status = ifneigh_status;			// If Neighbor Found or Lost
	}

	if (pending_lst_list.nl_lookup(pk_source) == 0)
		pending_lst_list.nl_insertNode(pk_source);

	bool lst_status;

	// Add to Link_State_Table Return status
	if (link_status == ZRP_UP) {
		bool fl[8] = {full_link_state,current_update,all_updates,mask_,link_status, 0, 0, 0};
		lst_status = lktable.lst_insert(link_source, link_dest, 0, radius, state_id, fl);
	}
	else {
		// REMOVE Link State Update for lost link
		// strategy negative update or refine table TODO
	}

	printf(" For node %d ::", index);
	lktable.lst_dump();

	if(lst_status == ZRP_NEW_LINK_INFO)	{							// If NEW_LINK_INFO
		cum_status = ZRP_UPDATE_IN_PROGRESS;
		if (my_link_changed) {
			if (ifneigh_status == ZRP_UP) {
				if(new_neighbour_list.nl_lookup(link_dest))
					new_neighbour_list.nl_insertNode(link_dest);
			}
			else {
				new_neighbour_list.nl_delete(link_dest);
			}
			my_state_id++;
		}
	}

	if (all_updates == ZRP_COMPLETE)
		pending_lst_list.nl_delete(pk_source);

	if (pending_lst_list.nl_isempty() && (cum_status == ZRP_UPDATE_IN_PROGRESS)) {

		// Do Routing Table
		zrp_rt_entry *rtentry = rtable.head();
		for (; rtentry; rtentry = rtentry->rt_link.le_next) {
			if (rtentry->zrp_intrazone)
				former_routing_zones.nl_insertNode(rtentry->zrp_dst);
		}

		// Rebuild Routing Table
		bool rebuild = TRUE;
		std::vector<nsaddr_t> temp;
		std::vector<nsaddr_t>::iterator iter;
		std::vector<zrp_lst_entry*>::iterator lsmit;
		std::vector<lsinfo_entry>::iterator initer;
		std::vector<ls_subentry>::iterator sbit;

		while(rebuild) {

			// Remove entries for IntraZone Nodes
			for (; rtentry; rtentry = rtentry->rt_link.le_next) {
				if (rtentry->zrp_intrazone)
					temp.push_back(rtentry->zrp_dst);
			}

			for (iter = temp.begin(); iter != temp.end(); ++iter) {
				rtable.rt_delete((*iter));
			}

			// Assuming all links in LST are UP links only and DOWN links have been removed
			zrp_rt_entry *rt;

			std::vector<nsaddr_t> nodes;
			for (lsmit = lktable.lshead.begin(); lsmit != lktable.lshead.end(); ++lsmit) {
				if ((*lsmit)->link_src == index) {
					for (initer = (*lsmit)->lsinfo.begin(); initer != (*lsmit)->lsinfo.end(); ++initer) {
						for (sbit = (*initer).ls_info.begin(); sbit != (*initer).ls_info.end(); ++sbit) {
							if ((*sbit).link_status) {
								nodes.push_back((*sbit).link_dst);

								rt = rtable.rt_add((*sbit).link_dst);
								rt->zrp_intrazone = TRUE;					// Assuming Zone_Radius >= 1 (i.e. atleast neighbors are in the zone)
								rt->route.push_back((*sbit).link_dst);
							}
						}
					}
				}
			}

			// Recompute entries for IntraZone Nodes in Routing Table
			std::vector<nsaddr_t>::iterator ii;
			std::vector<nsaddr_t> nodes_new;
			std::vector<nsaddr_t> level;
			level.push_back(index);
			int i = 1;

			for (lsmit = lktable.lshead.begin(); lsmit != lktable.lshead.end(); ++lsmit) {
				// Compute Route to each node in LST from Link State Table

				// Check is size of route is less than zone radius

				// Add to Routing Table as IntraZone
			}

			rebuild = FALSE; // Update Link State Tables
		}
	}

	// Construct Bordercast Tree -- BANNED

	// broadcast_link_state_updates

	std::vector<zrp_lst_entry*>::iterator mit;
	std::vector<lsinfo_entry>::iterator lsitr;
	std::vector<ls_subentry>::iterator it;

	for (mit = lktable.lshead.begin(); mit != lktable.lshead.end(); ++mit) {

		for (lsitr = (*mit)->lsinfo.begin(); lsitr != (*mit)->lsinfo.end(); ++lsitr) {

			for (it = (*lsitr).ls_info.begin(); it != (*lsitr).ls_info.end(); ++it) {

				// Send Link State Table to new neighbors
				zrp_nodelist_entry *nle;
				nle = new_neighbour_list.head();
				for (; nle; nle = nle->nl_link.le_next) {
					bool flg[8] = {0, 0,0,0,(*it).link_status,0,0,0};
					sendLinkState((*mit)->link_src, (*it).link_dst, (*lsitr).lst_id, (*mit)->zone_radius, flg, 0, (*it).link_status, nle->node);
				}
				// Done

				// Broadcast Previously unforwarded link state entries
				if ((*it).forwarded = FALSE) {
					bool flg[8] = {0, 0, 0, 0, (*it).link_status, 0, 0, 0};
					sendLinkState((*mit)->link_src, (*it).link_dst, (*lsitr).lst_id, (*mit)->zone_radius, flg, 0, (*it).link_status, IP_BROADCAST);
				}
				(*it).forwarded = TRUE;
				// Broadcasting done
			}
		}

	}

	new_neighbour_list.nl_purge();

	cum_status = ZRP_UPDATE_COMPLETE;

	zrp_nodelist_entry *nle;
	nle = former_routing_zones.head();
	for (; nle; nle = nle->nl_link.le_next) {

		// Delete all routes in the table containing

		if (rtable.rt_lookup(nle->node) == 0) {
			// Report Lost Nodes to IERP

			// Routing Table Cleanup

			std::vector<nsaddr_t> temp;
			std::vector<nsaddr_t>::iterator ii;
			zrp_rt_entry *rt = rtable.head();
			nsaddr_t remv = nle->node;
			for (; rt; rt=rt->rt_link.le_next) {
				if (rt->zrp_intrazone == TRUE) {
					for(ii = rt->route.begin(); ii!= rt->route.end(); ++ii) {
						if (*ii == remv) {
							temp.push_back(*ii);
							break;
						}
					}

					for(ii = temp.begin(); ii!=temp.end(); ++ii) {
						rtable.rt_delete(*ii);
					}

				}

				// Routing Table Cleanup Complete
			}
		}
	}

	former_routing_zones.nl_purge();
}
