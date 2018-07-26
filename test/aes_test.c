#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "aes.h"

#define diag_printf printf

#define R_FILE "r_20180726161519"
#define W_FILE "w_20180726161519"

unsigned char lm_aes_key[16]={0xcf, 0xdc, 0x96, 0x86, 0x35, 0x32, 0x91, 0x3c, 0x92, 0x85};
aes_context my_aes_ctx;

void print_packet(unsigned char *packet, int len)
{
	int i;

	for(i=0; i<len; i++)
	{
		diag_printf("%02x ", packet[i]);
		if((i+1)%16 == 0)
			diag_printf("\n");
	}
	diag_printf("\n");
}

int file_to_buf(unsigned char **file_buf_save, char *file_path, int file_len)
{
	unsigned char *file_buf = NULL;
	int file_fd = -1, read_count = 0;

	file_buf = (unsigned char *)malloc(file_len);
	if(NULL == file_buf )
	{
		perror("file_to_buf : No memory.\n");
		goto fail_exit;
	}
	
	memset(file_buf, 0x0, file_len);

	file_fd = open(file_path, O_RDONLY);
	if(file_fd < 0)
	{
		diag_printf("[%s][%d] open file_path failed\n", __FUNCTION__, __LINE__);
		goto fail_exit;
	}
	
	read_count = read(file_fd, file_buf, file_len);
	if(read_count != file_len)
	{
		diag_printf("[%s][%d] read_count = %d, file_len = %d\n", __FUNCTION__, __LINE__, read_count, file_len);
		goto fail_exit;
	}

	close(file_fd);
	//unlink(file_path);
	//diag_printf("[%s][%d] clear_file \n", __FUNCTION__, __LINE__);
	//clear_file(file_path);
	*file_buf_save = file_buf;
	return 0;

fail_exit:
	if(NULL != file_buf)
		free(file_buf);
	if(file_fd != -1)
		close(file_fd);
	//unlink(file_path);
	//diag_printf("[%s][%d] clear_file \n", __FUNCTION__, __LINE__);
	//clear_file(file_path);
	return -1;
}
#if 1
int main()
{
	unsigned char *file_buf = NULL, *conf_content;
	int conf_file_len = 528, conf_content_len, total_len, ret;

	memset(&my_aes_ctx, 0x0, sizeof(my_aes_ctx));
	aes_setkey_dec(&my_aes_ctx, lm_aes_key, 128);

	if(0 != file_to_buf(&file_buf, W_FILE, conf_file_len))
	{
		printf("file_to_buf failed\n");
		return -1;
	}

	print_packet(file_buf, conf_file_len);

	conf_content_len = (conf_file_len/16+1)*16;
	conf_content = (unsigned char *)malloc(conf_content_len);
	if(NULL == conf_content )
	{
		diag_printf("[%s][%d] errno = %d\n", __FUNCTION__, __LINE__, errno);
		perror("[keep_get_conf] malloc failed.");
		free(file_buf);
		return -1;
	}

	printf("[%s][%d] conf_file_len = %d, conf_content_len = %d\n", __FUNCTION__, __LINE__, conf_file_len, conf_content_len);
	memset(conf_content, 0x0, conf_content_len);
	ret = aes_decrypt(&my_aes_ctx, file_buf, conf_file_len, 0, conf_content, &conf_content_len);
	if(ret == -1)
	{
		diag_printf("[%s][%d]lm_aes_decrypt failed\n", __FUNCTION__, __LINE__);
		free(file_buf);
		free(conf_content);
		return -1;
	}
	total_len = strlen(conf_content);
	diag_printf("[%s][%d]total_len = %d\n", __FUNCTION__, __LINE__, total_len);
	printf("[%s][%d] conf_content = %.*s\n", __FUNCTION__, __LINE__, total_len, conf_content);

	return 0;
}
#else
int main()
{
	char aa[64] = "qwertyuiopasdfghjklzxcvbnm";
	char dd[64] = {0};
	int dd_len = 64;
	char cc[64] = {0};
	int cc_len = 64;
	int ret;

	memset(&my_aes_ctx, 0x0, sizeof(my_aes_ctx));
	ret = aes_setkey_enc(&my_aes_ctx, lm_aes_key, 128);
	printf("[%s][%d] ret = %d\n", __FUNCTION__, __LINE__, ret);

	ret = aes_encrypt(&my_aes_ctx, aa, strlen(aa), 0, dd, &dd_len);
	printf("[%s][%d] ret = %d, dd_len = %d\n", __FUNCTION__, __LINE__, ret, dd_len);

	ret = aes_setkey_dec(&my_aes_ctx, lm_aes_key, 128);
	printf("[%s][%d] ret = %d\n", __FUNCTION__, __LINE__, ret);

	ret = aes_decrypt(&my_aes_ctx, dd, dd_len, 0, cc, &cc_len);
	printf("[%s][%d] ret = %d, cc_len = %d\n", __FUNCTION__, __LINE__, ret, cc_len);

	printf("%s\n", cc);
}
#endif
