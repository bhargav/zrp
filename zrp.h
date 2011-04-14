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
#define ALLOWED_NEIGHBOR_LOSS	(3 * MaxHelloInterval)	// Rationale .. if we do not get 3 hello's assume lost

// Macros for my use
#define ZRP_COMPLETE			1
#define ZRP_INCOMPLETE			0
#define ZRP_UP					1
#define ZRP_DOWN				0
#define ZRP_UPDATE_IN_PROGRESS  1
#define ZRP_UPDATE_COMPLETE		0

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
	ZRP 	*agent;
	Event 	intr;
};

/*
 *  the Routing Agent
 */

class ZRP: public Agent {
	friend class zrp_rt_entry;
	friend class zrp_lstable;
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
	uint32_t 			seqno;					// Sequence Number
	uint8_t				zone_radius;			// Zone Radius
	uint16_t			my_state_id;
	uint16_t			my_reply_id;

	bool 				cum_status;				// Updating Routing Table

	// Neighbor Table Management
	zrp_ntable         	nbhead;                 // Neighbor Cache
	void            	nb_insert(nsaddr_t id);
	ZRP_Neighbor*   	nb_lookup(nsaddr_t id);
	void            	nb_delete(nsaddr_t id);
	void            	nb_purge(void);
	void				nb_dump();

	// Packet Transmission Routines
	void 				forward(zrp_rt_entry*, Packet*, double);
	void				sendQuery(nsaddr_t);
	void				sendLinkState(nsaddr_t link_src, nsaddr_t link_dst, u_int16_t state_id, u_int8_t radius, bool* flags, nsaddr_t subnet_mask, bool link_status);
	void				sendQueryExtension(nsaddr_t);
	void				sendReply(nsaddr_t ipdst, u_int32_t hop_count, nsaddr_t rpdst,u_int32_t rpseq, double timestamp,nsaddr_t*);
	void				sendHello();

	// Packet Reception Routines
	void 				recvZRP(Packet *p);
	void 				recvLinkState(Packet *p);
	void 				recvHello(Packet *p);
	void				recvQuery(Packet *p);
	void				recvReply(Packet *p);
	void 				recvExtension(Packet *p);

	// Routing Table
	zrp_rtable			rtable;				// Routing Table
	void				updateIntraRoutingTable(Packet*, nsaddr_t, bool, bool);


	void 				rt_dump();			// Print the routing table for the present node

	// Link State IARP
	zrp_nodelist 		pending_lst_list;
	zrp_nodelist 		new_neighbour_list;
	zrp_nodelist 		former_routing_zones;
	zrp_lstable			lktable;
	void 				neighborFound(nsaddr_t);
	void 				neighborLost(nsaddr_t);

	// IERP
	query_cache 		tempqc;
	// Bordercast Tree Table


	// Timer Management
	ZrpHelloTimer      	htimer;				// Hello Timer
	ZrpNeighborTimer   	ntimer;				// Neighbor Table Management Timer

	// logging the contents of the routing databases
	Trace 				*logtarget;
	PriQueue 			*ifqueue;

	/* for passing packets up to agents */
	PortClassifier *dmux_;
};
#endif
