#ifndef __zrp_h__
#define __zrp_h__

#include <cmu-trace.h>
#include <packet.h>
#include <zrp/zrp_rtable.h>
#include <zrp/zrp_rqueue.h>
#include <classifier/classifier-port.h>

class ZRP;						// Forward Declaration

/*
 * Zone Management Timer
 */

class ZoneManagementTimer : public Handler {
public:
	ZoneManagementTimer(ZRP* a) : agent(a) {}
	void handle(Event*);
private:
	ZRP *agent;
	Event intr;
};

/*
 *  the Routing Agent
 */

class ZRP: public Agent {
	friend class zrp_rt_entry;
	friend class ZoneManagementTimer;

public:
	ZRP(nsaddr_t);

	void recv(Packet *,Handler *);
protected:
	int command(int, const char *const*);
	int initialized() { return 1 && target_; }

	nsaddr_t index;					// IP Address of this node
	u_int32_t seqno;				// Sequence Number


	//
	// Packet Reception Routines
	void 	recvZRP(Packet *);
	void 	recvReply(Packet *);

	// Routing Table
	zrp_rtable rtable;				// Routing Table

	// Bordercast Tree Table

	/*
	 * logging the contents of the routing databases
	 */
	Trace *logtarget;

	/* for passing packets up to agents */
	PortClassifier *dmux_;
};
#endif
