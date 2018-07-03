#ifndef __HAVE_TENDA_NVRAM__
#define __HAVE_TENDA_NVRAM__

#ifndef TYPEDEF_UINT8
typedef unsigned char	uint8;
#endif

#ifndef TYPEDEF_UINT16
typedef unsigned short	uint16;
#endif

#ifndef TYPEDEF_UINT32
typedef unsigned int	uint32;
#endif

#ifndef TYPEDEF_UINT64
typedef unsigned long long uint64;
#endif

#ifndef TYPEDEF_UINTPTR
typedef unsigned int	uintptr;
#endif

#ifndef TYPEDEF_INT8
typedef signed char	int8;
#endif

#ifndef TYPEDEF_INT16
typedef signed short	int16;
#endif

#ifndef TYPEDEF_INT32
typedef signed int	int32;
#endif

#ifndef TYPEDEF_INT64
typedef signed long long int64;
#endif

/* define float32/64, float_t */

#ifndef TYPEDEF_FLOAT32
typedef float		float32;
#endif

#ifndef TYPEDEF_FLOAT64
typedef double		float64;
#endif

#define	ROUNDUP(x, y)		((((x)+((y)-1))/(y))*(y))
#ifndef ARRAYSIZE
#define ARRAYSIZE(a)		(sizeof(a)/sizeof(a[0]))
#endif


#define NVRAM_MAGIC     0x48534C46  /* 'FLSH' */
#define NVRAM_CLEAR_MAGIC   0x0
#define NVRAM_INVALID_MAGIC 0xFFFFFFFF
#define NVRAM_VERSION       1
#define NVRAM_CRC_START_POSITION    9 /* magic, len, crc8 to be skipped */
#define NVRAM_CRC_VER_MASK  0xffffff00 /* for crc_ver_init */


struct nvram_header {
	uint32 magic;
	uint32 len;
	uint32 crc_ver_init;	/* 0:7 crc, 8:15 ver, 16:31 sdram_init */
	uint32 config_refresh;	/* 0:15 sdram_config, 16:31 sdram_refresh */
	uint32 config_ncdl;	/* ncdl values for memc */
};

struct nvram_tuple {
	char *name;
	char *value;
	struct nvram_tuple *next;
};

struct nvram_mtd_info {
    char                name[16];
    int                 has_init;
    unsigned int        base_addr;
    unsigned int        mtd_size;
    struct nvram_tuple **nvram_hash;
    unsigned int        nvram_hash_size;
    struct nvram_tuple *nvram_dead;
};


#define NVRAM_MTD_NAME      "nvram"
//#define NVRAM_MTD_BASE_ADDR 0x001F8000
#define NVRAM_MTD_SIZE      0x8000
#define NVRAM_MTD_HASH_SIZE 257

/* lq修改envram的其实地址为120k的地方
目前添加5G的参数调整envram的大小为12K*/
#define ENVRAM_MTD_NAME      "envram"
#define ENVRAM_MTD_SIZE      0x3000
#define ENVRAM_MTD_BASE_ADDR 0x001C000
#define ENVRAM_MTD_HASH_SIZE 100


int tenda_get_flash_size(void);
int nvram_cmd_main(int argc,char **argv);
int nvram_init(void);
void nvram_exit(void);

char *nvram_get(const char *name);
int nvram_set(const char *name, const char *value);
int nvram_unset(const char *name);
int nvram_erase(void);
int nvram_commit(void);
void nvram_default(void);

int envram_init(int argc, char* argv[]);
int envram_get(int argc, char* argv[]);
int envram_set(int argc, char* argv[]);
int envram_unset(int argc, char* argv[]);
int envram_commit(int argc, char* argv[]);
int envram_show(int argc, char* argv[]);
int envram_erase(int argc, char *argv[]);
int envram_to_nvram(void);
void envram_default(void);
char *__envram_get(const char *name);
int __envram_set(const char *name, const char *value);

#endif
