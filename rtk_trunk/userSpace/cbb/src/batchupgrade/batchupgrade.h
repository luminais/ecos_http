
#define UPGRADE_DEBUG(fmt, args...) 		do{		/* show debug*/	\
	printf("upgrade debug->%s: %s(%d)--"fmt, __FILE__, __func__, __LINE__, ## args);	\
}while(0);

#define MAGIC_HTTPD_HEAD	"\r\n\r\n"
#define MAGIC_IMAGE_HEAD	"HDR0"

typedef enum tpi_upgrade_ret{	
	BAD_HTTPD_FORMAT,			/* 无法解析的HTTPD数据流 */
	BAD_IMAGE_FORMAT,			/* 错误的升级文件格式*/
	READ_FLASH_FAIL,				/* 无法读取FLASH分区 */
	OUT_OF_SIZE,					/* 升级文件太大 */
	UPGRADE_FAIL,					/* 升级失败 */	
	UPGRADE_OK					/* 升级成功 */
}TPI_UPGRADE_RET;

typedef enum{
	TPI_FALSE,				/* 假 */
	TPI_TRUE				/* 真 */
}TPI_BOOL;

/* 接口函数返回值类型 */
typedef enum tpi_ret{
	TPI_RET_OK = 0,						/* 成功 */
	TPI_RET_APP_RUNNING = 1,			/* 模块正在运行 */
	TPI_RET_APP_DEAD = 2,				/* 模块已经退出 */
	TPI_RET_NULL_POINTER = 1001,		/* 空指针错误 */
	TPI_RET_INVALID_PARAM = 1002,		/* 非法参数 */
	TPI_RET_ERROR = 0xff				/* 失败 */
}TPI_RET;

extern int get_flash_offset(int *offset);
extern int tenda_upload_fw1(char *stream, int offset_in,int flash_offset);
extern TPI_BOOL validate_image_format(char *stream, int httpd_offset);
extern void ifupgrade_deinit(void);
extern int batch_upgrade_start(void);
extern void cluster_free(char* data);
extern void set_all_led_blink(void);
extern void ate_set_all_led_on(void);
extern void restore_defaults(void);