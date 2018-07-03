#include <network.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/kernel/kapi.h>
#ifdef CYGPKG_IO_FLASH
#include <cyg/io/flash.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#include "cyg/io/eth/rltk/819x/wlan/ieee802_mib.h"
#endif
#include "sys_utility.h"
#include "apmib.h"
#include "hw_settings.h"

extern int set_mac_address(const char *interface, char *mac_address);

int vwlan_index=0;	// initially set interface index to root

/* Do checksum and verification for configuration data */
/*
static unsigned char CHECKSUM(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	sum = ~sum + 1;
	return sum;
}

static int CHECKSUM_OK(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	if (sum == 0)
		return 1;
	else
		return 0;
}
*/

char *read_hw_settings(void)
{
	int ver;
	char *buff;
	PARAM_HEADER_T hsHeader;

	// Read hw setting
	if ( rtk_flash_read((char *)&hsHeader, HW_SETTING_OFFSET, sizeof(hsHeader))==0 ) {
		printf("Read hw setting header failed!\n");
		return NULL;
	}

	if (sscanf((char *)&hsHeader.signature[TAG_LEN], "%02d", &ver) != 1)
		ver = -1;

	if ( memcmp(hsHeader.signature, HW_SETTING_HEADER_TAG, TAG_LEN) || // invalid signatur
		(ver != HW_SETTING_VER) || // version not equal to current
		(hsHeader.len < (sizeof(HW_SETTING_T)+1)) ) { // length is less than current
		printf("Invalid hw setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n", hsHeader.signature[0],
			hsHeader.signature[1], ver, hsHeader.len);
		return NULL;
	}
	if (ver > HW_SETTING_VER)
		printf("HW setting version is greater than current [f:%d, c:%d]!\n", ver, HW_SETTING_VER);
	//printf("hsHeader.len=%d\n", hsHeader.len);
	buff = calloc(1, hsHeader.len);
	if ( buff == 0 ) {
		printf("%s: Allocate buffer failed!\n", __FUNCTION__);
		return NULL;
	}

	if ( rtk_flash_read(buff, HW_SETTING_OFFSET+sizeof(hsHeader), hsHeader.len)==0 ) {
		printf("Read hw setting failed!\n");
		free(buff);
		return NULL;
	}
	if ( !CHECKSUM_OK((unsigned char *)buff, hsHeader.len) ) {
		printf("Invalid checksum of hw setting!\n");
		free(buff);
		return NULL;
	}
	return buff;
}

