/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Address Resolution Protocol		File: net_arp.c
    *  
    *  This module implements RFC826, the Address Resolution Protocol.
    *  
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */



#include "lib_types.h"
#include "lib_string.h"
#include "lib_queue.h"
#include "lib_malloc.h"
#include "lib_printf.h"

#include "net_ebuf.h"
#include "net_ether.h"

#include "net_ip.h"
#include "net_ip_internal.h"

#include "cfe_iocb.h"
#include "cfe_devfuncs.h"
#include "cfe_ioctl.h"
#include "cfe_timer.h"
#include "cfe_error.h"
#include "cfe.h"
#include "cfe_flashimage.h"

#include "trxhdr.h"
#include <bcmutils.h>

#include "net_upgrade.h"

#define UPGRADE_CONTINUE  1
#define UPGRADE_SUCCESS   2
#define TRX_CHECK_ERR  0
#define TRX_CHECK_SUC  1

uint8_t  dst_hwaddr[ENET_ADDR_LEN];
static struct upgrade_dat supgrade_data;
static struct upgrade_dat *upgrade_data = &supgrade_data;
static ebuf_t *ctrlbuf;

static int tenda_write_flash(char *flashdev, uint8_t *load_addr, int len);
static int trx_headr_check( uint8_t *load_addr, int len);

static int upgrade_suc_flag = UPGRADE_CONTINUE;
int g_upgrade_flag = 0;
int g_upgrade_delay_time = 0;

/*  *********************************************************************
 ********************************************************************* */

static int upgrade_rx_query(ebuf_t *buf, struct eth_pack_data  *pack_data)
{
	int ret=0;
	uint8_t *data_addr = NULL;

	ebuf_prepend(buf,40);
	pack_data->rq_type = ACK;
	memset(pack_data->product_data.data,0,dataLen);
	memset(pack_data->product_data.version,0,36);
	pack_data->product_data.dataLength = 0;	
	ebuf_put_bytes(buf,pack_data->flag,32);
	ebuf_put_u16_be(buf,pack_data->index);
	ebuf_put_u16_be(buf,pack_data->rq_type);
	ebuf_put_u16_be(buf,pack_data->product_data.fileFlag);
	ebuf_put_u16_be(buf,pack_data->product_data.restoreFlag);	
	ebuf_put_bytes(buf,pack_data->product_data.version,36);
	ebuf_put_bytes(buf,pack_data->product_data.data,dataLen);
	ebuf_put_u16_be(buf,pack_data->product_data.dataLength);
	ebuf_seek(buf,-(40+38+dataLen));

	//ctrldata = pack_data;
    /* adjust pointer and add in the header length */    
	ret = eth_send(buf,(uint8_t *) upgrade_mac);
	//if(ret != 0){
	//		printf("eth send error!:%d\n",ret);
	//}
	
	/*init the upgrade data struct*/
	/*restore to default flag*/
	upgrade_data->restoreFlag =  pack_data->product_data.restoreFlag;
	/*the len of the upgrade data we get!*/
	upgrade_data->offset = 0;
	if (upgrade_data->load_addr == NULL) {
		upgrade_data->load_limit = 0;
		ui_get_flash_buf(&data_addr, &upgrade_data->load_limit);
		upgrade_data->load_addr = (char *)data_addr;
	}

	eth_free(buf);
	return 0;
}



/*  *********************************************************************
    *  upgrade_rx_callback(buf,ref)
    *  
    *  Callback for ARP protocol packets.  This routine is called
    *  by the datalink layer when we receive an ARP packet.  Parse
    *  the packet and call any packet-specific processing routines
    *  
    *  Input parameters: 
    *  	   buf - ebuf that we received
    *  	   ref - reference data when we opened the port.  This is
    *  	         our IP information structure
    *  	   
    *  Return value:
    *  	   ETH_DROP or ETH_KEEP.
    ********************************************************************* */

static int upgrade_rx_callback(ebuf_t *buf,void *ref)
{
 	static int UPG_flag = 1; 
 	struct eth_pack_data reply_pack;
	struct eth_pack_data  *pack_data = &reply_pack; 
	char *uploadStr = pack_data->flag;
	ebuf_t *send_buf = buf;
	if(upgrade_suc_flag == UPGRADE_CONTINUE)
	{
		ebuf_get_bytes(buf,pack_data->flag,32);
		ebuf_get_u16_be(buf,pack_data->index);
		ebuf_get_u16_be(buf,pack_data->rq_type);
		ebuf_get_u16_be(buf,pack_data->product_data.fileFlag);
		ebuf_get_u16_be(buf,pack_data->product_data.restoreFlag);
		if(UPG_flag == 1 && (!strcmp(uploadStr,TENDA_UPGRADE)) && pack_data->rq_type == UPG)
		{		 
			//printf("Send back ACK msg:%s\n",uploadStr);
			upgrade_rx_query(send_buf,pack_data);	 	
			g_upgrade_delay_time = 2;
			UPG_flag = 2;
		}
	}
	KFREE(pack_data);
	KFREE(uploadStr);
	return 0;

}


