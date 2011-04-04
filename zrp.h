#ifndef __zrp_h__
#define __zrp_h__

#include <cmu-trace.h>
#include <packet.h>
#include <zrp/zrp_rtable.h>
#include <zrp/zrp_rqueue.h>
#include <classifier/classifier-port.h>

class ZRP;						// Forward Declaration

/*
 *  The Routing Agent
 */
class ZRP: public Agent {

public:

protected:
	int command(int, const char *const*);
	int initialized() { return 1 && target_; }

	nsaddr_t index;					// IP Address of this node
	u_int32_t seqno;				// Sequence Number

	/*
	 * logging the contents of the routing databases
	 */
	Trace *logtarget;

	/* for passing packets up to agents */
	PortClassifier *dmux_;
};
#endif