int modify_mac_addr_of_hw_settings(char *mac_address)
{
	PARAM_HEADER_T header;
	char *data;
	int status, offset, idx;
	unsigned char checksum;
	unsigned char *buff;
	int stat;
#ifdef CYGPKG_IO_FLASH
	cyg_flashaddr_t err_addr;
#endif
	HW_SETTING_T *hs;
	
	buff=calloc(1, 0x2100);
	if ( buff == NULL ) {
		printf("Allocate buffer failed!\n");
		return 0;
	}

	// header
	sprintf((char *)header.signature, "%s%02d", HW_SETTING_HEADER_TAG, HW_SETTING_VER);
	header.len = sizeof(HW_SETTING_T) + sizeof(checksum);

	hs = (HW_SETTING_T *)read_hw_settings();
	if (hs == NULL) {
		printf("%s failure.\n", __FUNCTION__);
		free(buff);
		return 0;
	}

	memcpy(hs->nic0Addr, mac_address, 6);
	memcpy(hs->nic1Addr, mac_address, 5);
	hs->nic1Addr[5] = mac_address[5] + NUM_WLAN_MULTIPLE_SSID;
	//wlan mac
	for (idx=0; idx<NUM_WLAN_INTERFACE; idx++) {
		memcpy(hs->wlan[idx].macAddr, mac_address, 5);
		hs->wlan[idx].macAddr[5] = mac_address[5] + idx +1;
		
		memcpy(hs->wlan[idx].macAddr1, mac_address, 5);
		hs->wlan[idx].macAddr1[5] = mac_address[5] + 2;
		
		memcpy(hs->wlan[idx].macAddr2, mac_address, 5);
		hs->wlan[idx].macAddr2[5] = mac_address[5] + 3;
		
		memcpy(hs->wlan[idx].macAddr3, mac_address, 5);
		hs->wlan[idx].macAddr3[5] = mac_address[5] + 4;
		
		memcpy(hs->wlan[idx].macAddr4, mac_address, 5);
		hs->wlan[idx].macAddr4[5] = mac_address[5] + 5;

		memcpy(hs->wlan[idx].macAddr5, mac_address, 5);
		hs->wlan[idx].macAddr5[5] = mac_address[5] + 6;
		
		memcpy(hs->wlan[idx].macAddr6, mac_address, 5);
		hs->wlan[idx].macAddr6[5] = mac_address[5] + 7;
		
		memcpy(hs->wlan[idx].macAddr7, mac_address, 5);
		hs->wlan[idx].macAddr7[5] = mac_address[5] + 8;
	}
	data = (char *)hs;
	checksum = CHECKSUM((unsigned char *)data, header.len-1);

	//erase
	#if 0
	if ((stat = cyg_flash_erase((FLASH_BASE_ADDR+HW_SETTING_OFFSET), HW_SETTING_SECTOR_LEN, &err_addr)) != CYG_FLASH_ERR_OK) {
		printf("FLASH: erase HS failed: %s\n", flash_errmsg(stat));
		free(buff);
		free(hs);
		return 0;
	}
	#endif
	//program
	memcpy(buff, &header, sizeof(header));
	memcpy(buff+sizeof(header), hs, sizeof(HW_SETTING_T));
	memcpy(buff+sizeof(header)+sizeof(HW_SETTING_T), &checksum, sizeof(checksum));
#ifdef CYGPKG_IO_FLASH
	if ((stat = cyg_flash_program((FLASH_BASE_ADDR+HW_SETTING_OFFSET), (void *)buff, (sizeof(header)+sizeof(HW_SETTING_T)+sizeof(checksum)), &err_addr)) != CYG_FLASH_ERR_OK) {
		printf("write hs header failed!\n");
		free(buff);
		free(hs);
		return 0;
	}
#endif	
	free(hs);
	
	// check if hw checksum is ok
	offset = HW_SETTING_OFFSET;
	if ( rtk_flash_read((char *)&header, offset, sizeof(header)) == 0) {
		printf("read hs header failed!\n");
		free(buff);
		return 0;
	}
	offset += sizeof(header);
	if ( rtk_flash_read((char *)buff, offset, header.len) == 0) {
		printf("read hs MIB failed!\n");
		free(buff);
		return 0;
	}
	status = CHECKSUM_OK(buff, header.len);
	if ( !status) {
		printf("hs Checksum error!\n");
		free(buff);
		return 0;
	}
	free(buff);
	return 1;
}

int modify_pin_code_of_hw_settings(char *pinbuf)
{
	PARAM_HEADER_T header;
	char *data;
	int status, offset;
	unsigned char checksum;
	unsigned char *buff;
	int stat;
#ifdef CYGPKG_IO_FLASH
	cyg_flashaddr_t err_addr;
#endif
	HW_SETTING_T *hs;
	
	buff=calloc(1, 0x2100);
	if ( buff == NULL ) {
		printf("Allocate buffer failed!\n");
		return 0;
	}

	// header
	sprintf((char *)header.signature, "%s%02d", HW_SETTING_HEADER_TAG, HW_SETTING_VER);
	header.len = sizeof(HW_SETTING_T) + sizeof(checksum);

	hs = (HW_SETTING_T *)read_hw_settings();
	if (hs == NULL) {
		printf("%s failure.\n", __FUNCTION__);
		free(buff);
		return 0;
	}

	//wsc pin
	memcpy(hs->wlan[0].wscPin, pinbuf, PIN_LEN);
	hs->wlan[0].wscPin[PIN_LEN] = '\0';

	data = (char *)hs;
	checksum = CHECKSUM((unsigned char *)data, header.len-1);

	//erase
	#if 0
	if ((stat = cyg_flash_erase((FLASH_BASE_ADDR+HW_SETTING_OFFSET), HW_SETTING_SECTOR_LEN, &err_addr)) != CYG_FLASH_ERR_OK) {
		printf("FLASH: erase HS failed: %s\n", flash_errmsg(stat));
		free(buff);
		free(hs);
		return 0;
	}
	#endif
	//program
	memcpy(buff, &header, sizeof(header));
	memcpy(buff+sizeof(header), hs, sizeof(HW_SETTING_T));
	memcpy(buff+sizeof(header)+sizeof(HW_SETTING_T), &checksum, sizeof(checksum));
#ifdef CYGPKG_IO_FLASH
	if ((stat = cyg_flash_program((FLASH_BASE_ADDR+HW_SETTING_OFFSET), (void *)buff, (sizeof(header)+sizeof(HW_SETTING_T)+sizeof(checksum)), &err_addr)) != CYG_FLASH_ERR_OK) {
		printf("write hs header failed!\n");
		free(buff);
		free(hs);
		return 0;
	}
#endif
	free(hs);
	
	// check if hw checksum is ok
	offset = HW_SETTING_OFFSET;
	if ( rtk_flash_read((char *)&header, offset, sizeof(header)) == 0) {
		printf("read hs header failed!\n");
		free(buff);
		return 0;
	}
	offset += sizeof(header);
	if ( rtk_flash_read((char *)buff, offset, header.len) == 0) {
		printf("read hs MIB failed!\n");
		free(buff);
		return 0;
	}
	status = CHECKSUM_OK(buff, header.len);
	if ( !status) {
		printf("hs Checksum error!\n");
		free(buff);
		return 0;
	}
	free(buff);
	return 1;
}

