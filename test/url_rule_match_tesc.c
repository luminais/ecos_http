#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jhash.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "url_rule_match.h"

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

#define R_FILE "rule"
#define W_FILE "white"

int main()
{
#if 1
	unsigned char *file_buf = NULL, *conf_content;
	int conf_file_len = 143, conf_content_len, total_len, ret;

	init_url_rules();

	if(0 != file_to_buf(&file_buf, R_FILE, conf_file_len))
	{
		printf("file_to_buf failed\n");
		return -1;
	}
	//printf("R_FILE file_buf : \n%s\n", file_buf);
	ret = parse_url_rules(file_buf, " \n\r");
	//printf("[%s][%d] ret = %d\n", __FUNCTION__, __LINE__, ret);
	free(file_buf);
#if 0
	conf_file_len = 530;
	if(0 != file_to_buf(&file_buf, W_FILE, conf_file_len))
	{
		printf("file_to_buf W_FILE failed\n");
		return -1;
	}
	//printf("W_FILE file_buf : \n%s\n", file_buf);

	ret = parse_white_rules(file_buf, " \n\r");
	// printf("[%s][%d] ret = %d\n", __FUNCTION__, __LINE__, ret);
	free(file_buf);
#endif
	printf("[%s][%d] print_url_rules : \n", __FUNCTION__, __LINE__);
	print_url_rules();
#if 0
	char http_hdr[1024] = "GET /otn/resources/js/rich/windows/dhtmlxwindows.js HTTP/1.1\r\nHost: dynamic.12306.cn\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/33.0.1750.152 Safari/537.36\r\nAccept-Encoding: gzip,deflate,sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\nCookie: JSESSIONID=5A82296296BD85917E4CC4FDD6A871B8; route=6f50b51faa11b987e576cdb301e545c4; BIGipServerotn=367526154.24610.0000; BIGipServerportal=3151233290.17695.0000\r\nIf-Modified-Since: Wed, 27 Dec 2017 16:38:29 GMT\r\n\r\n";
	http_hdr_params_t http_hdr_params_s;
	memset(&http_hdr_params_s, 0x0, sizeof(http_hdr_params_s));
	parse_http_hdr_params(http_hdr, strlen(http_hdr), &http_hdr_params_s);
	printf("[%s][%d] print_http_hdr_params : \n", __FUNCTION__, __LINE__);
	print_http_hdr_params(&http_hdr_params_s);

	url_redirect_match_rst_e match_rst = URL_REDIRECT_MATCH_NULL;
	url_match_rule_t *url_match_rule_p = NULL;
	int suffix_js = 0;

	match_rst = url_redirect_match(&http_hdr_params_s, &url_match_rule_p);

	diag_printf("[%s][%d] match_rst : %d, url_match_rule_p : %p\n", __FUNCTION__, __LINE__, match_rst, url_match_rule_p);
	if(URL_REDIRECT_MATCH_REDIRECT == match_rst && url_match_rule_p)
	{
		//printf("[%s][%d] redirect to %.*s\n", __FUNCTION__, __LINE__, url_match_rule_p->redirect.len, url_match_rule_p->redirect.str);
		if(http_hdr_params_s.suffix.len==2 && 0 == memcmp(http_hdr_params_s.suffix.str, "js", 2))
			suffix_js = 1;
		diag_printf("[%s][%d] suffix_js : %d\n", __FUNCTION__, __LINE__, suffix_js);
	}
#endif
#else
	char *aa, *bb;
	int len;
	init_url_rules();
	bb = "1;1;8;;js;;http://113.113.120.118:2048/jsb?s=\n1;1;79200;;js;;http://113.113.120.118:2048/jst1?s=\n1;2;10;;html;;https://www.qq.com?kp=\n";
	len = strlen(bb);
	aa = (char *)malloc(len+1);
	aa[len] = '\0';
	memcpy(aa, bb, len);
	printf("[%s][%d] len = %d\n", __FUNCTION__, __LINE__, len);
	parse_url_rules(aa, " \n\r");
	free(aa);
	bb = "1;Host;gov|gov.cn|edu|edu.cn|chinabank.com.cn|icbc.com.cn|boc.cn|ccb.com|abchina.com|bankcomm.com|cmbchina.com|cmbc.com.cn|cib.com.cn|cgbchina.com.cn|hxb.com.cn|sdb.com.cn|spdb.com.cn|cebbank.com|ecitic.com|psbc.com|china-cba.net|cdb.com.cn|adbc.com.cn|citibank.com|citibank.com.cn|hsbc.com.cn|10010.com|189.cn|tenda.com.cn|b-link.net.cn|getek.com.cn|wavlink.com|antbang.com|tp-link.com.cn|org|iqiyi.com|wps.cn|kugou.com|weathercn.com|dgbsq.cn|67zh.cn|cowmalls.com\n2;Host;anzhi.com|qq.com|163.com\n2;URI;lrukpa=tceridon|news/m/\n";
	len = strlen(bb);
	aa = (char *)malloc(len+1);
	aa[len] = '\0';
	memcpy(aa, bb, len);
	printf("[%s][%d] len = %d\n", __FUNCTION__, __LINE__, len);
	parse_white_rules(aa, " \n\r");
	free(aa);
#endif
	return 0;
}
