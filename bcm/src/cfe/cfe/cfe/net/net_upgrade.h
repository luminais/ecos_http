#ifndef __NET_UPGREADE_H
#define __NET_UPGREADE_H

   
#define dataLen                 1024       

#define TENDA_UPGRADE	"TENDA_UPGRADE"	
#define TENDA_DATA		"TENDA_DATA"	

typedef  unsigned short u_short;


#define UPG (u_short)1 
#define DAT (u_short)2 
#define ACK (u_short)3 
#define SUC (u_short)4 
#define ERR (u_short)5


struct eth_pack_head
{
    unsigned char  dstMac[6];    
	unsigned char  srcMac[6];         
    unsigned char  ethType[2];     
};


struct product_pack_data{
	u_short fileFlag; 
	u_short restoreFlag;
	char version[36]; 
	char  data[dataLen]; 
	u_short dataLength;
};

struct eth_pack_data{
	char flag[32]; 
	u_short index; 
	u_short rq_type; 
	struct product_pack_data  product_data;

};

struct eth_transmit_pack{
  	struct eth_pack_head eth_head;
	struct eth_pack_data eth_data;
};


struct upgrade_dat{
	u_short restoreFlag;
	char *load_addr;
	unsigned int offset;
	unsigned int load_limit;
};



#endif