int write_hw_settings_to_default(void)
{
	PARAM_HEADER_T header;
	HW_SETTING_T hwmib;
	char *data;
	int status, offset, i, idx;
	unsigned char checksum;
	unsigned char *buff;
	int stat;
#ifdef CYGPKG_IO_FLASH
	cyg_flashaddr_t err_addr;
#endif

	buff=calloc(1, 0x2100);
	if ( buff == NULL ) {
		printf("Allocate buffer failed!\n");
		return -1;
	}

	// write hw setting
	sprintf((char *)header.signature, "%s%02d", HW_SETTING_HEADER_TAG, HW_SETTING_VER);
	header.len = sizeof(hwmib) + sizeof(checksum);

	memset((char *)&hwmib, '\0', sizeof(hwmib));
	//hwmib.boardVer = 1; // 2T2R
	hwmib.boardVer = 2;   // 1T1R
	//hwmib.boardVer = 3;   // 1T2R
	memcpy(hwmib.nic0Addr, "\x0\xe0\x4c\x81\x96\xc1", 6);
	memcpy(hwmib.nic1Addr, "\x0\xe0\x4c\x81\x96", 5);
	hwmib.nic1Addr[5] = 0xc1 + NUM_WLAN_MULTIPLE_SSID;

	// set RF parameters
	for (idx=0; idx<NUM_WLAN_INTERFACE; idx++) {
		memcpy(hwmib.wlan[idx].macAddr, "\x0\xe0\x4c\x81\x96", 5);
		hwmib.wlan[idx].macAddr[5] = 0xc1 + idx;
		
		memcpy(hwmib.wlan[idx].macAddr1, "\x0\xe0\x4c\x81\x96", 5);
		hwmib.wlan[idx].macAddr1[5] = 0xc1 + 1;
		
		memcpy(hwmib.wlan[idx].macAddr2, "\x0\xe0\x4c\x81\x96", 5);
		hwmib.wlan[idx].macAddr2[5] = 0xc1 + 2;
		
		memcpy(hwmib.wlan[idx].macAddr3, "\x0\xe0\x4c\x81\x96", 5);
		hwmib.wlan[idx].macAddr3[5] = 0xc1 + 3;
		
		memcpy(hwmib.wlan[idx].macAddr4, "\x0\xe0\x4c\x81\x96", 5);
		hwmib.wlan[idx].macAddr4[5] = 0xc1 + 4;

		memcpy(hwmib.wlan[idx].macAddr5, "\x0\xe0\x4c\x81\x96", 5);
		hwmib.wlan[idx].macAddr5[5] = 0xc1 + 5;
		
		memcpy(hwmib.wlan[idx].macAddr6, "\x0\xe0\x4c\x81\x96", 5);
		hwmib.wlan[idx].macAddr6[5] = 0xc1 + 6;
		
		memcpy(hwmib.wlan[idx].macAddr7, "\x0\xe0\x4c\x81\x96", 5);
		hwmib.wlan[idx].macAddr7[5] = 0xc1 + 7;
		hwmib.wlan[idx].regDomain = FCC;
		hwmib.wlan[idx].rfType = 10;
		hwmib.wlan[idx].xCap = 0;
		hwmib.wlan[idx].Ther = 0;
#ifdef CONFIG_RTL_8881A_SELECTIVE
		hwmib.wlan[idx].xCap2 = 0;
		hwmib.wlan[idx].Ther2 = 0;
#endif
		
		for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
			hwmib.wlan[idx].pwrlevelCCK_A[i] = 0;
			
		for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
			hwmib.wlan[idx].pwrlevelCCK_B[i] = 0;
			
		for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
			hwmib.wlan[idx].pwrlevelHT40_1S_A[i] = 0;	
			
		for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
			hwmib.wlan[idx].pwrlevelHT40_1S_B[i] = 0;	
			
		for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
			hwmib.wlan[idx].pwrdiffHT40_2S[i] = 0;	
			
		for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
			hwmib.wlan[idx].pwrdiffHT20[i] = 0;	
			
		for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
			hwmib.wlan[idx].pwrdiffOFDM[i] = 0;	
			
		hwmib.wlan[idx].TSSI1 = 0;
		hwmib.wlan[idx].TSSI2 = 0;
		for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++)
			hwmib.wlan[idx].pwrlevel5GHT40_1S_A[i] = 0;
		for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++)
			hwmib.wlan[idx].pwrlevel5GHT40_1S_B[i] = 0;
		for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++)
			hwmib.wlan[idx].pwrdiff5GHT40_2S[i] = 0;		
		for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++)
			hwmib.wlan[idx].pwrdiff5GHT20[i] = 0;
		for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++)
			hwmib.wlan[idx].pwrdiff5GOFDM[i] = 0;	
		hwmib.wlan[idx].ledType = 11;
	}
	data = (char *)&hwmib;
	checksum = CHECKSUM((unsigned char *)data, header.len-1);

	//erase
	#if 0
	if ((stat = cyg_flash_erase((FLASH_BASE_ADDR+HW_SETTING_OFFSET), HW_SETTING_SECTOR_LEN, &err_addr)) != CYG_FLASH_ERR_OK) {
		printf("FLASH: erase HS failed: %s\n", flash_errmsg(stat));
		free(buff);
		return -1;
	}
	#endif
	//program
	memcpy(buff, &header, sizeof(header));
	memcpy(buff+sizeof(header), &hwmib, sizeof(hwmib));
	memcpy(buff+sizeof(header)+sizeof(hwmib), &checksum, sizeof(checksum));