/*  *********************************************************************
    *  _upgrade_init(ipi)
    *  
    *  Initialize the upgtade layer [internal routine]
    *  
    *  Input parameters: 
    *  	   ipi - IP information
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else error code
    ********************************************************************* */

int _upgrade_init(ip_info_t *ipi)
{
    int8_t upgradeproto[2];
  	  /*
     * Open the Ethernet portal for ARP packets
     */

    upgradeproto[0] = (PROTOSPACE_UPGRADE >> 8) & 0xFF;
    upgradeproto[1] = (PROTOSPACE_UPGRADE & 0xFF);
    ipi->arp_port = eth_open(ipi->eth_info,ETH_PTYPE_DIX,upgradeproto,upgrade_rx_callback,ipi);

    /*
     * Remember our hardware address
     */
  	eth_gethwaddr(ipi->eth_info,dst_hwaddr);
	ctrlbuf = eth_alloc(ipi->eth_info,ipi->arp_port);
	if(!ctrlbuf) 
		printf("Ctrl buf init error\n");
	printf("Mac:%x-%x-%x-%x-%x-%x \n",dst_hwaddr[0],dst_hwaddr[1],dst_hwaddr[2],dst_hwaddr[3],dst_hwaddr[4],dst_hwaddr[5]);
       return 0;

}

static int upgrade_suc_err_query(ebuf_t *buf,struct eth_pack_data  *pack_data,int flag)
{
	int ret = 0;
	
	pack_data->rq_type = flag;
	pack_data->product_data.fileFlag = 0;
	pack_data->product_data.restoreFlag = 0;
	pack_data->index = 0;
	memset(pack_data->flag,0,32);
	memcpy(pack_data->flag,TENDA_DATA,sizeof(TENDA_DATA));
	memset(pack_data->product_data.data,0,dataLen);
	pack_data->product_data.dataLength = 0;	
	
	ebuf_append_bytes(buf,pack_data->flag,32);
	ebuf_append_u16_be(buf,pack_data->index);
	ebuf_append_u16_be(buf,pack_data->rq_type);
	ebuf_append_u16_be(buf,pack_data->product_data.fileFlag);
	ebuf_append_u16_be(buf,pack_data->product_data.restoreFlag);
	ebuf_append_bytes(buf,pack_data->product_data.version,36);
	ebuf_append_bytes(buf,pack_data->product_data.data,dataLen);
	ebuf_append_u16_be(buf,pack_data->product_data.dataLength);
	
	ret = eth_send(buf,(uint8_t *) upgrade_mac);
	eth_free(buf);
	return 0;
}

/*  *********************************************************************
    *  _upgreade_data_init(ipi)
    *  
    *  Initialize the ARP layer [internal routine]
    *  
    *  Input parameters: 
    *  	   ipi - IP information
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else error code
    ********************************************************************* */


static int upgrade_data_rx_query(ebuf_t *buf,struct eth_pack_data  *pack_data)
{
	int ret=0;
	
	ebuf_prepend(buf,1102);
	pack_data->rq_type = ACK;
	memset(pack_data->product_data.data,0,dataLen);
	pack_data->product_data.dataLength = 0;	
	ebuf_put_bytes(buf,pack_data->flag,32);
	ebuf_put_u16_be(buf,pack_data->index);
	ebuf_put_u16_be(buf,pack_data->rq_type);
	ebuf_put_u16_be(buf,pack_data->product_data.fileFlag);
	ebuf_put_u16_be(buf,pack_data->product_data.restoreFlag);
	ebuf_put_bytes(buf,pack_data->product_data.version,36);
	ebuf_put_bytes(buf,pack_data->product_data.data,dataLen);
	ebuf_put_u16_be(buf,pack_data->product_data.dataLength);
    /* adjust pointer and add in the header length */  
	ebuf_seek(buf,-(40+38+dataLen));

	ret = eth_send(buf,(uint8_t *) upgrade_mac);
	// if(ret != 0){
	// 		printf("0x5252 send error!:%d\n",ret);
	// 	}
	eth_free(buf);
	return 0;
}



