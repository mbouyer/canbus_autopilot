#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>

#include <net/if.h>
#include <netcan/can.h>

typedef union nmea2000_canid {
	struct {
		uint8_t source_addr;
		uint8_t pdu_specific;
		uint8_t pdu_format;
		uint8_t data_page : 1;
		uint8_t res       : 1;
		uint8_t	priority  : 3;
		uint8_t	can_local : 3;
	} s;
	uint32_t v;
} nmea2000_canid_t;

#define PGN_PRIVATE_REMOTE_CONTROL 39680
struct private_remote_control {
	char control_type;
	char control_subtype;
	char control_data[6]; /* actually variable type-dependant lenght */
};

#define CONTROL_BEEP    0x04
#define CONTROL_BEEP_SHORT      0x00
#define CONTROL_BEEP_LONG       0x01
#define CONTROL_BEEP_SIZE 2

int
main(int argc, const char *argv[])
{
	int s;
	struct can_frame cf_received;
	struct can_filter cfi;
	nmea2000_canid_t id;
	socklen_t salen;
	struct sockaddr_can sa;
	pid_t child;
	u_int type;

	if (argc < 2) {
		errx(1, "usage: %s <file> [<file>] ...", getprogname());
	}

	if ((s = socket(AF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		err(1, "create CAN socket");
	}

	/* filter for PRIVATE_REMOTE_CONTROL frames */
	id.v = 0;
	id.s.data_page = (PGN_PRIVATE_REMOTE_CONTROL >> 16) & 0x1;
	id.s.pdu_format = (PGN_PRIVATE_REMOTE_CONTROL >> 8) & 0xff;
	cfi.can_id = id.v;
	id.v = 0;
	id.s.data_page = 1;
	id.s.pdu_format = 0xff;
	cfi.can_mask = id.v;

	printf("id 0x%x mask 0x%x\n", cfi.can_id, cfi.can_mask);
	if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &cfi, sizeof(cfi)) < 0) {
		err(1, "setsockopt(CAN_RAW_FILTER)");
	}

	/* and wait for frames */
	while (1) {
		memset(&cf_received, 0, sizeof(struct can_frame));
		salen = sizeof(struct sockaddr_can);
		if (recvfrom(s, &cf_received, sizeof(struct can_frame), 
			    0, (struct sockaddr *)&sa, &salen) < 0) {
			warn("recvfrom");
			continue;
		}
		if (sa.can_family != AF_CAN) {
			warnx("wrong family %d on CAN socket");
			continue;
		}
		id.v = cf_received.can_id;
		if (cf_received.can_dlc < 2) {
			warnx("short can frame");
			continue;
		}
		if (cf_received.data[0] != CONTROL_BEEP || cf_received.can_dlc != CONTROL_BEEP_SIZE) {
			/* not a BEEP message */
			continue;
		}
		type = cf_received.data[1];
		/* we got a BEEP message */
		printf("BEEP %d from source 0x%x\n", type, id.s.source_addr);
		if (type + 1 > argc) {
			warn("wrong BEEP type %d\n", type);
			continue;
		}
			
		switch((child = fork())) {
		case -1:
			warn("fork");
			break;
		case 0:
			execlp("/usr/bin/audioplay", "audioplay", argv[type+1]);
			err(1, "exec");
			break;
		default:
			wait(NULL);
			break;
		}
	}
}