#ifdef CYGPKG_IO_FLASH
	if ((stat = cyg_flash_program((FLASH_BASE_ADDR+HW_SETTING_OFFSET), (void *)buff, (sizeof(header)+sizeof(hwmib)+sizeof(checksum)), &err_addr)) != CYG_FLASH_ERR_OK) {
		printf("write hs header failed!\n");
		free(buff);
		return -1;
	}
#endif
	// check if hw checksum is ok
	offset = HW_SETTING_OFFSET;
	if ( rtk_flash_read((char *)&header, offset, sizeof(header)) == 0) {
		printf("read hs header failed!\n");
		free(buff);
		return -1;
	}
	offset += sizeof(header);
	if ( rtk_flash_read((char *)buff, offset, header.len) == 0) {
		printf("read hs MIB failed!\n");
		free(buff);
		return -1;
	}
	status = CHECKSUM_OK(buff, header.len);
	if ( !status) {
		printf("hs Checksum error!\n");
		free(buff);
		return -1;
	}
	free(buff);
	return 0;
}

void print_hw_settings(HW_SETTING_T *hs)
{
	int i, j;

	printf("boardVer: 0x%02x\n",hs->boardVer);
	printf("nic0Addr: %02x-%02x-%02x-%02x-%02x-%02x\n",
		hs->nic0Addr[0], hs->nic0Addr[1], hs->nic0Addr[2],
		hs->nic0Addr[3], hs->nic0Addr[4], hs->nic0Addr[5]);
	printf("nic1Addr: %02x-%02x-%02x-%02x-%02x-%02x\n",
		hs->nic1Addr[0], hs->nic1Addr[1], hs->nic1Addr[2],
		hs->nic1Addr[3], hs->nic1Addr[4], hs->nic1Addr[5]);

	for (i=0; i<NUM_WLAN_INTERFACE; i++) {
		printf("wlan[%d]:\n", i);
		printf("\tmacAddr:  %02x-%02x-%02x-%02x-%02x-%02x\n",
			hs->wlan[0].macAddr[0], hs->wlan[0].macAddr[1], hs->wlan[0].macAddr[2],
			hs->wlan[0].macAddr[3], hs->wlan[0].macAddr[4], hs->wlan[0].macAddr[5]);
		printf("\tmacAddr1: %02x-%02x-%02x-%02x-%02x-%02x\n",
			hs->wlan[0].macAddr1[0], hs->wlan[0].macAddr1[1], hs->wlan[0].macAddr1[2],
			hs->wlan[0].macAddr1[3], hs->wlan[0].macAddr1[4], hs->wlan[0].macAddr1[5]);
		printf("\tmacAddr2: %02x-%02x-%02x-%02x-%02x-%02x\n",
			hs->wlan[0].macAddr2[0], hs->wlan[0].macAddr2[1], hs->wlan[0].macAddr2[2],
			hs->wlan[0].macAddr2[3], hs->wlan[0].macAddr2[4], hs->wlan[0].macAddr2[5]);
		printf("\tmacAddr3: %02x-%02x-%02x-%02x-%02x-%02x\n",
			hs->wlan[0].macAddr3[0], hs->wlan[0].macAddr3[1], hs->wlan[0].macAddr3[2],
			hs->wlan[0].macAddr3[3], hs->wlan[0].macAddr3[4], hs->wlan[0].macAddr3[5]);
		printf("\tmacAddr4: %02x-%02x-%02x-%02x-%02x-%02x\n",
			hs->wlan[0].macAddr4[0], hs->wlan[0].macAddr4[1], hs->wlan[0].macAddr4[2],
			hs->wlan[0].macAddr4[3], hs->wlan[0].macAddr4[4], hs->wlan[0].macAddr4[5]);
		printf("\tmacAddr5: %02x-%02x-%02x-%02x-%02x-%02x\n",
			hs->wlan[0].macAddr5[0], hs->wlan[0].macAddr5[1], hs->wlan[0].macAddr5[2],
			hs->wlan[0].macAddr5[3], hs->wlan[0].macAddr5[4], hs->wlan[0].macAddr5[5]);
		printf("\tmacAddr6: %02x-%02x-%02x-%02x-%02x-%02x\n",
			hs->wlan[0].macAddr6[0], hs->wlan[0].macAddr6[1], hs->wlan[0].macAddr6[2],
			hs->wlan[0].macAddr6[3], hs->wlan[0].macAddr6[4], hs->wlan[0].macAddr6[5]);
		printf("\tmacAddr7: %02x-%02x-%02x-%02x-%02x-%02x\n",
			hs->wlan[0].macAddr7[0], hs->wlan[0].macAddr7[1], hs->wlan[0].macAddr7[2],
			hs->wlan[0].macAddr7[3], hs->wlan[0].macAddr7[4], hs->wlan[0].macAddr7[5]);
		
		printf("\tpwrlevelCCK_A:     ");
		for (j=0; j<MAX_2G_CHANNEL_NUM_MIB; j++)
			printf("0x%02x ", hs->wlan[0].pwrlevelCCK_A[j]);
		printf("\n");
		printf("\tpwrlevelCCK_B:     ");
		for (j=0; j<MAX_2G_CHANNEL_NUM_MIB; j++)
			printf("0x%02x ", hs->wlan[0].pwrlevelCCK_B[j]);
		printf("\n");
		printf("\tpwrlevelHT40_1S_A: ");
		for (j=0; j<MAX_2G_CHANNEL_NUM_MIB; j++)
			printf("0x%02x ", hs->wlan[0].pwrlevelHT40_1S_A[j]);
		printf("\n");
		printf("\tpwrlevelHT40_1S_B: ");
		for (j=0; j<MAX_2G_CHANNEL_NUM_MIB; j++)
			printf("0x%02x ", hs->wlan[0].pwrlevelHT40_1S_B[j]);
		printf("\n");
		printf("\tpwrdiffHT40_2S:    ");
		for (j=0; j<MAX_2G_CHANNEL_NUM_MIB; j++)
			printf("0x%02x ", hs->wlan[0].pwrdiffHT40_2S[j]);
		printf("\n");
		printf("\tpwrdiffHT20:       ");
		for (j=0; j<MAX_2G_CHANNEL_NUM_MIB; j++)
			printf("0x%02x ", hs->wlan[0].pwrdiffHT20[j]);
		printf("\n");
		printf("\tpwrdiffOFDM:       ");
		for (j=0; j<MAX_2G_CHANNEL_NUM_MIB; j++)
			printf("0x%02x ", hs->wlan[0].pwrdiffOFDM[j]);
		printf("\n");
		
		printf("\tregDomain: 0x%02x\n",hs->wlan[0].regDomain);
		printf("\trfType: 0x%02x\n",hs->wlan[0].rfType);
		printf("\tledType: 0x%02x\n",hs->wlan[0].ledType);
		printf("\txCap: 0x%02x\n",hs->wlan[0].xCap);
		printf("\tTSSI1: 0x%02x\n",hs->wlan[0].TSSI1);
		printf("\tTSSI2: 0x%02x\n",hs->wlan[0].TSSI2);
		printf("\tTher: 0x%02x\n",hs->wlan[0].Ther);
		printf("\ttrswitch: 0x%02x\n",hs->wlan[0].trswitch);
		printf("\ttrswpape_C9: 0x%02x\n",hs->wlan[0].trswpape_C9);
		printf("\ttrswpape_CC: 0x%02x\n",hs->wlan[0].trswpape_CC);
		printf("\ttarget_pwr: 0x%02x\n",hs->wlan[0].target_pwr);
		printf("\tpa_type: 0x%02x\n",hs->wlan[0].pa_type);
#ifdef CONFIG_RTL_8881A_SELECTIVE		
		printf("\tTher2: 0x%02x\n",hs->wlan[0].Ther2);
		printf("\txCap2: 0x%02x\n",hs->wlan[0].xCap2);
#endif
		printf("\tReserved8: 0x%02x\n",hs->wlan[0].Reserved8);
		printf("\tReserved9: 0x%02x\n",hs->wlan[0].Reserved9);
		printf("\tReserved10: 0x%02x\n",hs->wlan[0].Reserved10);
		
		printf("\tpwrlevel5GHT40_1S_A:");
		for (j=0; j<MAX_5G_CHANNEL_NUM_MIB; j++) {
			if (j%16 == 0)
				printf("\n\tch%03d: ", j);
			printf("0x%02x ", hs->wlan[0].pwrlevel5GHT40_1S_A[j]);
		}
		printf("\n");
		printf("\tpwrlevel5GHT40_1S_B: ");
		for (j=0; j<MAX_5G_CHANNEL_NUM_MIB; j++) {
			if (j%16 == 0)
				printf("\n\tch%03d: ", j);
			printf("0x%02x ", hs->wlan[0].pwrlevel5GHT40_1S_B[j]);
		}
		printf("\n");
		printf("\tpwrdiff5GHT40_2S: ");
		for (j=0; j<MAX_5G_CHANNEL_NUM_MIB; j++) {
			if (j%16 == 0)
				printf("\n\tch%03d: ", j);
			printf("0x%02x ", hs->wlan[0].pwrdiff5GHT40_2S[j]);
		}
		printf("\n");
		printf("\tpwrdiff5GHT20: ");
		for (j=0; j<MAX_5G_CHANNEL_NUM_MIB; j++) {
			if (j%16 == 0)
				printf("\n\tch%03d: ", j);
			printf("0x%02x ", hs->wlan[0].pwrdiff5GHT20[j]);
		}
		printf("\n");
		printf("\tpwrdiff5GOFDM: ");
		for (j=0; j<MAX_5G_CHANNEL_NUM_MIB; j++) {
			if (j%16 == 0)
				printf("\n\tch%03d: ", j);
			printf("0x%02x ", hs->wlan[0].pwrdiff5GOFDM[j]);
		}
		printf("\n");
		
		printf("\twscPin: %s\n", hs->wlan[0].wscPin);
	}

}

