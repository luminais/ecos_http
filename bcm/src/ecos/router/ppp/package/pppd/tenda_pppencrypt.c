
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#include <bcmnvram.h>
//int iflib_getifhwaddr(char *ifname, unsigned char ifmac[6]);
#include <iflib.h>
#include <sys/md5.h>

#include "pppd.h"

#ifdef CONFIG_CHINA_NET_CLIENT  


#if 0
#define pprintf printf
#else
#define pprintf(format,args...)
#endif

extern int nvram_set(const char *name, const char *value);
/*add by liangia*/
extern void sys_pppoe_disconn();

void sub_00402CCD(char *a)
{
	char tmp;
	int i;
	for(i=0; i<16; i++)
	{
		tmp = *(a+i*4);
		*(a+i*4) = *(a+i*4+3);
		*(a+i*4+3) = tmp;
		tmp = *(a+i*4+1);
		*(a+i*4+1) = *(a+i*4+2);
		*(a+i*4+2) = tmp;
	}
	return;
}

void sub_402335(char *arg1, char *arg2)
{
	unsigned int p1;
	unsigned int p2;
	unsigned int p3;
	unsigned int p4;
	unsigned int p5;
	unsigned int p6;
	unsigned int p7;
	unsigned int p8;
	unsigned int p9;
	unsigned int p10;
	unsigned int p11;
	unsigned int p12;
	unsigned int p13;
	unsigned int p14;
	unsigned int p15;
	unsigned int p16;
	unsigned int p17;

	unsigned int t1;
	unsigned int t2;
	unsigned int t3;
	unsigned int t4;
	unsigned int t5;
	unsigned int t6;
	unsigned int tt1;
	unsigned int tt2;

	sub_00402CCD(arg2);
	tt1 = *((unsigned int *)arg2);

	t3 = *((unsigned int *)(arg1+4));
	t1 = *((unsigned int *)(arg1+12));
	t5 = *((unsigned int *)(arg1+8));
	t4 = t3;
	t4 = ~t4;
	t4 &= t1;
	t2 = *((unsigned int *)arg1);
	t1 = t5;
	t1 &= t3;
	t4 |= t1;
	t1 = tt1;
	t4 += t1;
	t2 = t2 + t4 + 0xD76AA478;
	t1 = t2;
	t1 >>= 0x19;
	t2 <<= 0x7;
	t1 |= t2;
	t2 = *((unsigned int *)(arg2+4));
	t1 += t3;
	p7 = t2;
	t4 = t1;
	t2 = t3;
	t4 = ~t4;
	t4 &= t5;
	t2 &= t1;
	t4 |= t2;
	t2 = p7;
	t4 += t2;
	t2 = *((unsigned int *)(arg1+12));
	t4 = t2 + t4 + 0xE8C7B756;
	t2 = t4;
	t2 >>= 0x14;
	t4 <<= 0x0C;
	t2 |= t4;
	t4 = *((unsigned int *)(arg2+8));
	t2 += t1;
	p13 = t4;
	t4 = t2;
	t4 = ~t4;
	t4 &= t3;
	t3 = t2;
	t3 &= t1;
	t4 |= t3;
	t3 = p13;
	t4 += t3;
	t4 = t5 + t4 + 0x242070DB;
	t3 = t4;
	t3 >>= 0x0F;
	t4 <<= 0x11;
	t3 |= t4;
	t4 = *((unsigned int *)(arg2+12));
	t3 += t2;
	p5 = t4;
	t5 = t3;
	t4 = t2;
	t5 = ~t5;
	t5 &= t1;
	t4 &= t3;
	t5 |= t4;
	t4 = p5;
	t5 += t4;
	t4 = *((unsigned int *)(arg1+4));
	t5 = t5 + t4 + 0xC1BDCEEE;
	t4 = t5;
	t4 <<= 0x16;
	t5 >>= 0x0A;
	t4 |= t5;
	t5 = *((unsigned int *)(arg2+16));
	t4 += t3;
	tt2 = t4;
	p11 = t5;
	t5 = t3;
	t5 &= tt2;
	t4 = ~t4;
	t4 &= t2;
	t4 |= t5;
	t5 = p11;
	t4 += t5;
	t5 = *((unsigned int *)(arg2+20));
	p3 = t5;
	t1 = t1 + t4 + 0xF57C0FAF;
	t4 = t1;
	t4 >>= 0x19;
	t1 <<= 0x7;
	t4 |= t1;
	t1 = tt2;
	t4 += t1;
	t5 = t4;
	t1 &= t4;
	t5 = ~t5;
	t5 &= t3;
	t5 |= t1;
	t1 = p3;
	t5 += t1;
	t2 = t5 + t2 + 0x4787C62A;
	t5 = tt2;
	t1 = t2;
	t1 >>= 0x14;
	t2 <<= 0x0C;
	t1 |= t2;
	t2 = *((unsigned int *)(arg2+24));
	t1 += t4;
	p9 = t2;
	t2 = t1;
	t2 = ~t2;
	t2 &= t5;
	t5 = t1;
	t5 &= t4;
	t2 |= t5;
	t5 = p9;
	t2 += t5;
	t5 = t1;
	t3 = t3 + t2 + 0xA8304613;
	t2 = t3;
	t2 >>= 0x0F;
	t3 <<= 0x11;
	t2 |= t3;
	t3 = *((unsigned int *)(arg2+28));
	t2 += t1;
	p1 = t3;
	t3 = t2;
	t5 &= t2;
	t3 = ~t3;
	t3 &= t4;
	t3 |= t5;
	t5 = p1;
	t3 += t5;
	t5 = tt2;
	t5 = t5 + t3 + 0xFD469501;
	t3 = t5;
	t3 <<= 0x16;
	t5 >>= 0x0A;
	t3 |= t5;
	t5 = *((unsigned int *)(arg2+32));
	t3 += t2;
	p8 = t5;
	tt2 = t3;
	t5 = t2;
	t5 &= tt2;
	t3 = ~t3;
	t3 &= t1;
	t3 |= t5;
	t5 = p8;
	t3 += t5;
	t4 = t4 + t3 + 0x698098D8;
	t3 = t4;
	t3 >>= 0x19;
	t4 <<= 0x7;
	t3 |= t4;
	t4 = tt2;
	t3 += t4;
	t5 = *((unsigned int *)(arg2+36));
	t4 &= t3;
	p14 = t5;
	t5 = t3;
	t5 = ~t5;
	t5 &= t2;
	t5 |= t4;
	t4 = p14;
	t5 += t4;
	t1 = t5 + t1 + 0x8B44F7AF;
	t5 = tt2;
	t4 = t1;
	t4 >>= 0x14;
	t1 <<= 0x0C;
	t4 |= t1;
	t1 = *((unsigned int *)(arg2+40));
	t4 += t3;
	p6 = t1;
	t1 = t4;
	t1 = ~t1;
	t1 &= t5;
	t5 = t4;
	t5 &= t3;
	t1 |= t5;
	t5 = p6;
	t1 += t5;
	t5 = t4;
	t2 = t2 + t1 + 0xFFFF5BB1;
	t1 = t2;
	t1 >>= 0x0F;
	t2 <<= 0x11;
	t1 |= t2;
	t2 = *((unsigned int *)(arg2+44));
	t1 += t4;
	p12 = t2;
	t2 = t1;
	t5 &= t1;
	t2 = ~t2;
	t2 &= t3;
	t2 |= t5;
	t5 = p12;
	t2 += t5;
	t5 = tt2;
	t5 = t5 + t2 + 0x895CD7BE;
	t2 = t5;
	t2 <<= 0x16;
	t5 >>= 0x0A;
	t2 |= t5;
	t5 = *((unsigned int *)(arg2+48));
	t2 += t1;
	p4 = t5;
	tt2 = t2;
	t5 = t1;
	t5 &= tt2;
	t2 = ~t2;
	t2 &= t4;
	t2 |= t5;
	t5 = p4;
	t2 += t5;
	t5 = *((unsigned int *)(arg2+52));
	p10 = t5;
	t3 = t3 + t2 + 0x6B901122;
	t2 = t3;
	t2 >>= 0x19;
	t3 <<= 0x7;
	t2 |= t3;
	t3 = tt2;
	t2 += t3;
	t5 = t2;
	t3 &= t2;
	t5 = ~t5;
	t5 &= t1;
	t5 |= t3;
	t3 = p10;
	t5 += t3;
	t4 = t5 + t4 + 0xFD987193;
	t3 = t4;
	t3 >>= 0x14;
	t4 <<= 0x0C;
	t3 |= t4;
	t3 += t2;
	t5 = *((unsigned int *)(arg2+56));
	t4 = t3;
	t4 = ~t4;
	p2 = t5;
	t5 = tt2;
	p16 = t4;
	t4 &= t5;
	t5 = t3;
	t6 = *((unsigned int *)(arg2+60));
	t5 &= t2;
	p15 = t6;
	t4 |= t5;
	t5 = p2;
	t4 += t5;
	t5 = t3;
	t1 = t1 + t4 + 0xA679438E;
	t4 = t1;
	t4 >>= 0x0F;
	t1 <<= 0x11;
	t4 |= t1;
	t4 += t3;
	t1 = t4;
	t5 &= t4;
	t1 = ~t1;
	p17 = t1;
	t1 &= t2;
	t1 |= t5;
	t5 = t3;
	t1 += t6;
	t6 = tt2;
	t6 = t6 + t1 + 0x49B40821;
	t1 = t6;
	t1 <<= 0x16;
	t6 >>= 0x0A;
	t1 |= t6;
	t6 = p16;
	t1 += t4;
	t6 &= t4;
	t5 &= t1;
	t6 |= t5;
	t5 = p7;
	t6 += t5;
	t5 = t4;
	t6 = t2 + t6 + 0xF61E2562;
	t2 = t6;
	t2 >>= 0x1B;
	t6 <<= 0x5;
	t2 |= t6;
	t6 = p17;
	t2 += t1;
	t6 &= t1;
	t5 &= t2;
	t6 |= t5;
	t5 = p9;
	t6 += t5;
	t6 = t3 + t6 + 0xC040B340;
	t3 = t6;
	t3 >>= 0x17;
	t6 <<= 0x9;
	t3 |= t6;
	t6 = t1;
	t3 += t2;
	t6 = ~t6;
	t5 = t3;
	t6 &= t2;
	t5 &= t1;
	t6 |= t5;
	t5 = p12;
	t6 += t5;
	t6 = t4 + t6 + 0x265E5A51;
	t4 = t6;
	t4 >>= 0x12;
	t6 <<= 0x0E;
	t4 |= t6;
	t6 = t2;
	t4 += t3;
	t6 = ~t6;
	t5 = t4;
	t6 &= t3;
	t5 &= t2;
	t6 |= t5;
	t5 = tt1;
	t6 += t5;
	t5 = t3;
	t6 = t1 + t6 + 0xE9B6C7AA;
	t1 = t6;
	t6 >>= 0x0C;
	t1 <<= 0x14;
	t1 |= t6;
	t6 = t3;
	t6 = ~t6;
	t1 += t4;
	t6 &= t4;
	t5 &= t1;
	t6 |= t5;
	t5 = p3;
	t6 += t5;
	t5 = t4;
	t6 = t2 + t6 + 0xD62F105D;
	t2 = t6;
	t6 <<= 0x5;
	t2 >>= 0x1B;
	t2 |= t6;
	t6 = t4;
	t2 += t1;
	t6 = ~t6;
	t6 &= t1;
	t5 &= t2;
	t6 |= t5;
	t5 = p6;
	t6 += t5;
	t6 = t3 + t6 + 0x2441453;
	t3 = t6;
	t3 >>= 0x17;
	t6 <<= 0x9;
	t3 |= t6;
	t6 = t1;
	t3 += t2;
	t6 = ~t6;
	t5 = t3;
	t6 &= t2;
	t5 &= t1;
	t6 |= t5;
	t5 = p15;
	t6 += t5;
	t6 = t4 + t6 + 0xD8A1E681;
	t4 = t6;
	t4 >>= 0x12;
	t6 <<= 0x0E;
	t4 |= t6;
	t6 = t2;
	t4 += t3;
	t6 = ~t6;
	t5 = t4;
	t6 &= t3;
	t5 &= t2;
	t6 |= t5;
	t5 = p11;
	t6 += t5;
	t5 = t3;
	t6 = t1 + t6 + 0xE7D3FBC8;
	t1 = t6;
	t1 <<= 0x14;
	t6 >>= 0x0C;
	t1 |= t6;
	t6 = t3;
	t1 += t4;
	t6 = ~t6;
	t6 &= t4;
	t5 &= t1;
	t6 |= t5;
	t5 = p14;
	t6 += t5;
	t6 = t2 + t6 + 0x21E1CDE6;
	t2 = t6;
	t2 >>= 0x1B;
	t6 <<= 0x5;
	t2 |= t6;
	t6 = t4;
	t6 = ~t6;
	t2 += t1;
	t6 &= t1;
	t5 = t4;
	t5 &= t2;
	t6 |= t5;
	t5 = p2;
	t6 += t5;
	t6 = t3 + t6 + 0xC33707D6;
	t3 = t6;
	t6 <<= 0x9;
	t3 >>= 0x17;
	t3 |= t6;
	t6 = t1;
	t3 += t2;
	t6 = ~t6;
	t5 = t3;
	t6 &= t2;
	t5 &= t1;
	t6 |= t5;
	t5 = p5;
	t6 += t5;
	t6 = t4 + t6 + 0xF4D50D87;
	t4 = t6;
	t4 >>= 0x12;
	t6 <<= 0x0E;
	t4 |= t6;
	t6 = t2;
	t4 += t3;
	t6 = ~t6;
	t5 = t4;
	t6 &= t3;
	t5 &= t2;
	t6 |= t5;
	t5 = p8;
	t6 += t5;
	t5 = t3;
	t6 = t1 + t6 + 0x455A14ED;
	t1 = t6;
	t1 <<= 0x14;
	t6 >>= 0x0C;
	t1 |= t6;
	t6 = t3;
	t1 += t4;
	t6 = ~t6;
	t6 &= t4;
	t5 &= t1;
	t6 |= t5;
	t5 = p10;
	t6 += t5;
	t5 = t4;
	t6 = t2 + t6 + 0xA9E3E905;
	t2 = t6;
	t2 >>= 0x1B;
	t6 <<= 0x5;
	t2 |= t6;
	t6 = t4;
	t2 += t1;
	t6 = ~t6;
	t6 &= t1;
	t5 &= t2;
	t6 |= t5;
	t5 = p13;
	t6 += t5;
	t6 = t3 + t6 + 0xFCEFA3F8;
	t3 = t6;
	t3 >>= 0x17;
	t6 <<= 0x9;
	t3 |= t6;
	t6 = t1;
	t3 += t2;
	t6 = ~t6;
	t5 = t3;
	t6 &= t2;
	t5 &= t1;
	t6 |= t5;
	t5 = p1;
	t6 += t5;
	t6 = t4 + t6 + 0x676F02D9;
	t4 = t6;
	t4 >>= 0x12;
	t6 <<= 0x0E;
	t4 |= t6;
	t4 += t3;
	t6 = t2;
	t5 = t4;
	t6 = ~t6;
	t6 &= t3;
	t5 &= t2;
	t6 |= t5;
	t5 = p4;
	t6 += t5;
	t5 = p3;
	t6 = t1 + t6 + 0x8D2A4C8A;
	t1 = t6;
	t6 >>= 0x0C;
	t1 <<= 0x14;
	t1 |= t6;
	t6 = t3;
	t6 ^= t4;
	t1 += t4;
	t6 ^= t1;
	t6 += t5;
	t5 = p8;
	t6 = t2 + t6 + 0xFFFA3942;
	t2 = t6;
	t6 <<= 0x4;
	t2 >>= 0x1C;
	t2 |= t6;
	t6 = t4;
	t6 ^= t1;
	t2 += t1;
	t6 ^= t2;
	t6 += t5;
	t5 = p12;
	t6 = t3 + t6 + 0x8771F681;
	t3 = t6;
	t3 >>= 0x15;
	t6 <<= 0x0B;
	t3 |= t6;
	t3 += t2;
	t6 = t3;
	t6 ^= t1;
	t6 ^= t2;
	t6 += t5;
	t5 = p2;
	t6 = t4 + t6 + 0x6D9D6122;
	t4 = t6;
	t4 >>= 0x10;
	t6 <<= 0x10;
	t4 |= t6;
	t6 = t3;
	t4 += t3;
	t6 ^= t4;
	tt2 = t6;
	t6 ^= t2;
	t6 += t5;
	t5 = p7;
	t6 = t1 + t6 + 0xFDE5380C;
	t1 = t6;
	t1 <<= 0x17;
	t6 >>= 0x9;
	t1 |= t6;
	t6 = tt2;
	t1 += t4;
	t6 ^= t1;
	t6 += t5;
	t5 = p11;
	t6 = t2 + t6 + 0xA4BEEA44;
	t2 = t6;
	t2 >>= 0x1C;
	t6 <<= 0x4;
	t2 |= t6;
	t6 = t4;
	t2 += t1;
	t6 ^= t1;
	t6 ^= t2;
	t6 += t5;
	t6 = t3 + t6 + 0x4BDECFA9;
	t3 = t6;
	t3 >>= 0x15;
	t6 <<= 0x0B;
	t3 |= t6;
	t3 += t2;
	t6 = t3;
	t5 = p1;
	t6 ^= t1;
	t6 ^= t2;
	t6 += t5;
	t5 = p6;
	t6 = t4 + t6 + 0xF6BB4B60;
	t4 = t6;
	t6 <<= 0x10;
	t4 >>= 0x10;
	t4 |= t6;
	t6 = t3;
	t4 += t3;
	t6 ^= t4;
	tt2 = t6;
	t6 ^= t2;
	t6 += t5;
	t5 = p10;
	t6 = t1 + t6 + 0xBEBFBC70;
	t1 = t6;
	t6 >>= 0x9;
	t1 <<= 0x17;
	t1 |= t6;
	t6 = tt2;
	t1 += t4;
	t6 ^= t1;
	t6 += t5;
	t5 = tt1;
	t6 = t2 + t6 + 0x289B7EC6;
	t2 = t6;
	t2 >>= 0x1C;
	t6 <<= 0x4;
	t2 |= t6;
	t6 = t4;
	t2 += t1;
	t6 ^= t1;
	t6 ^= t2;
	t6 += t5;
	t5 = p5;
	t6 = t3 + t6 + 0xEAA127FA;
	t3 = t6;
	t3 >>= 0x15;
	t6 <<= 0x0B;
	t3 |= t6;
	t3 += t2;
	t6 = t3;
	t6 ^= t1;
	t6 ^= t2;
	t6 += t5;
	t5 = p9;
	t6 = t4 + t6 + 0xD4EF3085;
	t4 = t6;
	t4 >>= 0x10;
	t6 <<= 0x10;
	t4 |= t6;
	t6 = t3;
	t4 += t3;
	t6 ^= t4;
	tt2 = t6;
	t6 ^= t2;
	t6 += t5;
	t5 = p14;
	t6 = t1 + t6 + 0x4881D05;
	t1 = t6;
	t1 <<= 0x17;
	t6 >>= 0x9;
	t1 |= t6;
	t6 = tt2;
	t1 += t4;
	t6 ^= t1;
	t6 += t5;
	t5 = p4;
	t6 = t2 + t6 + 0xD9D4D039;
	t2 = t6;
	t2 >>= 0x1C;
	t6 <<= 0x4;
	t2 |= t6;
	t6 = t4;
	t2 += t1;
	t6 ^= t1;
	t6 ^= t2;
	t6 += t5;
	t6 = t3 + t6 + 0xE6DB99E5;
	t5 = p15;
	t3 = t6;
	t6 <<= 0x0B;
	t3 >>= 0x15;
	t3 |= t6;
	t3 += t2;
	t6 = t3;
	t6 ^= t1;
	t6 ^= t2;
	t6 += t5;
	t5 = p13;
	t6 = t4 + t6 + 0x1FA27CF8;
	t4 = t6;
	t6 <<= 0x10;
	t4 >>= 0x10;
	t4 |= t6;
	t6 = t3;
	t4 += t3;
	t6 ^= t4;
	t6 ^= t2;
	t6 += t5;
	t5 = tt1;
	t6 = t1 + t6 + 0xC4AC5665;
	t1 = t6;
	t6 >>= 0x9;
	t1 <<= 0x17;
	t1 |= t6;
	t6 = t3;
	t1 += t4;
	t6 = ~t6;
	t6 |= t1;
	t6 ^= t4;
	t6 += t5;
	t5 = p1;
	t6 = t2 + t6 + 0xF4292244;
	t2 = t6;
	t2 >>= 0x1A;
	t6 <<= 0x6;
	t2 |= t6;
	t6 = t4;
	t2 += t1;
	t6 = ~t6;
	t6 |= t2;
	t6 ^= t1;
	t6 += t5;
	t5 = p2;
	t6 = t3 + t6 + 0x432AFF97;
	t3 = t6;
	t3 >>= 0x16;
	t6 <<= 0x0A;
	t3 |= t6;
	t6 = t1;
	t3 += t2;
	t6 = ~t6;
	t6 |= t3;
	t6 ^= t2;
	t6 += t5;
	t5 = p3;
	t6 = t4 + t6 + 0xAB9423A7;
	t4 = t6;
	t4 >>= 0x11;
	t6 <<= 0x0F;
	t4 |= t6;
	t6 = t2;
	t4 += t3;
	t6 = ~t6;
	t6 |= t4;
	t6 ^= t3;
	t6 += t5;
	t5 = p4;
	t6 = t1 + t6 + 0xFC93A039;
	t1 = t6;
	t1 <<= 0x15;
	t6 >>= 0x0B;
	t1 |= t6;
	t6 = t3;
	t1 += t4;
	t6 = ~t6;
	t6 |= t1;
	t6 ^= t4;
	t6 += t5;
	t5 = p5;
	t6 = t2 + t6 + 0x655B59C3;
	t2 = t6;
	t6 <<= 0x6;
	t2 >>= 0x1A;
	t2 |= t6;
	t6 = t4;
	t6 = ~t6;
	t2 += t1;
	t6 |= t2;
	t6 ^= t1;
	t6 += t5;
	t5 = p6;
	t6 = t3 + t6 + 0x8F0CCC92;
	t3 = t6;
	t6 <<= 0x0A;
	t3 >>= 0x16;
	t3 |= t6;
	t6 = t1;
	t6 = ~t6;
	t3 += t2;
	t6 |= t3;
	t6 ^= t2;
	t6 += t5;
	t5 = p7;
	t6 = t4 + t6 + 0xFFEFF47D;
	t4 = t6;
	t4 >>= 0x11;
	t6 <<= 0x0F;
	t4 |= t6;
	t6 = t2;
	t4 += t3;
	t6 = ~t6;
	t6 |= t4;
	t6 ^= t3;
	t6 += t5;
	t5 = p8;
	t6 = t1 + t6 + 0x85845DD1;
	t1 = t6;
	t1 <<= 0x15;
	t6 >>= 0x0B;
	t1 |= t6;
	t6 = t3;
	t1 += t4;
	t6 = ~t6;
	t6 |= t1;
	t6 ^= t4;
	t6 += t5;
	t5 = p15;
	t6 = t2 + t6 + 0x6FA87E4F;
	t2 = t6;
	t2 >>= 0x1A;
	t6 <<= 0x6;
	t2 |= t6;
	t6 = t4;
	t2 += t1;
	t6 = ~t6;
	t6 |= t2;
	t6 ^= t1;
	t6 += t5;
	t5 = p9;
	t6 = t3 + t6 + 0xFE2CE6E0;
	t3 = t6;
	t3 >>= 0x16;
	t6 <<= 0x0A;
	t3 |= t6;
	t6 = t1;
	t3 += t2;
	t6 = ~t6;
	t6 |= t3;
	t6 ^= t2;
	t6 += t5;
	t4 = t4 + t6 + 0xA3014314;
	t6 = t4;
	t6 >>= 0x11;
	t4 <<= 0x0F;
	t6 |= t4;
	t4 = t2;
	t6 += t3;
	t4 = ~t4;
	t5 = p10;
	t4 |= t6;
	t4 ^= t3;
	t4 += t5;
	t5 = p11;
	t4 = t1 + t4 + 0x4E0811A1;
	t1 = t4;
	t1 <<= 0x15;
	t4 >>= 0x0B;
	t1 |= t4;
	t4 = t3;
	t1 += t6;
	t4 = ~t4;
	t4 |= t1;
	t4 ^= t6;
	t4 += t5;
	t5 = p12;
	t4 = t2 + t4 + 0xF7537E82;
	t2 = t4;
	t2 >>= 0x1A;
	t4 <<= 0x6;
	t2 |= t4;
	t4 = t6;
	t2 += t1;
	t4 = ~t4;
	t4 |= t2;
	t4 ^= t1;
	t4 += t5;
	t5 = p13;
	t4 = t3 + t4 + 0xBD3AF235;
	t3 = t4;
	t3 >>= 0x16;
	t4 <<= 0x0A;
	t3 |= t4;
	t4 = t1;
	t3 += t2;
	t4 = ~t4;
	t4 |= t3;
	t4 ^= t2;
	t4 += t5;
	t5 = p14;
	t6 = t6 + t4 + 0x2AD7D2BB;
	t4 = t6;
	t4 >>= 0x11;
	t6 <<= 0x0F;
	t4 |= t6;
	t6 = t2;
	t4 += t3;
	t6 = ~t6;
	t6 |= t4;
	t6 ^= t3;
	t6 += t5;
	t1 = t1 + t6 + 0xEB86D391;
	t6 = *((unsigned int *)arg1);
	t6 += t2;
	t2 = t1;
	t2 <<= 0x15;
	t1 >>= 0x0B;
	t2 |= t1;
	t1 = *((unsigned int *)(arg1+8));
	*((unsigned int *)arg1) = t6;
	t6 = *((unsigned int *)(arg1+4));
	t1 += t4;
	t2 += t6;
	*((unsigned int *)(arg1+8)) = t1;
	t1 = *((unsigned int *)(arg1+12));
	t2 += t4;
	t1 += t3;
	*((unsigned int *)(arg1+4)) = t2;
	*((unsigned int *)(arg1+12)) = t1;
	return;
}

