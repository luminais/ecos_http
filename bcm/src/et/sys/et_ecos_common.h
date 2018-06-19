#ifndef __ET_ECOS_COMMON_H__
#define __ET_ECOS_COMMON_H__

bool et_chipmatch(uint vendor, uint device);
void *et_attach(void *et_info, uint vendor, uint device, uint unit, void *osh, void *base);
void et_detach(void *et_info);
void et_get_hwaddr(void *etc, char *addr);
void et_set_hwaddr(void *etc, char *addr);
void et_promisc(void *etc, bool promisc);
void et_allmulti(void *etc, bool allmulti);
int et_get_linkstate(void *etc, int port_num);
int et_get_linkspeed(void *etc, int port_num);
int et_get_duplex(void *etc, int port_num);
int et_set_speed(void *etc, int port_num, int speed);
int et_txdesc_num(void *etc);
void et_xmit(void *etc, void *p, int len);
void et_recv_update(void *etc, int len);
void et_show_errors(void *etc, struct ether_addr *src, uint16 flags);
void *et_recv_pkt(void *etc);
void et_rxfill(void *etc);
void et_txreclaim(void *etc);
bool et_chip_errors(void *etc);
void et_enable_int(void *etc);
void et_disable_int(void *etc);
bool et_chip_up(void *etc);
uint et_int_events(void *etc, bool in_isr);
void et_chip_init(void *etc);
void et_chip_reset(void *etc);
void et_chip_open(void *etc);
void et_chip_close(void *etc, int reset);
bool et_nicmode(void *etc);
uint et_etc_unit(void *etc);
uint et_core_unit(void *etc);
uint16 et_chip_phyrd(void *etc, uint phyaddr, uint reg);
void et_chip_phywr(void *etc, uint phyaddr, uint reg, uint16 val);
void et_chip_watchdog(void *etc);
void et_chip_dump(void *etc);
int et_rx_hwoffset(void);
uint et_rx_event(uint events);
uint et_tx_event(uint events);
uint et_error_event(uint events);
uint et_new_event(uint events);
bool et_qos_pkt(void *etc);
void et_qos_config(void *etc, uint val);
int et_etc_ioctl(void *etc, int cmd, void *argv);
uint16 et_etc_rxh_flags(void *etc, bcmenetrxh_t *rxh);
#ifdef BCMDBG
void et_dbg_dump(void *etc, struct bcmstrbuf *b);
#endif

#endif	/* __ET_ECOS_COMMON_H__ */