void dump_hw_settings(void)
{
	HW_SETTING_T *hs;
	
	hs = (HW_SETTING_T *)read_hw_settings();
	if (hs == NULL) {
		printf("%s failure.\n", __FUNCTION__);
		return;
	}
	print_hw_settings(hs);
	free(hs);
}

#ifndef HAVE_APMIB
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
static int apply_wlan_hw_settings_to_driver(char *ifname, HW_SETTING_T *hs)
{
	struct wifi_mib *pmib;
	int intVal,retVal=0;
	unsigned char mac[6];
	int buf1_len = 1024;
	unsigned char * buf1 = malloc(buf1_len);
	if(buf1 == NULL)
	{
		printf("malloc error in file:%s;function:%s;line:%d;\n",__FILE__,__FUNCTION__,__LINE__);
        return -1;
	}
	memset(buf1, 0, buf1_len);
	int skfd;
	struct iwreq wrq, wrq_root;
	
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
		printf("Interface open failed!\n");
		free(buf1);
		return -1;
	}

	if ((pmib = (struct wifi_mib *)malloc(sizeof(struct wifi_mib))) == NULL) {
		printf("MIB buffer allocation failed!\n");
		retVal=-1;
		goto APPLY_HWSET_TO_DRIVER_END;
	}

	// Disable WLAN MAC driver and shutdown interface first