void sub_4021E1(char *arg1, char *arg2, int arg3)
{
	char p[64];
	unsigned int t1;
	unsigned int t2;
	unsigned int t3;
	unsigned int t4;
	unsigned char ch1;
	unsigned char ch2;

	t1 = arg3;
	if(t1 != 0)
	{
		goto loc_4021FD;
	}
	t2 = *((unsigned int *)(arg1+24));
	if(t2 != 0)
	{
		goto loc_40232F;
	}

loc_4021FD:
	t2 = *((unsigned int *)(arg1+24));
	if(t2 == 0)
	{
		goto loc_402212;
	}
	return;

loc_402212:
	t2 = t1;
	t4 = 16;
	if(t1 == 0)
	{
		goto loc_402229;
	}

loc_40221B:
	t3 = *(arg1+t4);
	t2 += t3;
	*(arg1+t4) = t2;
	t4++;
	t2 >>= 8;
	if(t2>0)
	{
		goto loc_40221B;
	}

loc_402229:
	if(t1 != 512)
	{
		goto loc_402244;
	}
	sub_402335(arg1, arg2);
	return;

loc_402244:
	if(t1 <= 512)
	{
		goto loc_402255;
	}
	return;

loc_402255:
	t3 = t1>>3;
	strcpy(p, arg2);
	memset(p+strlen(arg2), 0, sizeof(p)-strlen(arg2));

loc_40229C:
	t2 = 7;
	t1 = 1;
	t1 <<= t2;
	ch1 = t1 & 0xff;
	ch2 = t1 & 0xff;
	ch1 |= p[t3];
	ch2--;
	ch2 = ~ch2;
	ch2 &= ch1;
	p[t3] = ch2;
	if(t3 > 0x37)
	{
		goto loc_4022F0;
	}
	t3 = *((unsigned int *)(arg1+16));
	t1 = *((unsigned int *)(arg1+20));
	*((unsigned int *)(p+56)) = t3;
	*((unsigned int *)(p+60)) = t1;
	sub_402335(arg1, p);
	*((unsigned int *)(arg1+24)) = 1;
	return;

loc_4022F0:
	sub_402335(arg1, p);
	memset(p, 0, sizeof(p));
	t1 = *((unsigned int *)(arg1+16));
	t2 = *((unsigned int *)(arg1+20));
	*((unsigned int *)(p+56)) = t1;
	*((unsigned int *)(p+60)) = t2;
	sub_402335(arg1, p);
	*((unsigned int *)(arg1+24)) = 1;

loc_40232F:
	return;
}

void sub_402112(char *arg1, int arg2, char *arg3)
{
	unsigned char ckey[28] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
					 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
					 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					 0x00, 0x00, 0x00, 0x00};
	char ctmp[1024] = { 0 };
	unsigned int t1;
	unsigned int t2;
	unsigned int t3;
	unsigned int t4;
	unsigned int t5;
	unsigned int t6;
	char *p;

	strcpy(ctmp, arg1);
	p = ctmp;
	t5 = arg2;
	if(arg2 < 64)
	{
		goto loc_402171;
	}
	t6 = t5;
	t6 >>= 6;
	t3 = t6;
	t3 = (~t3) + 1;
	t3 <<= 6;
	t5 += t3;


