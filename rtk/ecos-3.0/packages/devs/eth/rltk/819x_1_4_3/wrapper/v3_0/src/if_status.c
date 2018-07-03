unsigned int IfStatus = 0;
unsigned int IfChangeStatus = 0;

#if defined(CONFIG_RTL_8881A) 
#if defined(CONFIG_RTL_8881AM) || defined(CONFIG_CUTE_MAHJONG)
#include <cyg/hal/bspchip.h>
#endif
#endif

int get_if_status(unsigned int IfType)
{
	if(IfType >= 32)
		return -1;
	return ((1 << IfType) & IfStatus);	
}

int  set_if_status(unsigned int IfType,int value)
{
	if(IfType >= 32 || (value != 0 && value != 1))
		return -1;
	IfStatus = IfStatus & (~(1 << IfType)) | (value << IfType);
	return 0;
}

int get_if_change_status(unsigned int IfType)
{
	if(IfType >= 32)
        return -1;
	return ((1 << IfType) & IfChangeStatus);
}

int set_if_change_status(unsigned int IfType,int value)
{
	if(IfType >= 32 || (value != 0 && value != 1))
		return -1;
	IfChangeStatus = IfChangeStatus & (~(1 << IfType)) | (value << IfType);
	return 0;
}

#if defined(CONFIG_RTL_8881A) 
#if defined(CONFIG_RTL_8881AM) || defined(CONFIG_CUTE_MAHJONG)
int get_cmj_xdg_wan_port()
{
	int wan_port=1;	
	if ((REG32(0xB800000C) & 0xF)==0xA) {		// Check is RTL8881AM 
		REG32(0xB8000040) = (REG32(0xB8000040)&~(0x7<<7))|(0x3<<7); //Set MUX to GPIO
		REG32(BSP_PEFGH_CNR) = REG32(BSP_PEFGH_CNR) & ~(0x80); //Set E7 for  Gpio
		REG32(BSP_PEFGH_DIR) = REG32(BSP_PEFGH_DIR) & ~(0x80); //Set E7 for Input Mode
		if((REG32(BSP_PEFGH_DAT)&0x80) == 0x80) { //Pull high
			//cmj_board_use_port4 = 1;
			wan_port=4;
		}
		else {
			//cmj_board_use_port4 = 0;
			wan_port=1;			
		}		
	}		
	return wan_port;
}
#endif
#endif