//	RunSystemCmd(NULL_FILE, "ifconfig", ifname, "down", NULL_STR);

/*
	if (vwlan_index == 0) {
		// shutdown all WDS interface
		for (i=0; i<8; i++) {
			sprintf(buf1, "ifconfig %s-wds%d down", ifname, i);
			system(buf1);
		}
	
		// kill wlan application daemon
		sprintf(buf1, "wlanapp.sh kill %s", ifname);
		system(buf1);
	}
	else { // virtual interface
		sprintf(buf1, "wlan%d", wlan_idx);		
		strncpy(wrq_root.ifr_name, buf1, IFNAMSIZ);
		if (ioctl(skfd, SIOCGIWNAME, &wrq_root) < 0) {
			printf("Root Interface open failed!\n");
			return -1;
		}		
	}
*/

#if 0	// unnecessary
	if (vwlan_index == 0) {
		apmib_get(MIB_HW_RF_TYPE, (void *)&intVal);
		if (intVal == 0) {
			printf("RF type is NULL!\n");
			return 0;
		}
	}
#endif

	// get mib from driver
	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);
	if (vwlan_index == 0) {
		if (ioctl(skfd, 0x8B42, &wrq) < 0) {
			printf("Get WLAN MIB failed!\n");
			retVal=-1;
			goto APPLY_HWSET_TO_DRIVER_END;
		}
	}
	else {
		wrq_root.u.data.pointer = (caddr_t)pmib;
		wrq_root.u.data.length = sizeof(struct wifi_mib);				
		if (ioctl(skfd, 0x8B42, &wrq_root) < 0) {
			printf("Get WLAN MIB failed!\n");
			retVal=-1;
			goto APPLY_HWSET_TO_DRIVER_END;
		}		
	}

	// check mib version
	if (pmib->mib_version != MIB_VERSION) {
		printf("WLAN MIB version mismatch!\n");
		retVal=-1;
		goto APPLY_HWSET_TO_DRIVER_END;
	}

	if (vwlan_index > 0) {	//if not root interface, clone root mib to virtual interface
		wrq.u.data.pointer = (caddr_t)pmib;
		wrq.u.data.length = sizeof(struct wifi_mib);
		if (ioctl(skfd, 0x8B43, &wrq) < 0) {
			printf("Set WLAN MIB failed!\n");
			retVal=-1;
			goto APPLY_HWSET_TO_DRIVER_END;
		}	
	}

	// Set parameters to driver
	if (vwlan_index == 0) {	
		pmib->dot11StationConfigEntry.dot11RegDomain = hs->wlan[0].regDomain;
	}