loc_402158:
	sub_4021E1((char *)ckey, p, 512);
	p += 64;
	t6--;
	if(t6 != 0)
	{
		goto loc_402158;
	}

loc_402171:
	sub_4021E1((char *)ckey, p, t5*8);
	t1 = 0;
	t4 = 0;
	t5 = 4;

loc_402194:
	t6 = *((unsigned int *)(ckey+t4));
	t2 = 0;

loc_402198:
	t3 = t6;
	t3 >>= (t2&0xff);
	t2 += 8;
	t1++;
	arg3[t1-1] = (t3&0xff);
	if(t2 < 0x20)
	{
		goto loc_402198;
	}
	t4 += 4;
	t5--;
	if(t5 != 0)
	{
		goto loc_402194;
	}

	return;
}

void sub_402000(char *arg1, char *arg2)
{
	char p[56];
	char tmpstr[16];
	int i;

	sub_402112(arg1, strlen(arg1), p);
	arg2[0] = 0;
	for(i=0; i<16; i++)
	{
		sprintf(tmpstr, "%02x", (unsigned char)p[i]);
		strcat(arg2, tmpstr);
	}
	return;
}

void xian_pppoe_user(char *user, char *passwd, char *result)
{
	char tmpstr1[256];
	char tmpstr2[256];

	memset(tmpstr1, 0, sizeof(tmpstr1));
	memset(tmpstr2, 0, sizeof(tmpstr2));
	strcpy(tmpstr1, user);
	strcat(tmpstr1, user);
	sub_402000(tmpstr1, tmpstr2);
	strcpy(result, passwd);
	strcat(result, tmpstr2);
	result[strlen(passwd)+15] = 0;
	return;

}


void henan_pppoe_user(char *user, char *result)
{

	char * temp;

	unsigned int count;
	unsigned int edx;
	unsigned int ecx;

	int i;
	unsigned int eax;
	char var_1DC[128] = {0};
	char buffer[128] = {0};
       unsigned  int var_DC[16]={0x11,0x34,0xC9,0x23,0x75,0x18,0xD7,0xE2,0x12,0x35,0x29,0x2B,0xEC,0xB6,0x23,0x19};
 
	char * var_9C= 0 ;
	unsigned int *var_98 = 0;
	char * var_94=0;
	char * var_90=0;
	unsigned int c;
	unsigned int *var_8C =0;
	char * var_88 = 0;
	unsigned int number = 0;
	unsigned int var_80 = 0;
	unsigned int var_7C = 0;
	unsigned int var_78 = 0;
	char s[64] = {0};
	unsigned int var_34 = 0;
	unsigned int var_24 = 0;
	unsigned int var_18 = 0;
	unsigned int var_10 = 0;
	unsigned int var_C = 0;
	unsigned int var_8 = 0;
	unsigned int var_4 = 0;
	unsigned int iTemp ;
	var_18 = 1;
	var_18++;
	var_24 = 8;
	strcpy(s, "9012345678abcdeABCDEFGHIJKLMNfghijklmnUVWXYZxyzuvwopqrstOPQRST");
	var_78 = 0x25;

loc_401C9C:
	sprintf(buffer, "%s", user);

	var_7C=0;
	count = 0;
	var_9C=buffer;
	var_98= (unsigned int *) var_DC;
	var_94= var_1DC;
	goto loc_401DE6;

loc_401CF3:
	var_24 = 8;
	i = 0;
	temp = s;
	var_90=var_94;
	var_8C=var_98;
	var_88=var_9C;
	goto loc_401DA1;

loc_401D2A:

	if(*var_88 != *temp)
		goto loc_401D9F;

	if(count >= 0x10)
		goto loc_401D45;

    c= *(var_8C);
	goto loc_401D61;

loc_401D45:
	number = count/0x10;
	var_80 = count%0x10;
	c =  var_DC[var_80];

loc_401D61:

	iTemp = var_78;
	iTemp += iTemp;
	iTemp += var_78;
	 c ^= iTemp;
	 c ^= var_7C;
	 c += i;
	number = c/0x3e;
	var_80 = c%0x3e;
	*var_90 = s[var_80];
	var_80 +=0x24d9;
	var_78 ^=var_80;
	goto loc_401DB3;

loc_401D9F:
	i++;
	temp++;

loc_401DA1:
	if(strlen(s)>i)
		goto loc_401D2A;

loc_401DB3:
	if(*var_94 !=0)
	goto loc_401DCE;
	*(var_94) = *(var_9C);

loc_401DCE:
	var_7C += 5;
	count++;
	var_9C++;
	var_98++;
	var_94++;

loc_401DE6:
	if(strlen(buffer)>count)
	goto      loc_401CF3;

	strcpy(result, "2:");
	strcat(result, var_1DC);
	return;
}

void henan_pppoe_user_shanxi(char *user, char *result)
{

	char * temp;

	unsigned int count;
	unsigned int edx;
	unsigned int ecx;

	int i;
	unsigned int eax;
	char var_1DC[256] = {0};
	char buffer[256] = {0};
    unsigned  int var_DC[16]={0x11,0x34,0xC9,0x23,0x75,0x18,0xD7,0xE2,0x12,0x35,0x29,0x2B,0xEC,0xB6,0x23,0x19};

	char * var_9C= 0 ;
	unsigned int *var_98 = 0;
	char * var_94=0;
	char * var_90=0;
	unsigned int c;
	unsigned int *var_8C =0;
	char * var_88 = 0;
	unsigned int number = 0;
	unsigned int var_80 = 0;
	unsigned int var_7C = 0;
	unsigned int var_78 = 0;
	char s[64] = {0};
	unsigned int var_34 = 0;
	unsigned int var_24 = 0;
	unsigned int var_18 = 0;
	unsigned int var_10 = 0;
	unsigned int var_C = 0;
	unsigned int var_8 = 0;
	unsigned int var_4 = 0;
	unsigned int iTemp ;
	var_18 = 1;
	var_18++;
	var_24 = 8;
	strcpy(s, "9012345678abcdeABCDEFGHIJKLMNfghijklmnUVWXYZxyzuvwopqrstOPQRST");
	var_78 = 0x25;

loc_401C9C:
	sprintf(buffer, "%s", user);

	var_7C=0;
	count = 0;
	var_9C=buffer;
	var_98= (unsigned int *) var_DC;
	var_94= var_1DC;
	goto loc_401DE6;

loc_401CF3:
	var_24 = 8;
	i = 0;
	temp = s;
	var_90=var_94;
	var_8C=var_98;
	var_88=var_9C;
	goto loc_401DA1;

loc_401D2A:

	if(*var_88 != *temp)
		goto loc_401D9F;

	if(count >= 0x10)
		goto loc_401D45;

    c= *(var_8C);
	goto loc_401D61;

loc_401D45:
	number = count/0x10;
	var_80 = count%0x10;
	c =  var_DC[var_80];

loc_401D61:

	iTemp = var_78;
	iTemp += iTemp;
	iTemp += var_78;
	 c ^= iTemp;
	 c ^= var_7C;
	 c += i;
	number = c/0x3e;
	var_80 = c%0x3e;
	*var_90 = s[var_80];
	var_80 +=0x24d9;
	var_78 ^=var_80;
	goto loc_401DB3;

loc_401D9F:
	i++;
	temp++;

loc_401DA1:
	if(strlen(s)>i)
		goto loc_401D2A;

loc_401DB3:
	if(*var_94 !=0)
	goto loc_401DCE;
	*(var_94) = *(var_9C);

loc_401DCE:
	var_7C += 5;
	count++;
	var_9C++;
	var_98++;
	var_94++;

loc_401DE6:
	if(strlen(buffer)>count)
	goto      loc_401CF3;

	strcpy(result, "1:");
	strcat(result, var_1DC);
	return;
}


void xkjs_ver31(char* arg_0)
{
    char var_CC[0xcc];
    int var_D0;
    int var_D4;

    unsigned int v1;
    unsigned int v2;
    unsigned int v3;
    unsigned int v4;

    unsigned int v5;
    unsigned int v6;

    unsigned int v7;
    unsigned int vt;

    char* strArg = "NJ3r05t949R9jdkdfo4lDLR2Evzl35Rkdl1tggtjofdKRIOkLH888iJkyUkjNNbVvjU84410Keloekri78DJ490I574RjK96HjJt7676554r5tgjhHhBGY78668754631HIUHUggGgyGFY78684Ffhyj6JJBN464335dfDDXZccblpoppytrdrdfGFtrgjii87pdl545";

    strcpy(var_CC, strArg);
    v3 = 201;
    v3 -= 1;

    v5 = arg_0;
    v4 ^= v4;
    v6 = v5;
    var_D0 = v3;
    v3 = strlen(arg_0);
    if(v3 <= 0)
    {
         goto loc_10002DD1;
    }

loc_10002DC6:
    if(*((char *)(v4+v5)) == 0x40)
    {
     	goto loc_10002DD1;
    }
    v4 += 1;
    if(v4 < v3)
    {
      	goto loc_10002DC6;
    }

loc_10002DD1:
    v1 ^= v1;
    v6 ^= v6;
    if(v4 <= 0)
    {
      	goto loc_10002DF1;
    }

loc_10002DDB:
    v2 = *(char *)(v6+v5);
    v7 = v1;
    v7 <<= 5;
    v7 += v1;
    v2 += v7;
    v6 += 1;
    v1 = v2;
    if(v6 < v4)
    {
      	goto loc_10002DDB;
    }

loc_10002DF1:
    v4 ^= v4;
    vt = v1;
    v1 = vt / var_D0;
    v4 = vt % var_D0;
    v3 += 1;
    if( v3 <= 0x1 )
    {
       	goto loc_10002E0A;
    }

loc_10002DFD:
    *((char *)(v3+v5)) = *((char *)(v3+v5-2));
    v3 -= 1;
    if(v3 > 0x1)
    {
       	goto loc_10002DFD;
    }

loc_10002E0A:
    *((char *)(v5+1)) = var_CC[v4];
    v4 = *((unsigned int *)(var_CC));
    *((char *)v5) = 0x7e;
    return;
}

void xkjs_ver32(char* pwd)
{
	unsigned int v1;
	unsigned int v3;
	unsigned int v4;

	unsigned int v5;
	unsigned int v6;

	int i = 0, j = 0;
	char temp[3]={0};
	char arg_10[256];
	char var_14[8];
	var_14[0] = 0xD2;
	var_14[1] = 0xA6;
	var_14[2] = 0xE0;
	var_14[3] = 0x9C;
	var_14[4] = 0xAD;
	var_14[5] = 0x93;
	var_14[6] = 0x86;
	var_14[7] = 0xBF;
	memset(arg_10, 0x00, 128);
	strcpy(arg_10, pwd);
	v5 = 0;
	v6 = strlen(arg_10);

loc_1000501A:
	v3 = arg_10;
	v4 = v5;
	v4 &= 0x80000007;
	v1  = *((char *)(v5 + v3));
	if((v4 & 0x80000000)==0)
		goto loc_10005030;
	v4 -= 1;
	v4 |= 0xFFFFFFF8;
	v4 += 1;

loc_10005030:
	v3 = var_14[v4];
	v3 ^= v1;
	arg_10[v5] = v3;
	v5 += 1;
	if( v5 < v6 )
		goto loc_1000501A;

	for(i; i < v6; i++)
	{
		sprintf(temp, "%02X", (unsigned char)arg_10[i]);
		pwd[j++] = temp[0];
		pwd[j++] = temp[1];
	}

	return;
}


void sub_41B4A0(char* arg_0, char* arg_4, int arg_8)
{
	int v1;
	int v2;
	int v3;
	int v4;
	int v5;
	int v6;
	int v8;

	v1 = arg_8;
	if( (v1 & v1) == 0 )
		goto loc_41B522;

	v5 = arg_0;
	v1 = arg_4;
	v3 = *(char*)v5 & 0xff;
	v3 >>= 2;
	*(char*)v1 = v3;
	v3 = *(char*)v5 & 0xff;
	v3 &= 3;
	v3 <<= 4;
	*(char*)(v1+1) = v3;
	v4 = *(char*)(v5+1)  & 0xff;
	v4 >>= 4;
	v4 |= v3;
	*(char*)(v1+1) = v4;
	v3 = *(char*)(v5+1)  & 0xff;
	v3 &= 0x0F;
	v3 <<= 2;
	*(char*)(v1+2) = v3;
	v4 = *(char*)(v5+2)  & 0xff;
	v4 >>= 6;
	v4 |= v3;
	*(char*)(v1+2) = v4;
	v3 = *(char*)(v5+2)  & 0xff;
	v3 &= 0x3F;
	*(char*)(v1+3) = v3;
	v4 = *(char*)(v5+3)  & 0xff;
	v4 >>= 2;
	*(char*)(v1+4) = v4;
	v3 = *(char*)(v5+3)  & 0xff;
	v3 &= 3;
	v3 <<= 4;
	*(char*)(v1+5) = v3;
	v3 = 0;

loc_41B505:
	v4 = *(char*)(v3+v1)  & 0xff;
	v4 += 0x20;
	*(char*)(v3+v1) = v4;
	if(v4 < 0x40)
		goto loc_41B518;
	v4 += 1;
	*(char*)(v3+v1) = v4;

loc_41B518:
	v3 += 1;
	if(v3 < 6)
		goto loc_41B505;

	return 0x0C;

loc_41B522:
	v1 = arg_4;
	v3 = 0;

loc_41B528:
	v4 = *(char*)(v3+v1)  & 0xff;
	if(v4 <= 0x40)
		goto loc_41B535;

	v4 -= 1;
	*(char*)(v3+v1) = v4;

loc_41B535:
	v4 = *(char*)(v3+v1)  & 0xff;
	v4 += 0x0E0;
	*(char*)(v3+v1) = v4;
	v3 += 1;
	if(v3 < 6)
		goto loc_41B528;

	v3 = *(char*)v1  & 0xff;
	v5 = arg_0;
	v3 <<= 2;
	*(char*)v5 = v3;
	v4 = *(char*)(v1+1)  & 0xff;
	v4 >>= 4;
	v4 |= v3;
	*(char*)v5 = v4;
	v3 = *(char*)(v1+1)  & 0xff;
	v3 <<= 4;
	*(char*)(v5+1) = v3;
	v4 = *(char*)(v1+2)  & 0xff;
	v4 >>= 2;
	v4 &= 0x0F;
	v4 |= v3;
	*(char*)(v5+1) = v4;
	v3 = *(char*)(v1+2)  & 0xff;
	v3 <<= 6;
	*(char*)(v5+2) = v3;
	v4 = *(char*)(v1+3)  & 0xff;
	v4 &= 0x3F;
	v4 |= v3;
	*(char*)(v5+2) = v4;
	v3 = *(char*)(v1+4)  & 0xff;
	v3 <<= 2;
	*(char*)(v5+3) = v3;
	v1 = *(char*)(v1+5)  & 0xff;
	v1 >>= 4;
	v1 &= 3;
	v1 |= v3;
	*(char*)(v5+3) = v1;
	return 0x0C;
}