/*  *********************************************************************
    *  upgrade_rx_callback(buf,ref)
    *  
    *  Callback for ARP protocol packets.  This routine is called
    *  by the datalink layer when we receive an ARP packet.  Parse
    *  the packet and call any packet-specific processing routines
    *  
    *  Input parameters: 
    *  	   buf - ebuf that we received
    *  	   ref - reference data when we opened the port.  This is
    *  	         our IP information structure
    *  	   
    *  Return value:
    *  	   ETH_DROP or ETH_KEEP.
    ********************************************************************* */

static int upgrade_data_rx_callback(ebuf_t *buf,void *ref)
{
 	static int UPG_flag = 1; 
 	struct eth_pack_data reply_pack;
	struct eth_pack_data  *pack_data = &reply_pack; 
	char *uploadStr = pack_data->flag;
	ebuf_t *send_buf = buf;
	static int index_num = 0; 


	if(upgrade_suc_flag == UPGRADE_CONTINUE)
	{
#ifdef CFG_NFLASH
		char flashdev[12];
		ui_get_trx_flashdev(flashdev);
#else
		char *flashdev = "flash1.trx";
#endif
		ebuf_get_bytes(buf,pack_data->flag,32);
		ebuf_get_u16_be(buf,pack_data->index);
		ebuf_get_u16_be(buf,pack_data->rq_type);
		ebuf_get_u16_be(buf,pack_data->product_data.fileFlag);
		ebuf_get_u16_be(buf,pack_data->product_data.restoreFlag);
		ebuf_get_bytes(buf,pack_data->product_data.version,36);
		ebuf_get_bytes(buf,pack_data->product_data.data,dataLen);
		ebuf_get_u16_be(buf,pack_data->product_data.dataLength);

		//printf(":%s type:%d \n",uploadStr,pack_data->rq_type);
		if(pack_data->rq_type == SUC)
		{
			printf("Tenda upgrade success !\n");
			upgrade_suc_flag = UPGRADE_SUCCESS;
		}
		/*when get the type suc ,the board should not delay time to start*/
		else if(index_num == 0)
		{
			printf("Get image data now!\n");
			/*lengthen the boot wait time*/
			g_upgrade_flag = 1;
		}
		if(UPG_flag == 1&&(!strcmp(uploadStr,TENDA_DATA))&&pack_data->rq_type == DAT)
		{
			if(pack_data->index == (index_num + 1))
			{
				printf(".");
				/*copy the upgrade data for the pack data*/
				memcpy(upgrade_data->load_addr+upgrade_data->offset,pack_data->product_data.data,pack_data->product_data.dataLength);
				/*change the len of the upgrade data*/
				upgrade_data->offset += pack_data->product_data.dataLength;
				index_num ++;
				/*if this is the last pack of upgrade data, set the flag*/
				if(pack_data->product_data.fileFlag == 2)
				{
					UPG_flag = 2;
				}
				
			}
			pack_data->index = index_num;
			upgrade_data_rx_query(send_buf,pack_data);	 	
		}
		if(2 == UPG_flag)
		{
			/*check the headr of the image we get*/
			if(trx_headr_check((uint8_t *)upgrade_data->load_addr, upgrade_data->offset) == TRX_CHECK_SUC)

			{
				printf("\nCheck the upgrade image we get is success!\n");					
				tenda_write_flash(flashdev, (uint8_t *)upgrade_data->load_addr, upgrade_data->offset);
				upgrade_suc_err_query(ctrlbuf,pack_data,SUC);
				printf("Reboot now ...\n");
				ui_docommands("reboot");
			}
			else 
			{
				printf("\nCheck the upgrade image we get is Error!\n");
				upgrade_suc_err_query(ctrlbuf,pack_data,ERR);
				g_upgrade_flag = 0;
			}

		}	
	}
	KFREE(pack_data);
	KFREE(uploadStr);
	return 0;

}


int _upgrade_data_init(ip_info_t *ipi)
{
    int8_t upgradeproto[2];
	/*
     * Open the Ethernet portal for ARP packets
     */

    upgradeproto[0] = (PROTOSPACE_UPGRADE_DATA>> 8) & 0xFF;
    upgradeproto[1] = (PROTOSPACE_UPGRADE_DATA& 0xFF);
    ipi->arp_port = eth_open(ipi->eth_info,ETH_PTYPE_DIX,upgradeproto,upgrade_data_rx_callback,ipi);

   /*
     * Remember our hardware address
     */
    eth_gethwaddr(ipi->eth_info,dst_hwaddr);
 	//printf("Mac:%x-%x-%x-%x-%x-%x \n",dst_hwaddr[0],dst_hwaddr[1],dst_hwaddr[2],dst_hwaddr[3],dst_hwaddr[4],dst_hwaddr[5]);
    return 0;

}



