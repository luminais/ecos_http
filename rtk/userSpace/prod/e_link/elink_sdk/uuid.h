
#ifndef __UUID_H_
#define __UUID_H_
#if defined(__ECOS)
#include<sys/bsdtypes.h>
#else
#include <stdint.h>
#endif
typedef unsigned char uuid_t[16];

struct uuid {
	uint32_t	time_low;
	uint16_t	time_mid;
	uint16_t	time_hi_and_version;
	uint16_t	clock_seq;
	uint8_t	node[6];
};

void uuid_generate_random(uuid_t out);
void uuid_unparse(const uuid_t uu, char *out);

#endif /* __UUID_H_ */