void sub_41AE80(char* arg_0)
{
	int var_4;
	int v1;
	int v2;
	int v3;
	int v4;
	int v5;
	int v6;
	int v8;
	char arg_4[200] = {0};

	v2 = arg_0;
	v3 = 0;
	v1 = *(int*)v2;
	v4 = v1 & 0xffff;
	var_4 = v1;
	*(int*)arg_0 = v3;
	v5 = 0;
	v1 = 3;

loc_41AE9C:
	v8 = (char*)&var_4+v1;
	v6 = arg_0+v3;
	v1 = *(char*)v8 & 0xff;
	v3 = *(char*)v6 & 0xff;
	v4 = v1;
	v3 <<= 1;
	v4 &= 1;
	v3 |= v4;
	v1 >>= 1;
	v5 += 1;
	*(char*)v8 = v1;
	v1 = v5;
	*(char*)v6 = v3;
	v4 = 0;						// .text:0041AEBC                 cdq
	v4 &= 7;
	v3 = 3;
	v1 += v4;
	v4 = v5;
	v1 >>= 3;
	v3 -= v1;
	v4 &= 0x80000003;
	v1 = v3;
	if((v4 & 0x08) == 0)
		goto loc_41AEDD;
	v4 -= 1;
	v4 |= 0x0FFFFFFFC;
	v4 += 1;

loc_41AEDD:
	v3 = v4;
	if(v5 < 0x20)
		goto loc_41AE9C;

	sub_41B4A0(arg_0, arg_4, 1);

	memcpy(arg_0 , arg_4, 8);
	return 4;
}

// username = 0E872256 + 07918117103 + radius
// arg_0 = 0123456789abcdeffedcba987654321016000000+username
// arg_4 = username
void sub_410970(char* arg_0, unsigned int arg_4)
{
	unsigned char v_cc[] =
		{0x01, 0x06, 0x0B, 0x00, 0x05, 0x0A, 0x0F, 0x04, 0x09, 0x0E, 0x03, 0x08, 0x0D, 0x02, 0x07, 0x0C
		 , 0x05, 0x08, 0x0B, 0x0E, 0x01, 0x04, 0x07, 0x0A, 0x0D, 0x00, 0x03, 0x06, 0x09, 0x0C, 0x0F, 0x02
		 , 0x00, 0x07, 0x0E, 0x05, 0x0C, 0x03, 0x0A, 0x01, 0x08, 0x0F, 0x06, 0x0D, 0x04, 0x0B, 0x02, 0x09
		 , 0x07, 0x0C, 0x11, 0x16, 0x05, 0x09, 0x0E, 0x14, 0x04, 0x0B, 0x10, 0x17, 0x06, 0x0A, 0x0F, 0x15
		 , 0x78, 0xA4, 0x6A, 0xD7, 0x56, 0xB7, 0xC7, 0xE8, 0xDB, 0x70, 0x20, 0x24, 0xEE, 0xCE, 0xBD, 0xC1
		 , 0xAF, 0x0F, 0x7C, 0xF5, 0x2A, 0xC6, 0x87, 0x47, 0x13, 0x46, 0x30, 0xA8, 0x01, 0x95, 0x46, 0xFD
		 , 0xD8, 0x98, 0x80, 0x69, 0xAF, 0xF7, 0x44, 0x8B, 0xB1, 0x5B, 0xFF, 0xFF, 0xBE, 0xD7, 0x5C, 0x89
		 , 0x22, 0x11, 0x90, 0x6B, 0x93, 0x71, 0x98, 0xFD, 0x8E, 0x43, 0x79, 0xA6, 0x21, 0x08, 0xB4, 0x49
		 , 0x62, 0x25, 0x1E, 0xF6, 0x40, 0xB3, 0x40, 0xC0, 0x51, 0x5A, 0x5E, 0x26, 0xAA, 0xC7, 0xB6, 0xE9
		 , 0x5D, 0x10, 0x2F, 0xD6, 0x53, 0x14, 0x44, 0x02, 0x81, 0xE6, 0xA1, 0xD8, 0xC8, 0xFB, 0xD3, 0xE7
		 , 0xE6, 0xCD, 0xE1, 0x21, 0xD6, 0x07, 0x37, 0xC3, 0x87, 0x0D, 0xD5, 0xF4, 0xED, 0x14, 0x5A, 0x45
		 , 0x05, 0xE9, 0xE3, 0xA9, 0xF8, 0xA3, 0xEF, 0xFC, 0xD9, 0x02, 0x6F, 0x67, 0x8A, 0x4C, 0x2A, 0x8D
		 , 0x42, 0x39, 0xFA, 0xFF, 0x81, 0xF6, 0x71, 0x87, 0x22, 0x61, 0x9D, 0x6D, 0x0C, 0x38, 0xE5, 0xFD
		 , 0x44, 0xEA, 0xBE, 0xA4, 0xA9, 0xCF, 0xDE, 0x4B, 0x60, 0x4B, 0xBB, 0xF6, 0x70, 0xBC, 0xBF, 0xBE
		 , 0xC6, 0x7E, 0x9B, 0x28, 0xFA, 0x27, 0xA1, 0xEA, 0x85, 0x30, 0xEF, 0xD4, 0x05, 0x1D, 0x88, 0x04
		 , 0x39, 0xD0, 0xD4, 0xD9, 0xE5, 0x99, 0xDB, 0xE6, 0xF8, 0x7C, 0xA2, 0x1F, 0x65, 0x56, 0xAC, 0xC4
		 , 0x44, 0x22, 0x29, 0xF4, 0x97, 0xFF, 0x2A, 0x43, 0xA7, 0x23, 0x94, 0xAB, 0x39, 0xA0, 0x93, 0xFC
		 , 0xC3, 0x59, 0x5B, 0x65, 0x92, 0xCC, 0x0C, 0x8F, 0x7D, 0xF4, 0xEF, 0xFF, 0xD1, 0x5D, 0x84, 0x85
		 , 0x4F, 0x7E, 0xA8, 0x6F, 0xE0, 0xE6, 0x2C, 0xFE, 0x14, 0x43, 0x01, 0xA3, 0xA1, 0x11, 0x08, 0x4E
		 , 0x82, 0x7E, 0x53, 0xF7, 0x35, 0xF2, 0x3A, 0xBD, 0xBB, 0xD2, 0xD7, 0x2A, 0x91, 0xD3, 0x86, 0xEB
		 , 0x80, 0x0D, 0x41, 0x00, 0xC8, 0x8D, 0x42, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};

	char temp[128] = {0};
	unsigned int v1;
	unsigned int v2;
	unsigned int v3;
	unsigned int v4;
	unsigned int v5;
	unsigned int v6;
	unsigned int v7;
	unsigned int v8;
	int nt1;
	int nt2;
	int nt3;
	int var_60;
	int var_5C;
	int var_58;
	int var_54;
	int var_50;
	int var_4C;
	int var_48;
	int var_44;
	int var_40;
	char var_3C[64];
	int i;


	v2 = arg_0;
	v1 = v2;
	v3 = *(int*)v1;
	v5 = &var_40;
	var_50 = v3;
	v6 = 0x10;
	v4 = *(int*)(v1 + 4);
	var_4C = v4;
	v3 = *(int*)(v1 + 8);
	var_48 = v3;
	v4 = *(int*)(v1 + 0xC);
	v1 = arg_4;
	var_44 = v4;

loc_4109A5:
    v3 = 0;
    v4 = 0;
    v3 = (*(int*)v1) & 0x00ff;                          // mov     cl, [eax]
    v4 = (*((char*)(v1 + 1)) & 0xff) << 8 ;				// mov     dh, [eax+1]
    v1 += 1;
    v8 = v3;
    v8 |= v4;
    *(int*)v5 = v3;
    v1 += 1;
    v3 = 0;
    v4 = v8;
    *(int*)v5 = v8;
    v3 = (*(int*)v1) & 0x00ff;                          // mov     cl, [eax]
    v5 += 0x4;
    v3 <<= 0x10;
    v4 |= v3;
    v1 += 1;
    *(int*)(v5-4) = v4;
    v3 = *(int*)(v5-4);
    v4 = 0;
    v4 = (*(int*)v1) & 0x00ff;                          // mov     dl, [eax]
    v4 <<= 0x18;
    v3 |= v4;
    v1 += 1;
    *((int*)(v5-4)) = v3;
    v6 -= 1;
	if( v6 != 0 )
		goto loc_4109A5;
    *(char*)arg_4 = 0;

loc_4109E5:
	v1 = &var_40;
    v5 = 0;
   	var_60 = v1;
	v1 = *(char*)arg_4;
	v1 <<= 0x6;
	v1 += (int)(v_cc + 16*4);
    var_5C = v1;

loc_4109FF:
	v1 = v5;
	v1 &= 0x80000003;
	if((v1 & 0x08) == 0)
		goto loc_410A0D;
	v1 -= 1;
	v1 |= 0x0FFFFFFFC;
	v1 += 1;

loc_410A0D:
	v3 = *(char*)arg_4;
	v4 = v1;
	v4 = ~v4 + 1;
	v1 = ((int)v_cc[v1 + v3*4 + 16*3]) & 0x00ff;        // mov     al, ds:byte_428CB0[eax+ecx*4]
	v4 &= 0x3;
	var_58 = v1;
	v3 = v4 + 1;
    v1 = v4 - 2;
	v3 &= 0x3;
	v1 &= 0x3;
	v8 = *(int*)(v2 + v3*4);
	v6 = *(int*)(v2 + v1*4);
	v1 = *(char*)arg_4;
	v3 = v4 - 1;
	v3 &= 0x3;
	v3 = *(int*)(v2 + v3*4);
	if(v1 == 0)
		goto loc_410A8F;
	v1 -= 1;
	if(v1 == 0)
		goto loc_410A77;
	v1 -= 1;
	if(v1 == 0)
		goto loc_410A63;
	v1 = 0;
	v1 = ((int)v_cc[v5 + 16*2]) & 0x00ff;                // mov     al, ds:byte_428CA0[esi]
	v3 = ~v3;
	v1 = *(int*)(var_3C - 4 + v1*4);
	v3 |= v8;
	v3 ^= v6;
	v1 += v3;
	goto loc_410AA1;

loc_410A63:
	v1 = 0;
	v3 ^= v6;
	v1 = ((int)v_cc[v5 + 16]) & 0x00ff;                 // mov     al, ds:byte_428C90[esi]
	v3 ^= v8;
	v1 = *(int*)(var_3C - 4 + v1*4);
	v1 += v3;
	goto loc_410AA1;

loc_410A77:
	v1 = v3;
	v3 &= v8;
	v1 = ~v1;
	v1 &= v6;
	v1 |= v3;
	v3 = 0;
	v3 = ((int)v_cc[v5]) & 0x00ff;                     // mov     cl, ds:byte_428C80[esi]
	v6 = *(int*)(var_3C - 4 + v3*4);
	goto loc_410A9F;

loc_410A8F:
	v1 = v8;
	v6 &= v8;
	v1 = ~v1;
	v1 &= v3;
	v3 = var_60;
	v1 |= v6;
	v6 = *(int*)v3;

loc_410A9F:
	v1 += v6;

loc_410AA1:
	v3 = var_5C+v5*4;
	v6 = *(int*)(v2 + v4*4);
	v3 = *(int*)v3;
	v3 += v6;
	v6 = var_58;
	v1 += v3;
	v6 &= 0x0FF;
	v3 = 0x20;
	v2 = v1;
	v3 -= v6;
	v2 >>= v3;                          // shr     ebx, cl
	v3 = v6;
	v6 = 0x4;
	v1 <<= v3;                          // shl     eax, cl
	*(char*)var_5C = v6;
	v2 |= v1;
	v2 += v8;
	v5 += 1;
	v1 = v2;
	v2 = arg_0;
	*(int*)(v2 + v4*4) = v1;
	v4 = var_60;
	v4 += v6;
	var_60 = v4;
	if(v5 < 0x10)
		goto loc_4109FF;

	v1 = *(char*)arg_4;
	v1 += 1;
	*(char*)arg_4 = v1;
	if(v1 < v6)
		goto loc_4109E5;

	v3 = &var_50;
	v1 = v2;
	v3 -= v2;
	v4 = v6;

loc_410B0C:
	v5 = *(int*)(v3 + v1);
	v8 = *(int*)v1;
	v8 += v5;
	*(int*)v1 = v8;
	v1 += v6;
	v4 -= 1;
	if(v4 != 0 )
		goto loc_410B0C;

	return;
}

void sub_410B30(char* arg_0, char* arg_4)
{
	unsigned int v1;
	unsigned int v2;
	unsigned int v3;
	unsigned int v4;
	unsigned int v5;
	unsigned int v6;
	unsigned int v7;

	int ilen;
	char t[8] = {0};
	char tmp[8] = {0};
	int i = 0;

	v2 = arg_4;
	memcpy(tmp, arg_4, 4);
	v5 = arg_0;
	v1 = *(int*)(v5+0x14);
	*(char*)(v5+v1+0x18) = 0x80;
	v4 = *(int*)(v5+0x14);
	v4 += 1;
	v1 = v4;
	*(int*)(v5+0x14) = v4;
	if(v1 >= 0x40)
		goto loc_410B69;

	v3 = 0x40;
	v6 = v1+v5+0x18;
	v3 -= v1;
	v1 = 0;
	v4 = v3;
	v3 >>= 2;
							// .text:00410B60                 rep stosd
	v3 = v4;
	v3 &= 3;
							// .text:00410B67                 rep stosb

loc_410B69:
	if(*(char*)(v5+0x14) <= 0x38)
		goto loc_410B88;
	v6 = v5 + 0x18;
	v1 = *(int*)(v5+4);
	sub_410970(v6, v1);
	v3 = 0x10;
	v1 = 0;
							// .text:00410B86                 rep stosd

loc_410B88:
	v6 = *(char*)v5;
	v6 <<= 3;
	v1 = 0;

loc_410B8F:
	v3 = v1;
	v2 = *(char*)(v5+v1+0x50);
	v3 &= 3;
	v4 = v6;
	v3 <<= 3;
	v4 >>= v3;
	v2 |= (v4 & 0xff);
	*(char*)(v1+v5+0x50) = v2;
	v1 += 1;
	if(v1 < 4)
		goto loc_410B8F;

	v1 = *(int*)v5;
	v3 = v5+0x18;
	v1 >>= 0x18;
	v2 = v5+4;
	v1 &= 0xff;
	v1 >>= 5;
	*(char*)(v5+0x54) = v1;

	arg_0 = v2;
	sub_410970(arg_0,v3);
	sub_41AE80(tmp);

	arg_4[0] = 0x0D;
	arg_4[1] = 0x0A;


	ilen = strlen(tmp);
	memcpy(arg_4 + 2, tmp, ilen);
	arg_4[4+ilen] = '\0';

	v3 = (*((unsigned char *)v2) & 0xf0)>>4;
	v4 = *((unsigned char *)v2) & 0xf;
	if(v3 > 9)
		t[0] = 'A' + v3 - 10;
	else
		t[0] = '0' + v3;
	if(v4 > 9)
		t[1] = 'A' + v4 - 10;
	else
		t[1] = '0' + v4;
	memcpy(arg_4 + 2 + ilen, t, 2);
	return;
}