//#ifdef EN_EFUSE
#if 0
		strcpy(buf1, "HW_WLAN0_WLAN_ADDR");
		wrq.u.data.pointer = (caddr_t)buf1;
		wrq.u.data.length = strlen(buf1);
		if (ioctl(skfd, 0x8B9A, &wrq) < 0) {
			printf("get WLAN MAC Address from EFUSE failed!\n");
			return -1;
		}
		if( !string_to_hex(buf1 + 1+ strlen("HW_WLAN0_WLAN_ADDR"), mac, 12)) {
			printf("invalid MAC Address format!\n");
			return -1;
		}
#ifdef MBSSID
		if (vwlan_index > 0 && vwlan_index != NUM_VWLAN_INTERFACE) {
			mac[5] += vwlan_index;
		}		
#endif	
#else
	//apmib_get(MIB_WLAN_MAC_ADDR, (void *)mac);
	memset(mac, 0, 6);
	if (!memcmp(mac, "\x00\x00\x00\x00\x00\x00", 6)) {
#ifdef MBSSID
		if (vwlan_index > 0 && vwlan_index != NUM_VWLAN_INTERFACE) {
			switch (vwlan_index)
			{
				case 1:
					memcpy(mac, hs->wlan[0].macAddr1, 6);
					break;
				case 2:
					memcpy(mac, hs->wlan[0].macAddr2, 6);
					break;
				case 3:
					memcpy(mac, hs->wlan[0].macAddr3, 6);
					break;
				case 4:
					memcpy(mac, hs->wlan[0].macAddr4, 6);
					break;
				default:
					printf("Fail to get MAC address of VAP%d!\n", vwlan_index-1);
					retVal=0;
					goto APPLY_HWSET_TO_DRIVER_END;
			}
		}
		else
#endif
		memcpy(mac, hs->wlan[0].macAddr, 6);
	}
