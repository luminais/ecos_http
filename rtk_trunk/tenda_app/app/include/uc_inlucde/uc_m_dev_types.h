#ifndef __UC_M_DEV_TYPES_H__
#define __UC_M_DEV_TYPES_H__

#define SET_DEV_X(dev, x)	\
	((dev)->mask |= (1 << x))
#define HAS_DEV_X(dev, x)	\
	(((dev)->mask & (1 << x)) == (1 << x))

enum {
	_DEV_NICK_NAME_INFO = 0,
};
	
typedef struct identity_s {
	char val[32];
} identity_t;

typedef struct name_pair_s {
	identity_t id; /* IP or MAC */
	char nickname[64];	//dev alias name
} name_pair_t;

typedef struct nickname_info_s {
	int cnt;
	name_pair_t name_pairs[0];
} nickname_info_t;

typedef struct dev_name_s {
	int cnt;
	identity_t ids[0]; /* IP or MAC */
} dev_name_t;

#define SET_DEV_ACK_NICK_NAME_INFO(ack)	\
	SET_DEV_X(ack, _DEV_NICK_NAME_INFO)
#define HAS_DEV_ACK_NICK_NAME_INFO(ack)	\
	HAS_DEV_X(ack, _DEV_NICK_NAME_INFO)
	
typedef struct dev_common_ack_s {
	int mask;
	int err_code;
	nickname_info_t	info;
}dev_common_ack_t;
#endif