void XKJS_V12(char* arg_0)
{
	char temp[256] = {0};
	char t[200] = {0};
	int vi = 0;
	int vj = 0;
	unsigned int i;
	unsigned int v4;
	unsigned int v2;
	unsigned int v1;
	unsigned long long v64;
	unsigned int ticks;
	unsigned char msg[] =
	{0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10};
	char tmp[8] = {0};
	char pppoe_randstr[32];
	int pppoe_rand;

	//CFG_get_str(CFG_POE_XKJX_TIME, pppoe_randstr);
	strcpy(pppoe_randstr,nvram_safe_get("wan0_pppoe_xkjx_time"));
	pppoe_rand = atol(pppoe_randstr);
	pppoe_rand += 100;
	v1 = pppoe_rand;
	sprintf(pppoe_randstr, "%d", pppoe_rand);
	//CFG_set_no_event( CFG_POE_XKJX_TIME, pppoe_randstr);
	//CFG_commit(0);

    	//sprintf(t, "%d", v1);
	//setcfmvalue("v12_time", t);

	v64 = ((unsigned long long)v1 * (unsigned long long)0x66666667);
	v4 = v64 >> 32;
	v4 = v4 >> 1;

	memcpy(temp, &v4, 4);
	temp[4] = temp[0];
	temp[0] = temp[3];
	temp[3] = temp[4];
	temp[4] = temp[1];
	temp[1] = temp[2];
	temp[2] = temp[4];
	temp[4] = '\0';

	sprintf(temp+4, "%sradius", arg_0);
	vi = (10+strlen(arg_0));

	memcpy(t, &vi, 4);
	memcpy(t+4, msg, 16);
	memcpy(t+20, &vi, 4);
	memcpy(t+24, temp, vi);

	sub_410B30(t, temp);
	vi -= 10;
	vj = strlen(temp);

	memcpy(temp + vj, arg_0, vi);
	memcpy(arg_0, temp, vi + vj);
	arg_0[vi + vj] = '\0';
}

int strtostr1(char * str){
	int len=0,i=0;
	int theend=0;
	while(str[i++]!='\0'){
		len++;
	}
	theend=len;
	for(i=0;i<len;i++){
		str[theend++]=str[i];
	}
	str[theend]='\0';
	return len;
}

long strtoascii(char* str,int strlen1,int start,int len)
{
	long ret=0x0,tmp=0x0;
	int ilen=len,i=start;

	if(start<strlen1){
		while(i<start+ilen){
			if(str[i]!='\0'){
				tmp=str[i];
				ret=ret*0x100+tmp;
				i++;
			}else{
				ret=ret*0x10000;
				break;
			}
		}
		return ret;
	}else
		return 0x00000000;
}

void isend(long *sxx)
{
	static int isend=0;//判断字符串是否结束
	if(isend==0x0){
		if(*sxx==0x00000000){
			*sxx=0x80000000;
			isend=1;
		}else if((*sxx&0x0000FFFF)==0x0){
			*sxx=*sxx+0x8000;
			isend=1;
		}
	}
}

