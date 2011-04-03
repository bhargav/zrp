#include <aodv/aodv.h>
#include <aodv/aodv_packet.h>
#include <random.h>
#include <cmu-trace.h>

#define max(a,b)			( (a) > (b) ? (a) : (b) )
#define CURRENT_TIME		Scheduler::instance().clock()

/*
 * TCL Hooks
 */

static class ZRPHeaderClass : public PacketHeaderClass {
public:

} class_rtProtoZRP_hdr;

static class ZRPclass : public TclClass {
public:
	ZRPclass() : TclClass("Agent/ZRP") {}

	TclObject* create(int argc, const char*const* argv) {
	//	assert(argc == 5);
	}
} class_rtProtoZRP;
