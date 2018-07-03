#ifndef ELINK_PROTO_HEAD__
#define ELINK_PROTO_HEAD__


/*
	E-link proto state define.

*/
/*
#define DEVICE_STATE_EXIT          (-1) 
#define DEVICE_STATE_RESET         (0) // State For clear and delay jobs.
#define DEVICE_STATE_INIT          (1)
#define DEVICE_STATE_CONNECT       (2)
#define DEVICE_STATE_NOGO_KEY      (3)
#define DEVICE_STATE_DH            (4)
#define DEVICE_STATE_REG           (5)
#define DEVICE_STATE_CONFIG        (6)
*/
typedef enum
{
	DEVICE_STATE_INIT = 0,
	DEVICE_STATE_CONNECT,       
	DEVICE_STATE_NOGO_KEY,
	DEVICE_STATE_DH,
	DEVICE_STATE_REG,
	DEVICE_STATE_CONFIG,
	DEVICE_STATE_RESET,		//State For clear and delay jobs.
	DEVICE_STATE_EXIT,	
}DEVICE_STATE;

typedef int (*device_state_process_func)(void *input);

/*elink version*/
#define ELINK_VERSION "V2017.1.0"

/*
	E-link proto timer define. See protocol.
*/
#define DELAY_T1 (10)
#define DELAY_T2 (10)
#define DELAY_T3 (20)
#define DELAY_T4 (20)
#define DELAY_T5 (5)
#define DELAY_T6 (5)
#define DELAY_T7 (5)
#define DELAY_T8 (2)
#define DELAY_T9 (5)

#define DELAY_TMAX_WAIT_ACK (30)

/*
	E-link proto, GateWay Listen PORT.
*/
#define GATE_WAY_PORT (32768)

/*
	E-link proto Header struct.
*/
#pragma pack(1)

typedef struct ELINK_HEADER
{
    unsigned int magic;  // Net order, 0x3f721fb5
    unsigned int length; // Net order, length of data after header(not contain head).
    unsigned char  data[0];
}ELINK_HEADER_STRU;
#pragma pack()


#define ELINK_HEADER_LENGTH (8)
#define ELINK_HEADER_MAGIC (0x3f721fb5)

#define MOD_16_INTGER(num) (num+(16-num%16)%16)

/*
	E-link proto Message type string.
*/
#define ELINK_MESSAGE_TYPE_KEY_NEG_REQ "keyngreq"
#define ELINK_MESSAGE_TYPE_KEY_NEG_ACK "keyngack"

#define ELINK_MESSAGE_TYPE_DH_ALGO "dh"

#define ELINK_MESSAGE_TYPE_DEV_REGISTER "dev_reg"

#define ELINK_MESSAGE_TYPE_ACK "ack"

#define ELINK_MESSAGE_TYPE_KEEP_ALIVE "keepalive"

#define ELINE_MESSAGE_TYPE_CFG "cfg"

#define ELINE_MESSAGE_TYPE_GET_STATUS "get_status"

#define ELINE_MESSAGE_TYPE_STATUS "status"

#define ELINE_MESSAGE_TYPE_DEASSOC "deassociation"

#define ELINE_MESSAGE_TYPE_GETRSSIINFO "getrssiinfo"

#define ELINE_MESSAGE_TYPE_RSSIINFO "rssiinfo"

#define ELINE_MESSAGE_TYPE_DEV_REPORT "dev_report"

#endif /* ELINK_PROTO_HEAD__ */