void xian_xkjs_v30(char *username,char *pass)
{
	char strusername[256];//用户名
	char strpass[256];
	int i=0,j=0;
	char buf[32]={0};
	int doublestrlen=0;
	int strlen1 = 0;
	char *p;
	long tmp1=0;
	long sdx=0xEFCDAB89;
	long sbp=0x98BADCFE;
	long sdi=0x10325476;
	long scx=0x67452301;
	long sax=0x88888888;
	long ssp28=0;
	long ssp40=0;
	long ssp20=0;
	long ssp5c=0;
	long ssp38=0;
	long ssp18=0;
	long ssp30=0;
	long ssp10=0;
	long ssp2c=0;
	long ssp44=0;
	long ssp24=0;
	long ssp3c=0;
	long ssp1c=0;
	long ssp34=0;
	long ssp14=0;
	long ssp4c=0;
	long ssi=0;
	long ssp48=0;
	long ssp50=0;
	long ssp58=0;

	sprintf(strusername,"%s",username);
	sprintf(strpass,"%s",pass);
	strlen1=strtostr1(strusername);
	tmp1=strlen1*0x10000000;

	doublestrlen=strlen1*2;
	sdi=0x98BADCFE;
	sax=strtoascii(strusername,doublestrlen,0,4);
	isend(&sax);
	sdi=sdi+sax;
	scx=sdi+scx+0xD76AA478;
	sax=scx;
	sax=sax>>0x19;
	scx=scx<<0x07;
	sax=sax|scx;
	scx=strtoascii(strusername,doublestrlen,4,4);
	isend(&scx);
	sax=sax+sdx;
	ssp28=scx;
	sdi=sax;
	scx=sdx;
	sdi=~sdi;
	sdi=sdi&sbp;
	scx=scx&sax;
	sdi=sdi|scx;
	scx=ssp28;
	sdi=sdi+scx;
	scx=0x10325476;
	sdi=sdi+scx+0xE8C7B756;
	scx=sdi;
	scx=scx>>0x14;
	scx=scx&0x00000FFF;
	sdi=sdi<<0x0C;
	scx=scx|sdi;
	sdi=strtoascii(strusername,doublestrlen,8,4);
	isend(&sdi);
	scx=scx+sax;
	ssp40=sdi;
	sdi=scx;
	sdi=~sdi;
	sdi=sdi&sdx;
	sdx=scx;
	sdx=sdx&sax;
	sdi=sdi|sdx;
	sdx=ssp40;
	sdi=sdi+sdx;
	sdi=sdi+sbp+0x242070DB;
	sdx=sdi;
	sdx=sdx>>0x0F;
	sdx=sdx&0x0001FFFF;
	sdi=sdi<<0x11;
	sdx=sdx|sdi;
	sdi=strtoascii(strusername,doublestrlen,0x0C,4);
	isend(&sdi);
	sdx=sdx+scx;
	ssp20=sdi;
	sbp=sdx;
	sdi=scx;
	sbp=~sbp;
	sbp=sbp&sax;
	sdi=sdi&sdx;
	sbp=sbp|sdi;
	sdi=ssp20;
	sbp=sbp+sdi;
	sdi=0xEFCDAB89;
	sbp=sdi+sbp+0xC1BDCEEE;
	sdi=sbp;
	sdi=sdi<<0x16;
	sbp=sbp>>0x0A;
	sbp=sbp&0x003FFFFF;
	sdi=sdi|sbp;
	sbp=strtoascii(strusername,doublestrlen,0x10,4);
	isend(&sbp);
	//printf("test1:sax=%lx,scx=%lx,sdx=%lx,sbp=%lx,ssi=%lx,sdi=%lx\n\n",sax,scx,sdx,sbp,8,sdi);
	sdi=sdi+sdx;
	ssp5c=sdi;
	ssp38=sbp;
	sbp=sdx;
	sbp=sbp&ssp5c;
	sdi=~sdi;
	sdi=sdi&scx;
	sdi=sdi|sbp;
	sbp=ssp38;
	sdi=sdi+sbp;
	sbp=strtoascii(strusername,doublestrlen,0x14,4);
	isend(&sbp);
	ssp18=sbp;
	sax=sdi+sax+0xF57C0FAF;
	sdi=sax;
	sdi=sdi>>0x19;
	sdi=sdi&0x0000007F;
	sax=sax<<0x07;
	sdi=sdi|sax;
	sax=ssp5c;
	sdi=sdi+sax;
	sbp=sdi;
	sax=sax&sdi;
	sbp=~sbp;
	sbp=sbp&sdx;
	sbp=sbp|sax;
	sax=ssp18;
	sbp=sbp+sax;
	scx=scx+sbp+0x4787C62A;
	sbp=ssp5c;
	sax=scx;
	sax=sax>>0x14;
	sax=sax&0x00000FFF;
	scx=scx<<0x0C;
	sax=sax|scx;
	scx=strtoascii(strusername,doublestrlen,0x18,4);
	isend(&scx);
	sax=sax+sdi;
	ssp30=scx;
	scx=sax;
	scx=~scx;
	scx=scx&sbp;
	sbp=sax;
	sbp=sbp&sdi;
	scx=scx|sbp;
	sbp=ssp30;
	scx=scx+sbp;
	sbp=sax;
	sdx=scx+sdx+0xA8304613;
	scx=sdx;
	scx=scx>>0x0F;
	scx=scx&0x0001FFFF;
	sdx=sdx<<0x11;
	scx=scx|sdx;
	sdx=strtoascii(strusername,doublestrlen,0x1C,4);
	isend(&sdx);
	scx=scx+sax;
	ssp10=sdx;
	sdx=scx;
	sbp=sbp&scx;
	sdx=~sdx;
	sdx=sdx&sdi;
	sdx=sdx|sbp;
	sbp=ssp10;
	sdx=sdx+sbp;
	sbp=ssp5c;
	sbp=sdx+sbp+0xFD469501;
	sdx=sbp;
	sdx=sdx<<0x16;
	sbp=sbp>>0x0A;
	sbp=sbp&0x003FFFFF;
	sdx=sdx|sbp;
	sbp=strtoascii(strusername,doublestrlen,0x20,4);
	isend(&sbp);
	sdx=sdx+scx;
	ssp2c=sbp;
	ssp5c=sdx;
	sbp=scx;
	sbp=sbp&ssp5c;
	sdx=~sdx;
	sdx=sdx&sax;
	sdx=sdx|sbp;
	sbp=ssp2c;
	sdx=sdx+sbp;
	sdi=sdx+sdi+0x698098D8;
	sdx=sdi;
	sdx=sdx>>0x19;
	sdx=sdx&0x0000007F;
	sdi=sdi<<0x07;
	sdx=sdx|sdi;
	sdi=ssp5c;
	sdx=sdx+sdi;
	sbp=strtoascii(strusername,doublestrlen,0x24,4);
	isend(&sbp);
	sdi=sdi&sdx;
	ssp44=sbp;
	sbp=sdx;
	sbp=~sbp;
	sbp=sbp&scx;
	sbp=sbp|sdi;
	sdi=ssp44;
	sbp=sbp+sdi;
	sax=sax+sbp+0x8B44F7AF;
	sbp=ssp5c;
	sdi=sax;
	sdi=sdi>>0x14;
	sdi=sdi&0x00000FFF;
	sax=sax<<0x0C;
	sdi=sdi|sax;
	sax=strtoascii(strusername,doublestrlen,0x28,4);
	isend(&sax);
	sdi=sdi+sdx;
	ssp24=sax;
	sax=sdi;
	sax=~sax;
	sax=sax&sbp;
	sbp=sdi;
	sbp=sbp&sdx;
	sax=sax|sbp;
	sbp=ssp24;
	sax=sax+sbp;
	sbp=sdi;
	scx=sax+scx+0xFFFF5BB1;
	sax=scx;
	sax=sax>>0x0F;
	sax=sax&0x0001FFFF;
	scx=scx<<0x11;
	sax=sax|scx;
	scx=strtoascii(strusername,doublestrlen,0x2C,4);
	isend(&scx);
	sax=sax+sdi;
	ssp3c=scx;
	scx=sax;
	sbp=sbp&sax;
	scx=~scx;
	scx=scx&sdx;
	scx=scx|sbp;
	sbp=ssp3c;
	scx=scx+sbp;
	sbp=ssp5c;
	sbp=scx+sbp+0x895CD7BE;
	scx=sbp;
	scx=scx<<0x16;
	sbp=sbp>>0x0A;
	sbp=sbp&0x003FFFFF;
	scx=scx|sbp;
	//sbp=str.subtring(30,4);
	//sbp=strtoascii(strusername,doublestrlen,0x30,4);
	sbp=0x00000000;

	scx=scx+sax;
	ssp1c=sbp;
	ssp5c=scx;
	sbp=sax;
	sbp=sbp&ssp5c;
	scx=~scx;
	scx=scx&sdi;
	scx=scx|sbp;
	sbp=ssp1c;
	scx=scx+sbp;
	//sbp=str.substring(34,4);
	sbp=0x00000000;
	ssp34=sbp;
	sdx=scx+sdx+0x6B901122;
	scx=sdx;
	scx=scx>>0x19;
	scx=scx&0x0000007F;
	sdx=sdx<<0x07;
	scx=scx|sdx;
	sdx=ssp5c;
	scx=scx+sdx;
	sbp=scx;
	sdx=sdx&scx;
	sbp=~sbp;
	sbp=sbp&sax;
	sbp=sbp|sdx;
	sdx=ssp34;
	sbp=sbp+sdx;
	sdi=sdi+sbp+0xFD987193;
	sdx=sdi;
	sdx=sdx>>0x14;
	sdx=sdx&0x00000FFF;
	sdi=sdi<<0x0C;
	sdx=sdx|sdi;
	sdx=sdx+scx;
	//sbp=str.substring(38,4);
	sbp=tmp1;//ok????
	sdi=sdx;
	sdi=~sdi;
	ssp14=sbp;
	sbp=ssp5c;
	ssp4c=sdi;
	sdi=sdi&sbp;
	sbp=sdx;
	//long ssi=str.substring(3C,4);
	ssi=0x00000000;
	sbp=sbp&scx;
	//long ssp48=str.substring(3C,4);
	ssp48=0x00000000;
	sdi=sdi|sbp;
	sbp=ssp14;
	sdi=sdi+sbp;
	sbp=sdx;
	sax=sdi+sax+0xA679438E;
	sdi=sax;
	sdi=sdi>>0x0F;
	sdi=sdi&0x0001FFFF;
	sax=sax<<0x11;
	sdi=sdi|sax;
	sdi=sdi+sdx;
	sax=sdi;
	sbp=sbp&sdi;
	sax=~sax;
	ssp50=sax;//0k
	sax=sax&scx;
	sax=sax|sbp;
	sbp=sdx;
	sax=sax+ssi;
	ssi=ssp5c;
	ssi=sax+ssi+0x49B40821;
	sax=ssi;
	sax=sax<<0x16;
	ssi=ssi>>0x0A;
	ssi=ssi&0x003FFFFF;//OK
	sax=sax|ssi;
	ssi=ssp4c;
	sax=sax+sdi;
	ssi=ssi&sdi;
	sbp=sbp&sax;
	ssi=ssi|sbp;
	sbp=ssp28;
	ssi=ssi+sbp;
	sbp=sdi;
	ssi=ssi+scx+0xF61E2562;
	scx=ssi;
	scx=scx>>0x1B;
	scx=scx&0x0000001F;
	ssi=ssi<<0x05;//OK
	scx=scx|ssi;
	ssi=ssp50;
	scx=scx+sax;
	ssi=ssi&sax;
	sbp=sbp&scx;
	ssi=ssi|sbp;
	sbp=ssp30;
	ssi=ssi+sbp;
	ssi=ssi+sdx+0xC040B340;
	sdx=ssi;
	sdx=sdx>>0x17;
	sdx=sdx&0x000001FF;
	ssi=ssi<<0x09;//OK
	sdx=sdx|ssi;
	ssi=sax;
	sdx=sdx+scx;
	ssi=~ssi;
	sbp=sdx;
	ssi=ssi&scx;
	sbp=sbp&sax;
	ssi=ssi|sbp;
	sbp=ssp3c;
	ssi=ssi+sbp;
	ssi=ssi+sdi+0x265E5A51;
	sdi=ssi;
	sdi=sdi>>0x12;
	sdi=sdi&0x00003FFF;
	ssi=ssi<<0x0E;//OK
	sdi=sdi|ssi;
	ssi=scx;
	sdi=sdi+sdx;
	ssi=~ssi;
	sbp=sdi;
	ssi=ssi&sdx;
	sbp=sbp&scx;
	ssi=ssi|sbp;
	ssp58=strtoascii(strusername,doublestrlen,0x00,4);
	sbp=ssp58;
	ssi=ssi+sbp;
	sbp=sdx;
	ssi=ssi+sax+0xE9B6C7AA;
	sax=ssi;
	ssi=ssi>>0x0C;
	ssi=ssi&0x000FFFFF;
	sax=sax<<0x14;//OK
	sax=sax|ssi;
	ssi=sdx;
	ssi=~ssi;
	sax=sax+sdi;
	ssi=ssi&sdi;
	sbp=sbp&sax;
	ssi=ssi|sbp;
	sbp=ssp18;
	ssi=ssi+sbp;
	sbp=sdi;
	ssi=ssi+scx+0xD62F105D;
	scx=ssi;
	ssi=ssi<<0x05;
	scx=scx>>0x1B;
	scx=scx&0x0000001F;
	scx=scx|ssi;
	ssi=sdi;
	scx=scx+sax;
	ssi=~ssi;
	ssi=ssi&sax;
	sbp=sbp&scx;
	ssi=ssi|sbp;
	sbp=ssp24;
	ssi=ssi+sbp;
	ssi=ssi+sdx+0x2441453;
	sdx=ssi;
	sdx=sdx>>0x17;
	sdx=sdx&0x000001FF;
	ssi=ssi<<0x09;
	sdx=sdx|ssi;
	ssi=sax;
	sdx=sdx+scx;
	ssi=~ssi;
	sbp=sdx;
	ssi=ssi&scx;
	sbp=sbp&sax;
	ssi=ssi|sbp;
	sbp=ssp48;
	ssi=ssi+sbp;
	ssi=ssi+sdi+0xD8A1E681;
	sdi=ssi;
	sdi=sdi>>0x12;
	sdi=sdi&0x00003FFF;
	ssi=ssi<<0x0E;
	sdi=sdi|ssi;
	ssi=scx;
	sdi=sdi+sdx;
	ssi=~ssi;
	sbp=sdi;
	ssi=ssi&sdx;
	sbp=sbp&scx;
	ssi=ssi|sbp;
	sbp=ssp38;
	ssi=ssi+sbp;
	sbp=sdx;
	ssi=ssi+sax+0xE7D3FBC8;
	sax=ssi;
	sax=sax<<0x14;
	ssi=ssi>>0x0C;
	ssi=ssi&0x000FFFFF;
	sax=sax|ssi;
	ssi=sdx;
	sax=sax+sdi;
	ssi=~ssi;
	ssi=ssi&sdi;
	sbp=sbp&sax;
	ssi=ssi|sbp;
	sbp=ssp44;
	ssi=ssi+sbp;
	ssi=ssi+scx+0x21E1CDE6;
	scx=ssi;
	scx=scx>>0x1B;
	scx=scx&0x0000001F;
	ssi=ssi<<0x05;
	scx=scx|ssi;
	ssi=sdi;
	ssi=~ssi;
	scx=scx+sax;
	ssi=ssi&sax;
	sbp=sdi;
	sbp=sbp&scx;
	ssi=ssi|sbp;
	sbp=ssp14;
	ssi=ssi+sbp;
	ssi=ssi+sdx+0xC33707D6;
	sdx=ssi;
	ssi=ssi<<0x09;
	sdx=sdx>>0x17;
	sdx=sdx&0x000001FF;
	sdx=sdx|ssi;
	ssi=sax;
	sdx=sdx+scx;
	ssi=~ssi;
	sbp=sdx;
	ssi=ssi&scx;
	sbp=sbp&sax;
	ssi=ssi|sbp;
	sbp=ssp20;
	ssi=ssi+sbp;
	ssi=ssi+sdi+0xF4D50D87;
	sdi=ssi;
	sdi=sdi>>0x12;
	sdi=sdi&0x000003FFF;
	ssi=ssi<<0x0E;
	sdi=sdi|ssi;
	ssi=scx;
	sdi=sdi+sdx;
	ssi=~ssi;
	sbp=sdi;
	ssi=ssi&sdx;
	sbp=sbp&scx;
	ssi=ssi|sbp;
	sbp=ssp2c;
	ssi=ssi+sbp;
	sbp=sdx;
	ssi=ssi+sax+0x455A14ED;
	sax=ssi;
	sax=sax<<0x14;
	ssi=ssi>>0x0C;
	ssi=ssi&0x000FFFFF;
	sax=sax+ssi;
	ssi=sdx;
	sax=sax+sdi;
	ssi=~ssi;
	ssi=ssi&sdi;
	sbp=sbp&sax;
	ssi=ssi|sbp;
	sbp=ssp34;
	ssi=ssi+sbp;
	sbp=sdi;
	ssi=ssi+scx+0xA9E3E905;
	scx=ssi;
	scx=scx>>0x1B;
	scx=scx&0x0000001F;
	ssi=ssi<<0x05;
	scx=scx|ssi;
	ssi=sdi;
	scx=scx+sax;
	ssi=~ssi;
	ssi=ssi&sax;
	sbp=sbp&scx;
	ssi=ssi|sbp;
	sbp=ssp40;
	ssi=ssi+sbp;
	ssi=ssi+sdx+0xFCEFA3F8;
	sdx=ssi;
	sdx=sdx>>0x17;
	sdx=sdx&0x000001FF;
	ssi=ssi<<0x09;
	sdx=sdx|ssi;
	ssi=sax;
	sdx=sdx+scx;
	ssi=~ssi;
	sbp=sdx;
	ssi=ssi&scx;
	sbp=sbp&sax;
	ssi=ssi|sbp;
	sbp=ssp10;
	ssi=ssi+sbp;
	ssi=ssi+sdi+0x676F02D9;
	sdi=ssi;
	sdi=sdi>>0x12;
	sdi=sdi&0x00003FFF;
	ssi=ssi<<0x0E;//ok
	sdi=sdi|ssi;
	sdi=sdi+sdx;
	ssi=scx;
	sbp=sdi;
	ssi=~ssi;
	ssi=ssi&sdx;
	sbp=sbp&scx;
	ssi=ssi|sbp;
	sbp=ssp1c;
	ssi=ssi+sbp;
	sbp=ssp18;
	ssi=ssi+sax+0x8D2A4C8A;
	sax=ssi;
	ssi=ssi>>0x0C;
	ssi=ssi&0x000FFFFF;
	sax=sax<<0x14;
	sax=sax|ssi;
	ssi=sdx;
	ssi=ssi^sdi;
	sax=sax+sdi;
	ssi=ssi^sax;
	ssi=ssi+sbp;
	sbp=ssp2c;
	ssi=ssi+scx+0xFFFA3942;
	scx=ssi;
	ssi=ssi<<0x04;
	scx=scx>>0x1C;
	scx=scx&0x0000000F;
	scx=scx|ssi;
	ssi=sdi;
	ssi=ssi^sax;
	scx=scx+sax;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	sbp=ssp3c;
	ssi=ssi+sdx+0x8771F681;
	sdx=ssi;
	sdx=sdx>>0x15;
	sdx=sdx&0x000007FF;
	ssi=ssi<<0x0B;//0k
	sdx=sdx|ssi;
	sdx=sdx+scx;
	ssi=sdx;
	ssi=ssi^sax;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	sbp=ssp14;
	ssi=ssi+sdi+0x6D9D6122;
	sdi=ssi;
	sdi=sdi>>0x10;
	sdi=sdi&0x0000FFFF;
	ssi=ssi<<0x10;
	sdi=sdi|ssi;
	ssi=sdx;
	sdi=sdi+sdx;
	ssi=ssi^sdi;
	ssp5c=ssi;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	sbp=ssp28;
	ssi=ssi+sax+0xFDE5380C;
	sax=ssi;
	sax=sax<<0x17;
	ssi=ssi>>0x09;
	ssi=ssi&0x007FFFFF;
	sax=sax|ssi;
	ssi=ssp5c;
	sax=sax+sdi;
	ssi=ssi^sax;
	ssi=ssi+sbp;
	sbp=ssp38;
	ssi=ssi+scx+0xA4BEEA44;
	scx=ssi;
	scx=scx>>0x1C;
	scx=scx&0x0000000F;
	ssi=ssi<<0x04;//ok
	scx=scx|ssi;
	ssi=sdi;
	scx=scx+sax;
	ssi=ssi^sax;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	ssi=ssi+sdx+0x4BDECFA9;
	sdx=ssi;
	sdx=sdx>>0x15;
	sdx=sdx&0x000007FF;
	ssi=ssi<<0x0B;
	sdx=sdx|ssi;
	sdx=sdx+scx;
	ssi=sdx;
	sbp=ssp10;
	ssi=ssi^sax;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	sbp=ssp24;
	ssi=ssi+sdi+0xF6BB4B60;
	sdi=ssi;
	ssi=ssi<<0x10;
	sdi=sdi>>0x10;
	sdi=sdi&0x0000FFFF;
	sdi=sdi|ssi;
	ssi=sdx;
	sdi=sdi+sdx;
	ssi=ssi^sdi;
	ssp5c=ssi;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	sbp=ssp34;
	ssi=ssi+sax+0xBEBFBC70;
	sax=ssi;
	ssi=ssi>>0x09;
	ssi=ssi&0x007FFFFF;
	sax=sax<<0x17;
	sax=sax|ssi;
	ssi=ssp5c;
	sax=sax+sdi;
	ssi=ssi^sax;
	ssi=ssi+sbp;
	sbp=ssp58;
	ssi=ssi+scx+0x289B7EC6;
	scx=ssi;
	scx=scx>>0x1C;
	scx=scx&0x0000000F;
	ssi=ssi<<0x04;
	scx=scx|ssi;
	ssi=sdi;
	scx=scx+sax;
	ssi=ssi^sax;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	sbp=ssp20;
	ssi=ssi+sdx+0xEAA127FA;
	sdx=ssi;
	sdx=sdx>>0x15;
	sdx=sdx&0x000007FF;
	ssi=ssi<<0x0B;
	sdx=sdx|ssi;
	sdx=sdx+scx;
	ssi=sdx;
	ssi=ssi^sax;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	sbp=ssp30;
	ssi=ssi+sdi+0xD4EF3085;
	sdi=ssi;
	sdi=sdi>>0x10;
	sdi=sdi&0x0000FFFF;
	ssi=ssi<<0x10;
	sdi=sdi|ssi;
	ssi=sdx;
	sdi=sdi+sdx;
	ssi=ssi^sdi;
	ssp5c=ssi;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	sbp=ssp44;
	ssi=ssi+sax+0x4881D05;
	sax=ssi;
	sax=sax<<0x17;
	ssi=ssi>>0x09;
	ssi=ssi&0x007FFFFF;//ok
	sax=sax|ssi;
	ssi=ssp5c;
	sax=sax+sdi;
	ssi=ssi^sax;
	ssi=ssi+sbp;
	sbp=ssp1c;
	ssi=ssi+scx+0xD9D4D039;
	scx=ssi;
	scx=scx>>0x1C;
	scx=scx&0x0000000F;
	ssi=ssi<<0x04;
	scx=scx|ssi;
	ssi=sdi;
	scx=scx+sax;
	ssi=ssi^sax;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	ssi=ssi+sdx+0xE6DB99E5;
	sbp=ssp48;
	sdx=ssi;
	ssi=ssi<<0x0B;
	sdx=sdx>>0x15;
	sdx=sdx&0x000007FF;
	sdx=sdx|ssi;
	sdx=sdx+scx;
	ssi=sdx;
	ssi=ssi^sax;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	sbp=ssp40;
	ssi=ssi+sdi+0x1FA27CF8;
	sdi=ssi;
	ssi=ssi<<0x10;
	sdi=sdi>>0x10;
	sdi=sdi&0x0000FFFF;//ok
	sdi=sdi|ssi;
	ssi=sdx;
	sdi=sdi+sdx;
	ssi=ssi^sdi;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	sbp=ssp58;
	ssi=ssi+sax+0xC4AC5665;
	sax=ssi;
	ssi=ssi>>0x09;
	ssi=ssi&0x007FFFFF;
	sax=sax<<0x17;
	sax=sax|ssi;
	ssi=sdx;
	sax=sax+sdi;
	ssi=~ssi;
	ssi=ssi|sax;
	ssi=ssi^sdi;
	ssi=ssi+sbp;
	sbp=ssp10;
	ssi=ssi+scx+0xF4292244;
	scx=ssi;
	scx=scx>>0x1A;
	scx=scx&0x0000003F;
	ssi=ssi<<0x06;
	scx=scx|ssi;
	ssi=sdi;
	scx=scx+sax;
	ssi=~ssi;
	ssi=ssi|scx;
	ssi=ssi^sax;
	ssi=ssi+sbp;
	sbp=ssp14;
	ssi=ssi+sdx+0x432AFF97;
	sdx=ssi;
	sdx=sdx>>0x16;
	sdx=sdx&0x000003FF;
	ssi=ssi<<0xA;
	sdx=sdx|ssi;
	ssi=sax;
	sdx=sdx+scx;
	ssi=~ssi;
	ssi=ssi|sdx;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	sbp=ssp18;
	ssi=ssi+sdi+0xAB9423A7;
	sdi=ssi;
	sdi=sdi>>0x11;
	sdi=sdi&0x00007FFF;
	ssi=ssi<<0x0F;
	sdi=sdi|ssi;
	ssi=scx;
	sdi=sdi+sdx;
	ssi=~ssi;
	ssi=ssi|sdi;
	ssi=ssi^sdx;
	ssi=ssi+sbp;
	sbp=ssp1c;
	ssi=ssi+sax+0xFC93A039;
	sax=ssi;
	sax=sax<<0x15;
	ssi=ssi>>0x0B;
	ssi=ssi&0x001FFFFF;
	sax=sax|ssi;
	ssi=sdx;
	sax=sax+sdi;
	ssi=~ssi;
	ssi=ssi|sax;
	ssi=ssi^sdi;
	ssi=ssi+sbp;
	sbp=ssp20;
	ssi=ssi+scx+0x655B59C3;
	scx=ssi;
	ssi=ssi<<0x06;
	scx=scx>>0x1A;
	scx=scx&0x0000003F;//ok
	scx=scx|ssi;
	ssi=sdi;
	ssi=~ssi;
	scx=scx+sax;
	ssi=ssi|scx;
	ssi=ssi^sax;
	ssi=ssi+sbp;
	sbp=ssp24;
	ssi=ssi+sdx+0x8F0CCC92;
	sdx=ssi;
	ssi=ssi<<0x0A;
	sdx=sdx>>0x16;
	sdx=sdx&0x000003FF;
	sdx=sdx|ssi;
	ssi=sax;
	ssi=~ssi;
	sdx=sdx+scx;
	ssi=ssi|sdx;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	sbp=ssp28;
	ssi=ssi+sdi+0xFFEFF47D;
	sdi=ssi;
	sdi=sdi>>0x11;
	sdi=sdi&0x00007FFF;
	ssi=ssi<<0x0F;
	sdi=sdi|ssi;
	ssi=scx;
	sdi=sdi+sdx;
	ssi=~ssi;
	ssi=ssi|sdi;
	ssi=ssi^sdx;
	ssi=ssi+sbp;
	sbp=ssp2c;
	ssi=ssi+sax+0x85845DD1;
	sax=ssi;
	sax=sax<<0x15;
	ssi=ssi>>0x0B;
	ssi=ssi&0x001FFFFF;
	sax=sax|ssi;
	ssi=sdx;
	sax=sax+sdi;
	ssi=~ssi;
	ssi=ssi|sax;
	ssi=ssi^sdi;
	ssi=ssi+sbp;
	sbp=ssp48;
	ssi=ssi+scx+0x6FA87E4F;
	scx=ssi;
	scx=scx>>0x1A;
	scx=scx&0x0000003F;
	ssi=ssi<<0x06;
	scx=scx|ssi;
	ssi=sdi;
	scx=scx+sax;
	ssi=~ssi;
	ssi=ssi|scx;
	ssi=ssi^sax;
	ssi=ssi+sbp;
	sbp=ssp30;
	ssi=ssi+sdx+0xFE2CE6E0;
	sdx=ssi;
	sdx=sdx>>0x16;
	sdx=sdx&0x000003FF;
	ssi=ssi<<0x0A;
	sdx=sdx|ssi;
	ssi=sax;
	sdx=sdx+scx;
	ssi=~ssi;
	ssi=ssi|sdx;
	ssi=ssi^scx;
	ssi=ssi+sbp;
	sdi=ssi+sdi+0xA3014314;
	ssi=sdi;
	ssi=ssi>>0x11;
	ssi=ssi&0x00007FFF;
	sdi=sdi<<0x0F;
	ssi=ssi|sdi;
	sdi=scx;
	ssi=ssi+sdx;
	sdi=~sdi;
	sbp=ssp34;
	sdi=sdi|ssi;
	sdi=sdi^sdx;
	sdi=sdi+sbp;
	sbp=ssp38;
	sdi=sdi+sax+0x4E0811A1;
	sax=sdi;
	sax=sax<<0x15;
	sdi=sdi>>0x0B;
	sdi=sdi&0x001FFFFF;
	sax=sax|sdi;
	sdi=sdx;
	sax=sax+ssi;
	sdi=~sdi;
	sdi=sdi|sax;
	sdi=sdi^ssi;
	sdi=sdi+sbp;
	sbp=ssp3c;
	sdi=sdi+scx+0xF7537E82;
	scx=sdi;
	scx=scx>>0x1A;
	scx=scx&0x0000003F;
	sdi=sdi<<0x06;
	scx=scx|sdi;
	sdi=ssi;
	scx=scx+sax;
	sdi=~sdi;
	sdi=sdi|scx;
	sdi=sdi^sax;
	sdi=sdi+sbp;
	sbp=ssp40;
	sdi=sdi+sdx+0xBD3AF235;
	sdx=sdi;
	sdx=sdx>>0x16;
	sdx=sdx&0x000003FF;
	sdi=sdi<<0x0A;
	sdx=sdx|sdi;
	sdi=sax;
	sdx=sdx+scx;
	sdi=~sdi;
	sdi=sdi|sdx;
	sdi=sdi^scx;
	sdi=sdi+sbp;
	sbp=ssp44;
	ssi=sdi+ssi+0x2AD7D2BB;
	sdi=ssi;
	sdi=sdi>>0x11;
	sdi=sdi&0x00007FFF;
	ssi=ssi<<0x0F;//ok////
	sdi=sdi|ssi;
	ssi=scx;
	sdi=sdi+sdx;
	ssi=~ssi;
	ssi=ssi|sdi;
	ssi=ssi^sdx;
	ssi=ssi+sbp;
	sax=ssi+sax+0xEB86D391;
	ssi=0x67452301;
	ssi=ssi+scx;
	scx=sax;
	scx=scx<<0x15;
	sax=sax>>0x0B;
	sax=sax&0x001FFFFF;
	scx=scx|sax;
	sax=0x98BADCFE;
	p = buf;
	memset(buf,0,sizeof(buf));
	sprintf(p,"%02x%02x%02x%02x",ssi&0xff,(ssi>>8)&0xff,(ssi>>16)&0xff,(ssi>>24)&0xff);
	//printf("01-08:%lx\n",ssi);
	ssi=0xEFCDAB89;
	sax=sax+sdi;
	scx=scx+ssi;
	//printf("17-24:%lx\n",sax);
	sax=0x10325476;
	scx=scx+sdi;
	sax=sax+sdx;
	p = p + 8;
	sprintf(p,"%02x%02x%02x%01x",scx&0xff,(scx>>8)&0xff,(scx>>16)&0xff,(scx>>28)&0xf);
	//printf("09-16:%lx\n",scx);
	//sprintf(p,"%02x%02x%02x%02x",sax&0xff,(sax>>8)&0xff,(sax>>16)&0xff,(sax>>24)&0xff);
	//printf("25-32:%lx\n",sax);
	sprintf(pass,"%s%s",strpass,buf);
}


