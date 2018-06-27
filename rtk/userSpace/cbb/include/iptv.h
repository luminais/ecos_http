#ifndef __IPTV_H__
#define __IPTV_H__


//vlan地区定义
#define VLAN_AREA_DEFAULT	"0"		//默认
#define VLAN_AREA_SHANGHAI	"1"		//上海地区
#define VLAN_AREA_CUSTOM	"2"		//自定义



typedef enum{
	DEFAULT = 0,
	SHANGHAI,
	CUSTOM,
}VLAN_AREA;

typedef struct iptv_info{
	int iptv_enable;
	int vlan_area;		//vlan地区 0:默认  1:上海 2:自定义
	int vlan_id;
	int vlan_sel;
}IPTV_INFO_STRUCT,*P_IPTV_INFO_STRUCT;


#endif
