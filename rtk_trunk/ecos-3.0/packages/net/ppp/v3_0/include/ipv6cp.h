#ifndef IPV6CP_HEADER
#define IPV6CP_HEADER
#define CI_IFACEID	1	/* Interface Identifier */
#define CI_COMPRESSTYPE	2	/* Compression Type     */

/* No compression types yet defined.
 *#define IPV6CP_COMP	0x004f
 */
typedef union
{
    u_int8_t e8[8];
    u_int16_t e16[4];
    u_int32_t e32[2];
} eui64_t;
#define eui64_iszero(e)		(((e).e32[0] | (e).e32[1]) == 0)
#define eui64_equals(e, o)	(((e).e32[0] == (o).e32[0]) && \
				((e).e32[1] == (o).e32[1]))
#define eui64_zero(e)		(e).e32[0] = (e).e32[1] = 0;

#define eui64_copy(s, d)	memcpy(&(d), &(s), sizeof(eui64_t))

#define eui64_magic(e)		do {			\
				(e).e32[0] = magic();	\
				(e).e32[1] = magic();	\
				(e).e8[0] &= ~2;	\
				} while (0)
#define eui64_magic_nz(x)	do {				\
				eui64_magic(x);			\
				} while (eui64_iszero(x))
#define eui64_magic_ne(x, y)	do {				\
				eui64_magic(x);			\
				} while (eui64_equals(x, y))

#define eui64_get(ll, cp)	do {				\
				eui64_copy((*cp), (ll));	\
				(cp) += sizeof(eui64_t);	\
				} while (0)

#define eui64_put(ll, cp)	do {				\
				eui64_copy((ll), (*cp));	\
				(cp) += sizeof(eui64_t);	\
				} while (0)

#define eui64_set32(e, l)	do {			\
				(e).e32[0] = 0;		\
				(e).e32[1] = htonl(l);	\
				} while (0)
#define eui64_setlo32(e, l)	eui64_set32(e, l)

typedef struct ipv6cp_options {
    int neg_ifaceid;		/* Negotiate interface identifier? */
    int req_ifaceid;		/* Ask peer to send interface identifier? */
    int accept_local;		/* accept peer's value for iface id? */
    int opt_local;		/* ourtoken set by option */
    int opt_remote;		/* histoken set by option */
    int use_ip;			/* use IP as interface identifier */
#if defined(SOL2) || defined(__linux__)
    int use_persistent;		/* use uniquely persistent value for address */
#endif /* defined(SOL2) */
    int neg_vj;			/* Van Jacobson Compression? */
    u_short vj_protocol;	/* protocol value to use in VJ option */
    eui64_t  ourid, hisid;	/* Interface identifiers */
} ipv6cp_options;


extern fsm ipv6cp_fsm[];
extern ipv6cp_options ipv6cp_wantoptions[];
extern ipv6cp_options ipv6cp_gotoptions[];
extern ipv6cp_options ipv6cp_allowoptions[];
extern ipv6cp_options ipv6cp_hisoptions[];

extern struct protent ipv6cp_protent;
#endif
