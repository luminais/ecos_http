
#ifndef RTL865X_FDB_API_H
#define RTL865X_FDB_API_H

#define CONFIG_RTL_IVL_SUPPORT 1

#define RTL_LAN_FID								0
#if defined (CONFIG_RTL_IVL_SUPPORT)	
#define RTL_WAN_FID								1
#else
#define RTL_WAN_FID								0
#endif

#define FDB_STATIC						0x01		/* flag for FDB: process static entry only */
#define FDB_DYNAMIC					0x02		/* flag for FDB: process dynamic entry only */

#define RTL865x_L2_TYPEI			0x0001		/* Referenced by ARP/PPPoE */
#define RTL865x_L2_TYPEII			0x0002		/* Referenced by Protocol */
#define RTL865x_L2_TYPEIII			0x0004		/* Referenced by PCI/Extension Port */

#define RTL865X_FDBENTRY_TIMEOUT		0x1001		/*fdb entry time out*/
#define RTL865X_FDBENTRY_450SEC		0x1002		/*fdb entry 450s timing*/
#define RTL865X_FDBENTRY_300SEC		0x1004		/*fdb entry 300s timing*/
#define RTL865X_FDBENTRY_150SEC		0x1008		/*fdb entry 150s timing*/
#define RTL865X_FDBENTRY_SWAP			0x1010		/*fdb entry swap */

int rtl865x_layer2_init(void);
int rtl865x_layer2_reinit(void);
int rtl865x_addFDBEntry(const unsigned char *addr);
int rtl865x_delLanFDBEntry(unsigned short l2Type,  const unsigned char *addr);
void update_hw_l2table(const char *srcName,const unsigned char *addr);

#endif
