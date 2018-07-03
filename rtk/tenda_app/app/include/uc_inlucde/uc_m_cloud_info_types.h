#ifndef __UC_M_CLOUD_INFO_TYPES_H__
#define __UC_M_CLOUD_INFO_TYPES_H__

#define SET_CLOUD_INFO_X(cinfo,x) \
	((cinfo)->mask |= (1 << x))
#define HAS_CLOUD_INFO_X(cinfo,x) \
	(((cinfo)->mask & (1 << x)) == (1 << x))

typedef struct m_cloud_info_manage_en_s {
	int en;
}m_cloud_info_manage_en_t;

enum {
	_CLOUD_INFO_ACK_MANAGE_EN = 0,
};

#define SET_CLOUD_INFO_ACK_MANAGE_EN(cinfo) \
	SET_CLOUD_INFO_X(cinfo,_CLOUD_INFO_ACK_MANAGE_EN)
#define HAS_CLOUD_INFO_ACK_MANAGE_EN(cinfo) \
	HAS_CLOUD_INFO_X(cinfo,_CLOUD_INFO_ACK_MANAGE_EN)
typedef struct m_cloud_info_ack_s {
	int mask;
	int err_code;
	m_cloud_info_manage_en_t manage_en;
}m_cloud_info_ack_t;

#endif