#endif

	// ifconfig all wlan interface when not in WISP
	// ifconfig wlan1 later interface when in WISP mode, the wlan0  will be setup in WAN interface
	/*
	apmib_get(MIB_OP_MODE, (void *)&intVal);
	apmib_get(MIB_WISP_WAN_ID, (void *)&intVal2);
	sprintf(buf1, "wlan%d", intVal2);
	*/
	intVal = 0;
	if ((intVal != 2) ||
#ifdef MBSSID
		vwlan_index > 0 ||
#endif
		strcmp(ifname, (char *)buf1)) {
		set_mac_address(ifname, (char *)mac);
		memcpy(&(pmib->dot11OperationEntry.hwaddr[0]), mac, 6);
	}

#ifdef BR_SHORTCUT	
	if (intVal == 2
#ifdef MBSSID
		&& vwlan_index == 0
#endif
	) 
		pmib->dot11OperationEntry.disable_brsc = 1;
#endif
	
	pmib->dot11OperationEntry.ledtype = hs->wlan[0].ledType;

	if (vwlan_index == 0) { // root interface	
		// set RF parameters
		pmib->dot11RFEntry.dot11RFType = hs->wlan[0].rfType;
		
		intVal = hs->boardVer;
		if (intVal == 1)
			pmib->dot11RFEntry.MIMO_TR_mode = 3;	// 2T2R
		else if(intVal == 2)
			pmib->dot11RFEntry.MIMO_TR_mode = 4; // 1T1R
		else
			pmib->dot11RFEntry.MIMO_TR_mode = 1;	// 1T2R
		
//#ifndef EN_EFUSE
#if 1
		memcpy(pmib->dot11RFEntry.pwrlevelCCK_A, hs->wlan[0].pwrlevelCCK_A, MAX_2G_CHANNEL_NUM_MIB);	
		memcpy(pmib->dot11RFEntry.pwrlevelCCK_B, hs->wlan[0].pwrlevelCCK_B, MAX_2G_CHANNEL_NUM_MIB);
		memcpy(pmib->dot11RFEntry.pwrlevelHT40_1S_A, hs->wlan[0].pwrlevelHT40_1S_A, MAX_2G_CHANNEL_NUM_MIB);
		memcpy(pmib->dot11RFEntry.pwrlevelHT40_1S_B, hs->wlan[0].pwrlevelHT40_1S_B, MAX_2G_CHANNEL_NUM_MIB);
		memcpy(pmib->dot11RFEntry.pwrdiffHT40_2S, hs->wlan[0].pwrdiffHT40_2S, MAX_2G_CHANNEL_NUM_MIB);
		memcpy(pmib->dot11RFEntry.pwrdiffHT20, hs->wlan[0].pwrdiffHT20, MAX_2G_CHANNEL_NUM_MIB);
		memcpy(pmib->dot11RFEntry.pwrdiffOFDM, hs->wlan[0].pwrdiffOFDM, MAX_2G_CHANNEL_NUM_MIB);
#endif
		pmib->dot11RFEntry.tssi1 = hs->wlan[0].TSSI1;
		pmib->dot11RFEntry.tssi2 = hs->wlan[0].TSSI2;
		pmib->dot11RFEntry.ther = hs->wlan[0].Ther;
	}

	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);
	if (ioctl(skfd, 0x8B43, &wrq) < 0) {
		printf("Set WLAN MIB failed!\n");
		retVal=-1;
		goto APPLY_HWSET_TO_DRIVER_END;
	}
APPLY_HWSET_TO_DRIVER_END:
	close(skfd);
	if(pmib)
		free(pmib);
//	RunSystemCmd(NULL_FILE, "ifconfig", ifname, "up", NULL_STR);
	free(buf1);
	return retVal;
}
#endif

int apply_hw_settings_to_driver(void)
{
	HW_SETTING_T *hs;
	
	hs = (HW_SETTING_T *)read_hw_settings();
	if (hs == NULL) {
		printf("%s failure.\n", __FUNCTION__);
		return 0;
	}

#ifndef HAVE_NOETH
#ifdef CYGHWR_NET_DRIVER_ETH0
	set_mac_address("eth0", (char *)hs->nic0Addr);
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
	set_mac_address("eth1", (char *)hs->nic1Addr);
#endif
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
	apply_wlan_hw_settings_to_driver("wlan0", hs);
#endif

	free(hs);

#ifdef HAVE_WPS
	if (!check_pin_code_of_hw_settings()) {
		// Generate WPS PIN number
		char tmpbuf[16];
		generate_pin_code(tmpbuf);
		modify_pin_code_of_hw_settings(tmpbuf);
		printf("Generated PIN = %s\n", tmpbuf);
	}
#endif

	return 1;
}
#endif