long Mem2Reg(unsigned long mem)
{
	int i=0;
	long retv=0x0,tmp=0x0;
	for(i=0;i<4;i++){
		tmp=0x0;
		tmp=(mem>>(i*8))&0x0ff;
		retv=retv*0x100+tmp;
	}
	return retv;
}

void nanchang_getstr2(char* username,unsigned long tm,unsigned long *lbuf)
{
	unsigned long eax=0x0,esi=0x0,ecx=0x0,edx=0x0,edi=0x0,ebp=0x0,ebx=0x0;
	unsigned long esp78=0x0,esp10=0x0,esp14=0x0,esp18=0x0;
	unsigned long lbuf2[4]={0x070C1116,0x05090E14,0x040B1017,0x060A0F15};
	unsigned long lbuf3[64]={0x78A46AD7,0x56B7C7E8,
		0xDB702024,0xEECEBDC1,0xAF0F7CF5,0x2AC68747,
		0x134630A8,0x019546FD,0xD8988069,0xAFF7448B,
		0xB15BFFFF,0xBED75C89,0x2211906B,0x937198FD,
		0x8E4379A6,0x2108B449,0x62251EF6,0x40B340C0,
		0x515A5E26,0xAAC7B6E9,0x5D102FD6,0x53144402,
		0x81E6A1D8,0xC8FBD3E7,0xE6CDE121,0xD60737C3,
		0x870DD5F4,0xED145A45,0x05E9E3A9,0xF8A3EFFC,
		0xD9026F67,0x8A4C2A8D,0x4239FAFF,0x81F67187,
		0x22619D6D,0x0C38E5FD,0x44EABEA4,0xA9CFDE4B,
		0x604BBBF6,0x70BCBFBE,0xC67E9B28,0xFA27A1EA,
		0x8530EFD4,0x051D8804,0x39D0D4D9,0xE599DBE6,
		0xF87CA21F,0x6556ACC4,0x442229F4,0x97FF2A43,
		0xA72394AB,0x39A093FC,0xC3595B65,0x92CC0C8F,
		0x7DF4EFFF,0xD15D8485,0x4F7EA86F,0xE0E62CFE,
		0x144301A3,0xA111084E,0x827E53F7,0x35F23ABD,
		0xBBD2D72A,0x91D386EB};
	unsigned long lbuf4[4]={0x01060B00,0x050A0F04,0x090E0308,0x0D02070C};
	unsigned long lbuf5[4]={0x05080B0E,0x0104070A,0x0D000306,0x090C0F02};
	unsigned long lbuf6[4]={0x00070E05,0x0C030A01,0x080F060D,0x040B0209};

	unsigned long u_name[16]={0x0};
	char count[16]={0};
	char *ver="nanchang3.0�";
	char strbuf[32];
	int i=0,s_len=0;
	u_name[0]=tm;
	while(username[i] !='\0' && username[i++]!='@')
		count[i-1]=username[i-1];
	count[i]='\0';
	u_name[14]=Mem2Reg((i+0x0f-1)<<3);
	sprintf(strbuf,"%s%s",count,ver);
	s_len=strlen(strbuf);

	for(i=0;i<s_len;i++)
	{
		int itmp=strbuf[i]&0xff;
		u_name[i/4+1]=(u_name[i/4+1]<<8)+itmp;
	}
	u_name[i/4+1]=u_name[i/4+1]<<((4-(i%4))*8);
	//u_name[0]=0x0ebe78ca;
	//sscanf("07916349240nanchang3.0�","%08x%08x%08x%08x%08x%08x",u_name[1],u_name[2],u_name[3],u_name[4],u_name[5],u_name[6]);
	//u_name[1]=0x30373931;
	//u_name[2]=0x36333439;
	//u_name[3]=0x3234306E;
	//u_name[4]=0x616E6368;
	//u_name[5]=0x616E6733;
	//u_name[6]=0x2E308000;
	//u_name[14]=0xd0000000;

	esp18=0x77c09f8e;



a00413F05:
	eax=0x0;
	esi=0x0;
	esp10=eax;
	eax=esp78;
	eax=eax<<0x6;
	eax=eax+0x004316B8;

	esp14=esp78*0x10;
a00413F1F:
	eax=esi;

	eax=eax&0x03;

	ecx=esp78;
	edx=eax;

	edx=0xffffffff-edx+1;

	eax=(Mem2Reg(lbuf2[ecx])>>(eax*8))&0xff;

	edx=edx&0x3;

	esp18=(esp18&0xffffff00)+(eax&0xff);

	ecx=edx+0x1;
	eax=edx-0x2;

	ecx=ecx&0x3;
	eax=eax&0x3;

	ebp=Mem2Reg(lbuf[ecx]);
	edi=Mem2Reg(lbuf[eax]);

	eax=esp78;


	ecx=edx-1;

	ecx=ecx&0x3;


	ecx=Mem2Reg(lbuf[ecx]);

	if(eax == 0)
		goto a00413FAF;
	--eax;
	if(eax == 0)
		goto a00413F97;
	--eax;
	if(eax == 0)
		goto a00413F83;
	eax=0x0;

	eax=(Mem2Reg(lbuf6[esi/4])>>((esi%4)*8))&0xff;
	ecx=~ecx;

	eax=Mem2Reg(u_name[eax]);

	ecx=ecx|ebp;
	ecx=ecx^edi;

	eax+=ecx;
	goto a00413FC1;

a00413F83:
	eax=0x0;

	ecx=ecx^edi;

	eax=(Mem2Reg(lbuf5[esi/4])>>((esi%4)*8))&0xff;
	ecx=ecx^ebp;

	eax=Mem2Reg(u_name[eax]);

	eax+=ecx;


	goto a00413FC1;

a00413F97:
	eax=ecx;
	ecx=ecx&ebp;
	eax=~eax;
	eax=eax&edi;
	eax=eax|ecx;
	ecx=0x0;

	ecx=(Mem2Reg(lbuf4[esi/4])>>((esi%4)*8))&0xff;
	edi=Mem2Reg(u_name[ecx]);
	goto a00413FBF;

a00413FAF:
	eax=ebp;
	edi=edi&ebp;
	eax=~eax;
	eax=eax&ecx;

	ecx=esp10;
	eax=eax|edi;
	edi=Mem2Reg(u_name[ecx]);

a00413FBF:
	eax=eax+edi;

a00413FC1:
	edi=Mem2Reg(lbuf[edx]);
	ecx=Mem2Reg(lbuf3[esp14]);
	ecx+=edi;
	edi=esp18;

	eax+=ecx;

	edi=edi&0x0ff;

	ecx=0x20;

	ebx=eax;

	ecx-=edi;

	ebx=ebx>>ecx;

	ecx=edi;

	edi=0x04;

	eax=eax<<ecx;

	esp14++;

	ebx=ebx|eax;

	ebx+=ebp;

	esi++;

	eax=ebx;

	lbuf[edx]=Mem2Reg(eax);


	esp10++;
	if(esi<0x10)
		goto a00413F1F;

	eax=++esp78;
	if(eax<edi)
		goto a00413F05;
	lbuf[0]=Mem2Reg(Mem2Reg(lbuf[0])+0x67452301);
	lbuf[1]=Mem2Reg(Mem2Reg(lbuf[1])+0xefcdab89);
	lbuf[2]=Mem2Reg(Mem2Reg(lbuf[2])+0x98badcfe);
	lbuf[3]=Mem2Reg(Mem2Reg(lbuf[3])+0x10325476);

}

