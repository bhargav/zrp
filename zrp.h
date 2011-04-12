#ifndef __zrp_h__
#define __zrp_h__

#include <cmu-trace.h>
#include <packet.h>
#include <zrp/zrp_rtable.h>
#include <queue/priqueue.h>
#include <classifier/classifier-port.h>

class ZRP;						// Forward Declaration

/*
 * Zone Management Timer
 */
#define HELLO_INTERVAL          1               // 1000 ms
#define ALLOWED_HELLO_LOSS      3               // packets
#define BAD_LINK_LIFETIME       3               // 3000 ms
#define MaxHelloInterval        (1.25 * HELLO_INTERVAL)
#define MinHelloInterval        (0.75 * HELLO_INTERVAL)
#define ALLOWED_NEIGHBOR_LOSS	 (3 * MaxHelloInterval)	// Rationale .. if we do not get 3 hello's assume lost

class ZrpHelloTimer : public Handler {
public:
	ZrpHelloTimer(ZRP* a) : agent(a) {}
	void	handle(Event*);
private:
	ZRP    *agent;
	Event	intr;
};

class ZrpNeighborTimer : public Handler {
public:
	ZrpNeighborTimer(ZRP* a) : agent(a) {}
	void	handle(Event*);
private:
	ZRP    *agent;
	Event	intr;
};

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
	friend class ZrpHelloTimer;
	friend class ZrpNeighborTimer;

public:
	ZRP(nsaddr_t);

	void recv(Packet *,Handler *);
protected:
	int command(int, const char *const*);
	int initialized() { return 1 && target_; }

	nsaddr_t 			index;					// IP Address of this node
	u_int32_t 			seqno;					// Sequence Number
	u_int8_t			zone_radius;			// Zone Radius


	zrp_ntable         	nbhead;                 // Neighbor Cache

	// Neighbor Table Management
	void            	nb_insert(nsaddr_t id);
	ZRP_Neighbor*   	nb_lookup(nsaddr_t id);
	void            	nb_delete(nsaddr_t id);
	void            	nb_purge(void);
	void				nb_dump();

	// Packet Transmission Routines
	void 				forward(zrp_rt_entry*, Packet*, double);
	void				sendQuery(nsaddr_t);
	void				sendHello();

	// Packet Reception Routines
	void 				recvZRP(Packet *p);
	void 				recvLinkState(Packet *p);
	void 				recvHello(Packet *p);
	void				recvQuery(Packet *p);
	void 				recvExtension(Packet *p);

;	// Routing Table
	zrp_rtable			rtable;				// Routing Table

	// Bordercast Tree Table

	// Timer Management
	ZrpHelloTimer      	htimer;				// Hello Timer
	ZrpNeighborTimer   	ntimer;				// Neighbor Table Management Timer

	/*
	 * logging the contents of the routing databases
	 */
	Trace 				*logtarget;
	PriQueue 			*ifqueue;
	/* for passing packets up to agents */
	PortClassifier *dmux_;
};
#endif