static int
tenda_write_flash(char *flashdev, uint8_t *load_addr, int len)
{
	int devtype;
	int copysize;
	flash_info_t flashinfo;
	int res;
	int fh;
	int retlen;
	int offset = 0;
	int noerase = 0;
	int amtcopy;

	/*
	 * Make sure it's a flash device.
	 */
	res = cfe_getdevinfo(flashdev);
	if (res < 0) {
		xprintf("Could not find flash device '%s'\n", flashdev);
		return -1;
	}

	devtype = res & CFE_DEV_MASK;

	copysize = len;
	if (0 == copysize)
		return 0;		/* 0 bytes, don't flash */

	/*
	 * Open the destination flash device.
	 */
	fh = cfe_open(flashdev);
	if (fh < 0) {
		xprintf("Could not open device '%s'\n", flashdev);
		return CFE_ERR_DEVNOTFOUND;
	}

	if (cfe_ioctl(fh, IOCTL_FLASH_GETINFO, (unsigned char *)&flashinfo,
		sizeof(flash_info_t), &res, 0) == 0) {
		/* Truncate write if source size is greater than flash size */
		if ((copysize + offset) > flashinfo.flash_size)
			copysize = flashinfo.flash_size - offset;
	}

	/*
	 * If overwriting the boot flash, we need to use the special IOCTL
	 * that will force a reboot after writing the flash.
	 */
	if (flashinfo.flash_flags & FLASH_FLAG_INUSE) {
#if CFG_EMBEDDED_PIC
		xprintf(
		"\n\n** DO NOT TURN OFF YOUR MACHINE UNTIL THE FLASH UPDATE COMPLETES!! **\n\n");
#else
#if CFG_NETWORK
		if (net_getparam(NET_DEVNAME)) {
			xprintf("Closing network.\n");
			net_uninit();
		}
#endif /* CFG_NETWORK */
		xprintf("Rewriting boot flash device '%s'\n", flashdev);
		xprintf("\n\n**DO NOT TURN OFF YOUR MACHINE UNTIL IT REBOOTS!**\n\n");
		cfe_ioctl(fh, IOCTL_FLASH_WRITE_ALL, load_addr, copysize, &retlen, 0);

		/* should not return */
		return CFE_ERR;
#endif	/* EMBEDDED_PIC */
	}

	/*
	 * Otherwise: it's not the flash we're using right
	 * now, so we can be more verbose about things, and
	 * more importantly, we can return to the command
	 * prompt without rebooting!
	 */

	/*
	 * Erase the flash, if the device requires it.  Our new flash
	 * driver does the copy/merge/erase for us.
	 */
	if (!noerase) {
		if ((devtype == CFE_DEV_FLASH) && !(flashinfo.flash_flags & FLASH_FLAG_NOERASE)) 
		{
			flash_range_t range;
			range.range_base = offset;
			range.range_length = copysize;
			xprintf("Erasing flash...");
			if (cfe_ioctl(fh, IOCTL_FLASH_ERASE_RANGE, (uint8_t *)&range,
				sizeof(range), NULL, 0) != 0) {
				xprintf("Failed to erase the flash\n");
				cfe_close(fh);
				return CFE_ERR_IOERR;
			}
		}
	}

	/*
	 * Program the flash
	 */
	xprintf("Programming...");

	amtcopy = cfe_writeblk(fh, offset, load_addr, copysize);
	if (copysize == amtcopy) {
		xprintf("done. %d bytes written\n", amtcopy);
		res = 0;
	}
	else {
		xprintf("Failed. %d bytes written\n", amtcopy);
		res = CFE_ERR_IOERR;
	}

	/*
	 * done!
	 */
	cfe_close(fh);
	return res;
}

int trx_headr_check( uint8_t *load_addr, int len)
{
	struct trx_header trx;
	uint32 crc;
	int crc_offset;
	memcpy(&trx,load_addr,sizeof(struct trx_header));
	//printf("\nTRX:\n");
	//printf("mgc:0x%x\n",trx.magic);
	//printf("len:0x%x 0x%x\n",trx.len,len);
	//printf("crc:0x%x\n",trx.crc32);
	if(trx.magic != TRX_MAGIC)
	{
		return TRX_CHECK_ERR;
	}
	if(trx.len != len)
	{
		return TRX_CHECK_ERR;
	}
	/* Checksum over header */
	crc_offset = sizeof(trx.magic) + sizeof(trx.len) +sizeof(trx.crc32);
	crc = CRC32_INIT_VALUE;
	crc = hndcrc32((uint8 *)(load_addr+crc_offset),len-crc_offset ,crc);
	//printf("crc:get:0x%x\n",crc);
	/* Verify checksum */
	if (trx.crc32 != crc)
	{
		return TRX_CHECK_ERR;
	}
	
	return TRX_CHECK_SUC;
}