unsigned long nanchang_getstr1(unsigned long str)
{
	unsigned long eax=0x0,ecx=0x0,edx=0x0;
	unsigned long ltmp=0x0,eaxtmp=0x0;
	int ebp=3,edi=0,esi=0;
	eax=0x03;
a42221c:
	ebp=eax;
	edi=ecx;
	eax=(str>>((3-ebp)*8))&0xff;
	ecx=(ltmp>>((3-edi)*8))&0xff;
	edx=eax;
	ecx=ecx<<1;
	edx=edx&0x01;
	ecx=ecx|edx;
	eax=eax>>0x01;
	esi++;
	if(ebp==3)
		str=(str&0xffffff00)+eax;
	else if(ebp==2)
		str=(str&0xffff00ff)+eax*0x100;
	else if(ebp==1)
		str=(str&0xff00ffff)+eax*0x10000;
	else if(ebp==0)
		str=(str&0x00ffffff)+eax*0x1000000;

	eax=esi;

	if(edi==0)
		ltmp=(ltmp&0x00ffffff)+ecx*0x1000000;
	else if(edi==1)
		ltmp=(ltmp&0xff00ffff)+ecx*0x10000;
	else if(edi==2)
		ltmp=(ltmp&0xffff00ff)+ecx*0x100;
	else if(edi==3)
		ltmp=(ltmp&0xffffff00)+ecx;

	edx=0x0;
	edx=edx&0x07;
	ecx=0x03;
	eax+=edx;
	edx=esi;
	eax=eax>>0x03;
	ecx-=eax;
	edx&=0x03;
	eax=ecx;
	ecx=edx;
	if(esi<0x20)
		goto a42221c;
	return ltmp;

}
void nanchang_getstr3(unsigned long tm2,unsigned long *ret)
{
	unsigned long eax=0x0,ecx=0x0,edx=0x0;
	int esi=0x0;

	ecx=(tm2>>24)&0xff;
	ecx=ecx>>2;
	ret[0]=ecx*0x1000000;

	ecx=(tm2>>24)&0xff;
	ecx=ecx&0x03;
	ecx=ecx<<4;
	ret[0]+=ecx*0x10000;

	edx=(tm2>>16)&0xff;
	edx=edx>>0x04;
	edx=edx|ecx;
	ret[0]=(ret[0]&0xff00ffff)+edx*0x10000;

	ecx=(tm2>>16)&0xff;
	ecx=ecx&0x0f;
	ecx=ecx<<2;
	ret[0]+=ecx*0x100;

	edx=(tm2>>8)&0xff;
	edx=edx>>6;
	edx=edx|ecx;
	ret[0]=(ret[0]&0xffff00ff)+edx*0x100;

	ecx=(tm2>>8)&0xff;
	ecx=ecx&0x3f;
	ret[0]+=ecx;

	edx=tm2&0xff;
	edx=edx>>2;
	ret[1]=edx*0x1000000;

	ecx=tm2&0xff;
	ecx=ecx&0x03;
	ecx=ecx<<4;
	ret[1]+=ecx*0x10000;

	ecx=0x0;
	while(ecx<6)
	{
		edx=(ret[ecx/4]>>((3-(ecx%4))*8))&0xff;
		edx+=0x20;
		if(edx>=0x40)
			edx++;
		if((ecx%4)==0)
			ret[ecx/4]=(ret[ecx/4]&0x00ffffff)+edx*0x1000000;
		if((ecx%4)==1)
			ret[ecx/4]=(ret[ecx/4]&0xff00ffff)+edx*0x10000;
		if((ecx%4)==2)
			ret[ecx/4]=(ret[ecx/4]&0xffff00ff)+edx*0x100;
		if((ecx%4)==3)
			ret[ecx/4]=(ret[ecx/4]&0xffffff00)+edx;
		ecx++;
	}
}


void nanchang_Ver18(char* uname,unsigned long tm,char* retchar)
{
	unsigned long ltemp=0x0;
	unsigned long ret[2]={0x0,0x0};
	unsigned long byte10[9];
	unsigned long lbuf[4]={0x01234567,0x89abcdef,0xfedcba98,0x76543210};//??
	char cbuf[10]="";
	//char *uname="07916349240@jxcard";
	//char retchar[10]={0};
	int i=0;
	//tm=0x0ebecbe0;

	ltemp=nanchang_getstr1(tm);

    	nanchang_getstr2(uname,tm,lbuf);

	nanchang_getstr3(ltemp,ret);

	byte10[0]=0x0d300000;
	byte10[0]+=ret[0]>>0x10;
	byte10[1]=(ret[0]<<0x10)+(ret[1]>>0x10);
	sprintf(cbuf,"%08x",lbuf[0]);
	byte10[2]=cbuf[0]*0x1000000+cbuf[1]*0x10000;
	while(i<10)
	{
		retchar[i]=(char)((byte10[i/4]>>((3-(i%4))*8))&0xff);
		i++;
	}
	strcat(retchar,uname);
}
///start 北京宽带我世界//////

void  kdwsj_EnctryUser(char const*,char *);
void gettmpuser(char const*,char *);

// 该函数用于取 加密算法的所以使用的用户名
// 临时用户名格式   12位 大写字符串形式的
// mac地址 + 14 当前日期字符串形式 + 5位 拨号失败次数
// 字符串形式 + "::" + 在真实用户名
// 注意各个路由器实现去时间 和 mac地址字符串方式
// 个不相同，因此要移植是需要修改函数的

int getIfMac(char *ifname, char *if_hw)
{
	unsigned char wan_mac[6]={0};
	int rtv;
	
	rtv = iflib_getifhwaddr(ifname, wan_mac);
	sprintf(if_hw, "%02X%02X%02X%02X%02X%02X",
			(wan_mac[0] & 0377), (wan_mac[1] & 0377), (wan_mac[2] & 0377),
			(wan_mac[3] & 0377), (wan_mac[4] & 0377), (wan_mac[5] & 0377));
	return rtv;
}

void gettmpuser(char const *user,char *tmpuser)
{
	char			strtmp[256]={0} ;
	char			error_times_str[6];
	char			timestr[15]={0};
	time_t			timep;
	struct tm		*p=NULL;
	static short	error_times = 0;
	char            mac_str[13]={0};
	int			 val ;

	char pc_time[64];

	memset(pc_time,0,sizeof(pc_time));
	strcpy(pc_time,nvram_safe_get("wan0_pppoe_xkjx_time"));
	timep = atol(pc_time);
	p = localtime(&timep);


	getIfMac(nvram_safe_get("wan_ifnames"),mac_str);


	sprintf(error_times_str,"%05d",error_times);
	error_times_str[5] ='\0';
	++error_times;



	sprintf(timestr,"%04d%02d%02d%02d%02d%02d",(1900+p->tm_year), (1+p->tm_mon),
				p->tm_mday,(p->tm_hour+8), p->tm_min, p->tm_sec);

	timestr[14]='\0';
	strcpy(strtmp,mac_str);
	strcat(strtmp,timestr);
	strcat(strtmp,error_times_str);
	strcat(strtmp,"::");
	strcat(strtmp,user);

	strcpy(tmpuser,strtmp);

}
//北京宽带我世界 加密算法
void kdwsj_EnctryUser(char const *user,char* enuser)
{
	long edi,ebx,eax,esi,edx,ecx;

	long p = (long) user; //p ebp+c;
	long v10;
	long v14;
	char v1;
	long vC;
	char *puser ;
	char *ptmp;
	long v8;
	long itmp;
	char al;
	char buf[17] = {0x11,0x34,0xc9,0x23,0x75,0x18,0xd7,0xe2,0x12,0x35,0x29,0x2b,0xec,0xb6,0x23,0x19,0x0};
	char strtable[] = "9012345678abcdeABCDEFGHIJKLMNfghijklmnUVWXYZxyzuvwopqrstOPQRST";

	edi = 0;
	if (user == NULL)
		return;
	else
		eax = strlen(user);
	ebx = eax;
	v10 = ebx;
	eax = ebx +1;

	puser = (char *)malloc(eax);
	if (puser == NULL)
		return;
	ptmp = puser;
	eax   = (long) puser;
	ecx   =  eax;
	v14   = (long) puser;
	v1     =  v1 & 0;
	esi    = 0x25;
	v8     =  edi;
	if (ebx < edi)
		return;
	edx  = p;
	ecx  = eax;
	edx  = edx - eax;
	vC   = edi;
	p     = edx;

loc_00FEC536:
	al  =  *((char *)((long)(ecx+edx)));
	if (v1 !=0)
		goto loc_00FEC58F;
	else
		edi = edi ^ edi;
loc_FEC541:
	if (al == (char)*(edi+strtable))
	{
		goto loc_00FEC551;
	}
	edi++;
	if (edi < 0x3e)
	{
		goto loc_FEC541;
	}
	goto loc_00FEC556;

loc_00FEC551:
	if (edi != -1)
		goto loc_00FEC55C;
loc_00FEC556:
	v1 =1;
	goto loc_00FEC58F;
loc_00FEC55C:
	eax = v8; //user len;
	if (eax < 0x8000000)
		edx = 0x0;
	else
		edx = 0xffffffff;
	ebx  = 0x10;
       itmp = eax;
	eax  = itmp /ebx;  //idiv ebx;
	edx  = itmp %ebx;
	ebx  = v10;
	eax  = (0x000000ff & buf[edx]);
	edx  = esi + esi *2;
	eax  = eax ^ edx;
	edx  = edx ^ edx;
	eax  = eax ^ vC;
	eax  = eax + edi;

	edi   = 0x3e;
	itmp = eax;
	eax  = itmp / edi;
	edx  = itmp % edi;
	eax  =  edx + 0x24d9;
	esi   = esi ^eax;

	al     = strtable[edx];
	edx  = p;
loc_00FEC58F:
	v8++;
	vC += 5;
	*(ptmp) = al;
	ptmp++;
	ecx++;
	if (v8 < ebx)
		goto loc_00FEC536;

	memcpy(enuser,puser,ebx);
	enuser[ebx] = '\0';
	if(puser!=NULL)
	{
		free(puser);
		puser = NULL;
	}
	return;
}
//北京宽带我世界
void  beijing_kdwsj(char *user, char *enuser)
{
	char tmpuser[256]={0};
	char tmpuser2[256]={0};

	gettmpuser(user,tmpuser);
	kdwsj_EnctryUser(tmpuser,tmpuser2);
	strcpy(enuser,"a:");
	strcat(enuser,tmpuser2);
}

///end 北京宽带我世界///////

void GetEncryptUserPasswd(char *user, char *passwd, int encrypttype, char *enuser, char *enpasswd)
{
    	MD5_CTX md5_ctx;

	char isshanxun[4];
	char isshanxun1[4];
	char is_nanchang[4];
	char err_value[4];
	char plugplay_flag[4] = {0};
	strcpy(plugplay_flag,nvram_safe_get("plugplay_flag"));
	if(plugplay_flag[0] == 'y'){
		strcpy(enuser,nvram_safe_get("wan0_pppoe_username"));
		strcpy(enpasswd,nvram_safe_get("wan0_pppoe_passwd"));
		return ;
	}else{
	strcpy(enpasswd, passwd);
	strcpy(enuser, user);
	
//add by roy, 2011/10/24
	strcpy(is_nanchang,nvram_safe_get("wan0_pppoe_nanchang"));

	if(strcmp(is_nanchang,"1") == 0)
	{
		// 南昌星空极速2.5.0016v14
		if(encrypttype == 0){
			if(strlen(user) > 16) {
					return;
			}
			strcpy(enuser, user);
			XKJS_V12(enuser);
		}
		else{
			strcpy(enuser, user);
		}
		diag_printf("pppoe xkjs: encrypttype[%d]user[%s]pass[%s]\n",encrypttype,enuser,enpasswd);
		return;
	}
//end	
	strcpy(isshanxun,nvram_safe_get("wan0_pppoe_shanxun"));
#if defined(PPPOE_SHANXI_WLJB)
	//special for shanxi wangluojianbing
	strcpy(isshanxun,"2");
	if(strcmp(isshanxun,"2") == 0)
	{
		//山西网络尖兵，20110311
			strcpy(enuser, user);
			henan_pppoe_user_shanxi(user, enuser);
	}
	else 
#endif
	if( strcmp(isshanxun,"1") == 0 )
	{
		int i=0, namelen=0;
		int itmp;
		char *p;
		char nametmp[64];
		char pppoename[256];

		strcpy(pppoename,user);
		p = pppoename+1;
		namelen=strlen(pppoename)/3;
		for(i=0;i<namelen;i++)
		{
			sscanf(p,"%2X",&itmp);
			nametmp[i] = (char)itmp;
			p += 3;
		}
		nametmp[namelen] = '\0' ;
		memset(enuser,0,sizeof(enuser));
		memcpy(enuser, nametmp, namelen+1);

	}
	else
	{
		if(encrypttype == 0)
		{
			strcpy(enuser, user);
		}
		else if(encrypttype==1)
		{

			strcpy(err_value, nvram_safe_get("err_check"));
			if(strlen(user) > 16) {
				if(strcmp(err_value,"2") != 0)
					nvram_set("err_check","7");
				return;
			}
			henan_pppoe_user(user, enuser);
			if(strcmp(err_value,"2") != 0)
				nvram_set("err_check","7");
		}
		else if(encrypttype == 2)
		{
			strcpy(enuser, "\r\n");
			strcat(enuser, user);
		}
		else if(encrypttype == 3)
		{
			strcpy(enuser, user);
			xian_pppoe_user(user, passwd, enpasswd);
		}
		else if(encrypttype == 4)
		{
			xian_pppoe_user(user, passwd, enuser);
		}
		else if(encrypttype == 5)
		{
			strcpy(enuser, user);
			xian_xkjs_v30(enuser,enpasswd);
		}
		else if(encrypttype == 6)
		{
			unsigned char digest[16],tmp[256];
			unsigned int len;
			len = strlen (user);
			len <<= 1;
			strcpy(tmp,user);
			strcat(tmp,user);

		    	MD5Init(&md5_ctx);
		    	MD5Update(&md5_ctx,tmp,len);
		    	MD5Final((char *)digest,&md5_ctx);

			//MD5String(tmp, len, digest);
			sprintf(tmp,"%02x%02x%02x%02x",digest[0],digest[1],digest[2],digest[3]);
			strcpy(enuser, "\r\n");
			strcat(enuser, tmp);
			strcat(enuser, user);
		}
		else if(encrypttype==7)
		{
			strcpy(enuser, user);
			xkjs_ver31(enuser);
		}
		else if(encrypttype==8)
		{
			strcpy(enuser, "^^");
			strcat(enuser, user);
			xkjs_ver32(enpasswd);
		}
		else if(encrypttype==9)
		{
		    beijing_kdwsj(user,enuser);
		}
		else if(encrypttype==10)
		{

			char pppoe_randstr[64];
			unsigned long tm;
			memset(pppoe_randstr,0,sizeof(pppoe_randstr));

			strcpy(pppoe_randstr,nvram_safe_get("wan0_pppoe_xkjx_time"));

			tm = atoi(pppoe_randstr)/5;

			nanchang_Ver18(user,tm,enuser);
		}
		/*
		else if (encrypttype == 11)
		{
			if(strlen(user) > 16) {
				return;
			}
			strcpy(enuser, user);
			XKJS_V12(enuser);
		}
		*/
		else
		{
			strcpy(enuser, user);
		}

	}
	//diag_printf("pppoe xkjs: encrypttype[%d]user[%s]pass[%s]\n",encrypttype,enuser,enpasswd);
	return;
	}
}
#endif

