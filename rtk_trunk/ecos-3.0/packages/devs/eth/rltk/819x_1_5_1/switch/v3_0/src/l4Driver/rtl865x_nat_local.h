#ifndef	RTL865X_NAT_LOCAL_H
#define	RTL865X_NAT_LOCAL_H

struct nat_host_info {
	ipaddr_t						ip;
	uint16						port;
};

struct nat_tuple {
	#if defined(CONFIG_RTL_REDUCE_MEMORY)
	ipaddr_t						in_host_ip;
	ipaddr_t						ext_host_ip;
	ipaddr_t						rem_host_ip;
	uint16						in_host_port;
	uint16						ext_host_port;
	uint16						rem_host_port;
	#else
	struct nat_host_info			int_host;
	struct nat_host_info			ext_host;
	struct nat_host_info			rem_host;
	#endif
	uint16						proto;
};


struct nat_entry {
	struct nat_tuple				tuple_info;

#if defined(CONFIG_RTL_REDUCE_MEMORY)
	uint8						natip_idx;
	uint8 						flags;
	uint16						in;
	uint16						out;
#else
	uint32						natip_idx;
	uint32 						flags;
	uint32						in;
	uint32						out;
#endif

#if defined (CONFIG_RTL_INBOUND_COLLISION_AVOIDANCE)
	uint32						reserveTime;
#endif
#if defined(CONFIG_RTL_REDUCE_MEMORY)
	#define int_ip_					tuple_info.int_host_ip
	#define int_port_					tuple_info.int_host_port
	#define ext_ip_					tuple_info.ext_host_ip
	#define ext_port_					tuple_info.ext_host_port
	#define rem_ip_					tuple_info.rem_host_ip
	#define rem_port_				tuple_info.rem_host_port
	#define proto_					tuple_info.proto
#else
	#define int_ip_					tuple_info.int_host.ip
	#define int_port_					tuple_info.int_host.port
	#define ext_ip_					tuple_info.ext_host.ip
	#define ext_port_					tuple_info.ext_host.port
	#define rem_ip_					tuple_info.rem_host.ip
	#define rem_port_					tuple_info.rem_host.port
	#define proto_					tuple_info.proto
#endif
};

struct nat_table {
	int32						connNum;		/* MUST equal or more than actually conntrack number */
	int32						freeEntryNum;
	int32						tcp_timeout;
	int32						udp_timeout;
	struct nat_entry 				nat_bucket[RTL8651_TCPUDPTBL_SIZE];
};

typedef struct rtl865x_naptHashInfo_s{
	 uint32 outIndex;
	 uint32 inIndex;
	 uint8 outCollision;
	 uint8 inCollision;
	 uint8 sameFourWay;
	 uint8 sameLocation;
	 uint8  inFreeCnt;
}rtl865x_naptHashInfo_t;

#endif

