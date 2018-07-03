#ifndef _CWMP_PORTING_H_
#define _CWMP_PORTING_H_

#include "libcwmp.h"

int port_init_userdata( struct cwmp_userdata *data );
int port_update_userdata( struct cwmp_userdata *data, int is_discon );
void port_save_reboot( struct cwmp_userdata *user, int reboot_flag );
void port_factoryreset_reboot(void);

//download & upload callback functions
int port_before_download( int file_type, char *target );
int port_after_download( int file_type, char *target );
int port_before_upload( int file_type, char *target );
int port_after_upload( int file_type, char *target );

//notify_save
int port_notify_save( char *name );

int port_session_closed(struct cwmp_userdata *data);

void port_setuppid(void);

//backup & restore config
int port_backup_config( void );
int port_restore_config( int restore );

#ifdef CONFIG_BOA_WEB_E8B_CH
int isTR069(char *name);
int isINTERNET(char *name);
#endif

#endif /*#ifndef _CWMP_PORTING_H_*/
