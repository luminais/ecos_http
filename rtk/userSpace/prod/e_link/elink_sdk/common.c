
#include "common.h"


unsigned char g_private_key[] = "-----BEGIN RSA PRIVATE KEY-----\n"
								"MIICXAIBAAKBgQDCz9w7fTin0vJC02vb9rZpjx12IFZf5ZqMXMNsjWrqiyVUrBB0\n"
								"6pUCUTS4R9HkUOo+YEXy/F1+6/h+WBaUNGoLy9SJ2jUTtcx4fXCES618YSy0Kued\n"
								"tZwImtKedmQRN2qJh/Vppiwv6MioxSKqUJQX3c2U24bR1316vKfaoloz5QIDAQAB\n"
								"AoGAPpAwoy5A3qzBPrLVRcwCov3iMUiLVLrRGljELf1wo6hSMdIcat/XQOpBaxkt\n"
								"kAxoXeXfMPzZLeSsZi71+Vyn3VhZgW8LxHHmgIZxOpZ7YUpxAsilYXZMSzTaT5Zo\n"
								"pNx9LeM2Dhu8jBVKJdspSl2qC93DxIWaRXI4X0E8JrNkbeECQQDqd4Os+Pz04VBC\n"
								"KRfyAWfiJ0fHcfsjSaEM67eaP+/mGpdfvH6a4+i0TYMMwvjtQhbD+9eMKCJC+NPU\n"
								"R42grL9tAkEA1LQHz+EaDcp+X1V4r/VtYA/hjJTbV2TWflndD0wFt7O6ir7Ry7Jw\n"
								"84bGDW39kSV+FKFKLiqs2X+JorM7DAbjWQJAJHS8xpJYUoMOBZt6G6jYTDOrV3mD\n"
								"gPMb1XR5McSrOm38B2i5jr/NGOvMapmONuXFwGdSFnrPCPbDiQgvH0J86QJBAMXl\n"
								"pmsMBYCVSRSL4ljcIOJChFiW2qiqIkK2e01TLvKm3sVeGrXJlPuPBk/SaRwVZQPu\n"
								"uKTRgbZuyfC9jw92jSkCQG+hsDO/SyiLnAoX7LNmuo/YARWdLq9uyTcZYJKFBIl4\n"
								"HmqPAYozsMjdd7gQaIVfru1Hj4wLtV/+1zI4SKwZsME=\n"
								"-----END RSA PRIVATE KEY-----";

int _strcmp(const char *str1, const char *str2)
{
	if (!str1)
		return -(str1 != str2);

	if (!str2)
		return str1 != str2;

	return strcmp(str1, str2);
}


static unsigned int _get_seed(void)
{
	unsigned int seed = 0;
	struct stat buf;
	int fd = 0, cnt = 0;
#if 0
	if ((stat("/dev/urandom", &buf) == 0) && (buf.st_mode & S_IFCHR) != 0) {
		fd = open("/dev/urandom", O_RDONLY);

		if (fd > 0) {
			cnt = read(fd, &seed, sizeof(seed));
			close(fd);
		}
	}
#endif
	if (seed == 0 || cnt <= 0)
		seed = time(0);

	return seed;
}

int _get_random_number(int from, int to)
{
	int num = 0;

	if (to <= from)
		return from;

	srand(_get_seed());
	num = (rand() % (to - from)) + from;

	return num;
}

unsigned char *_get_random_string(int size)
{
	const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	unsigned char *result = NULL;
	unsigned int seed = 0;
	int i = 0;

	seed = _get_seed();

	result = malloc(size + 1);

	if (!result)
		return NULL;

	for (; i < size; i++)
		result[i] = chars[rand_r(&seed) % strlen(chars)];

	result[size] = '\0';

	return result;
}


int _check_msg_header(const char *buf, int size)
{
	uint32_t magic1 = 0;
	uint64_t magic2 = 0;
	uint32_t len = 0;

	if (size == ELINK_HEADER_LEN) {
		memcpy(&magic1, buf, sizeof(magic1));

		if (htonl(magic1) != ELINK_MAGIC) {
			printf("invalid magic 0x%x\n", htonl(magic1));
			return 0;
		} else {
			memcpy(&len, buf + ELINK_MAGIC_LEN, ELINK_CONTENT_LEN);
		}
	} else if (size == ELINKCC_HEADER_LEN) {
		memcpy(&magic2, buf, sizeof(magic2));

		if (htobe64(magic2) != ELINKCC_MAGIC) {
			printf("invalid magic 0x%x\n", htobe64(magic2));
			return 0;
		} else {
			memcpy(&len, buf + ELINKCC_MAGIC_LEN, ELINK_CONTENT_LEN);
		}
	}

	//ULOG_DEBUG("%s: hdr size %d, msg len is %d\n", __func__, size, len);

	return htonl(len);
}

int _check_msg_tail(const char *buf)
{
	if ((buf[0] == '\r') && (buf[1] == '\n'))
		return 1;

	printf("Invalid msg: not end with \\r\\n\n");
	return 0;
}


unsigned char *_add_elink_hdr_local(unsigned char *buf, unsigned int len)
{
	uint32_t buf_len = 0, magic = htonl(ELINK_MAGIC);
	unsigned char *newbuf = NULL;

	if (!buf) {
		printf("%s, invalid buf\n", __func__);
		return NULL;
	}

	buf_len = htonl(len);
	newbuf = malloc(ELINK_HEADER_LEN + len);

	if (newbuf) {
		memcpy(newbuf, &magic, ELINK_MAGIC_LEN);
		memcpy(newbuf + ELINK_MAGIC_LEN, &buf_len, 4);
		memcpy(newbuf + ELINK_HEADER_LEN, buf, len);
	} else {
		printf("%s, failed to malloc\n", __func__);
	}

	return newbuf;
}

unsigned char *_add_elink_hdr_remote(unsigned char *buf, unsigned int len)
{
	uint32_t buf_len = 0;
	uint64_t magic = htobe64(ELINKCC_MAGIC);
	unsigned char *newbuf = NULL;

	if (!buf) {
		printf("%s, invalid buf\n", __func__);
		return NULL;
	}

	buf_len = htonl(len + ELINKCC_TAIL_LEN); //include \r\n
	newbuf = malloc(ELINK_MSG_LEN(len));

	if (newbuf) {
		memcpy(newbuf, &magic, ELINKCC_MAGIC_LEN);
		memcpy(newbuf + ELINKCC_MAGIC_LEN, &buf_len, 4);
		memcpy(newbuf + ELINKCC_HEADER_LEN, buf, len);
		memcpy(newbuf + ELINKCC_HEADER_LEN + len, "\r\n", 2);
	} else {
		printf("%s, failed to malloc\n", __func__);
	}

	return newbuf;
}


void _bin_print(unsigned char *value, unsigned int len)
{
	int i = 0;

	for (i = 0; i < len; i++) {
		if (i && (i % 64 == 0))
			printf("\n");

		printf("%02X", value[i] & 0xff);
	}

	printf(" (%u)\n", len);

	return;
}

void _char_print(unsigned char *value, unsigned int len)
{
	int i = 0;

	for (i = 0; i < len; i++)
		printf("%c", value[i]);

	printf(" (%u)\n", len);

	return;
}

int _do_aes_cbc_crypt(unsigned char *in, int inlen, unsigned char **out, int *outlen, unsigned char *key, int do_encrypt)
{
	int tmplen = 0;
	unsigned char *buf = NULL;
	unsigned char iv[EVP_MAX_IV_LENGTH] = {0};
	EVP_CIPHER_CTX ctx;

#if ENABLE_AES_DEBUG
	printf("%s from: \n", do_encrypt ? "encrypt" : "decrypt");
	_bin_print(in, inlen);
#endif

	/* make sure key size == 16 */

	EVP_CIPHER_CTX_init(&ctx);

	EVP_CipherInit_ex(&ctx, EVP_aes_128_cbc(), NULL, key, iv, do_encrypt);

	EVP_CIPHER_CTX_set_padding(&ctx, 0);

	buf = calloc(1, inlen + 1);

	if (!buf) {
		printf("%s: %s malloc %d failed\n", __func__, do_encrypt ? "encrypt" : "decrypt", inlen + 1);
		return -1;
	}

	if (EVP_CipherUpdate(&ctx, buf, outlen, in, inlen) == 0) {
		printf("aes_128_cbc %s failed: EVP_CipherUpdate\n", do_encrypt ? "encrypt" : "decrypt");
		EVP_CIPHER_CTX_cleanup(&ctx);
		FREE(buf);
		return -1;
	}

	//printf("outlen = %d\n", *outlen);

	if (EVP_CipherFinal_ex(&ctx, buf + *outlen, &tmplen) == 0 && (tmplen > 0)) {
		printf("aes_128_cbc %s failed: EVP_CipherFinal_ex, tmplen %d\n", do_encrypt ? "encrypt" : "decrypt", tmplen);
		EVP_CIPHER_CTX_cleanup(&ctx);
		FREE(buf);
		return -1;
	}

	//printf("tmplen = %d\n", tmplen);

	*out = buf;

	*outlen += tmplen;

#if ENABLE_AES_DEBUG
	printf("to: \n");
	_bin_print(*out, *outlen);
#endif
	EVP_CIPHER_CTX_cleanup(&ctx);
	return *outlen;
}

int _do_aes_ecb_crypt(unsigned char *in, int inlen, unsigned char **out, int *outlen, unsigned char *key, int do_encrypt)
{
	int tmplen;
	unsigned char *buf = NULL;
	EVP_CIPHER_CTX ctx;

#if ENABLE_AES_DEBUG
	printf("from: \n");
	_bin_print(in, inlen);
#endif
	EVP_CIPHER_CTX_init(&ctx);

	EVP_CipherInit_ex(&ctx, EVP_aes_128_ecb(), NULL, key, NULL, do_encrypt);

	if (do_encrypt)
		buf = malloc(inlen + AES_128_BLOCK_SIZE - inlen % AES_128_BLOCK_SIZE);
	else
		buf = malloc(inlen);

	if (!buf) {
		printf("%s: %s malloc %d failed\n", __func__, do_encrypt ? "encrypt" : "decrypt", inlen);
		return -1;
	}

	if (EVP_CipherUpdate(&ctx, buf, outlen, in, inlen) == 0) {
		printf("aes_128_ecb %s failed: EVP_CipherUpdate\n", do_encrypt ? "encrypt" : "decrypt");
		EVP_CIPHER_CTX_cleanup(&ctx);
		FREE(buf);
		return -1;
	}

	//printf("outlen = %d\n", *outlen);

	if (EVP_CipherFinal_ex(&ctx, buf + *outlen, &tmplen) == 0 && (tmplen > 0)) {
		printf("aes_128_ecb %s failed: EVP_CipherFinal_ex, tmplen %d\n", do_encrypt ? "encrypt" : "decrypt", tmplen);
		EVP_CIPHER_CTX_cleanup(&ctx);
		FREE(buf);
		return -1;
	}

	//printf("tmplen = %d\n", tmplen);

	*outlen += tmplen;

	if (!do_encrypt) {
		int i = *outlen;

		//ULOG_DEBUG("%s: remove padding 0x%x (%d-%d)\n", __func__, buf[i], i, inlen);
		while (i < inlen)
			buf[i++] = '\0';
	}

	*out = buf;

#if ENABLE_AES_DEBUG
	printf("to: \n");
	_bin_print(*out, *outlen);
#endif
	EVP_CIPHER_CTX_cleanup(&ctx);
	return *outlen;
}

static void _pkcs7_pad(unsigned char *buf, int buf_len, int block_size, int block_cnt, unsigned char *padding_buf)
{
	int i = 0;
	int p = block_size - buf_len % block_size;

	for (i = 0; i < buf_len; i++)
		padding_buf[i] = buf[i];

	for (i = buf_len; i < block_size * block_cnt; i++)
		padding_buf[i] = p;
}

static int _pkcs7_undo(unsigned char *buf, int buf_len, int block_size)
{
	int i = 0;
	unsigned char p = buf[buf_len - 1];

	if (p >= block_size)
		return buf_len;

	for (i = buf_len - 1; i >= buf_len - p; i--)
		if (buf[i] != p)
			return buf_len;

	i++;
	bzero(buf + i, buf_len - i);
	return i;
}

int _do_rsa_encrypt(unsigned char *data, int data_len, unsigned char **encrypted, unsigned char *key)
{
	RSA *rsa = NULL;
	BIO *keybio = NULL;
	int ret = 0, offset = 0;
	int block_size = 0, block_cnt = 0;
	unsigned char *pad_buf = NULL;

	keybio = BIO_new_mem_buf(key, -1);

	if (keybio == NULL) {
		printf("Failed to create key BIO\n");
		return 0;
	}

	PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);

	if (rsa == NULL) {
		BIO_free(keybio);
		printf("Failed to get rsa\n");
		return 0;
	}

	block_size = RSA_size(rsa);
	block_cnt = data_len / block_size + !!(data_len % block_size);

	ret = block_size * block_cnt;
	pad_buf = (unsigned char *) malloc(ret);

	if (pad_buf == NULL) {
		printf("Failed to malloc pad_buf\n");
		ret = 0;
		goto done;
	}

	_pkcs7_pad(data, data_len, block_size, block_cnt, pad_buf);
	//_bin_print(pad_buf, ret);

	*encrypted = malloc(ret);

	if (*encrypted == NULL) {
		printf("failed to malloc %d bytes\n", block_size);
		ret = 0;
		goto done;
	}

	while (block_cnt > 0) {
		RSA_public_encrypt(block_size, pad_buf + offset, *encrypted + offset, rsa, RSA_NO_PADDING);
		offset += block_size;
		block_cnt--;
	}

	//ULOG_DEBUG("%s (%d)\n", __func__, __LINE__);
	//_bin_print(*encrypted, ret);

done:
	BIO_free(keybio);
	RSA_free(rsa);
	free(pad_buf);

	return ret;
}


int _do_rsa_decrypt(unsigned char *data, int data_len, unsigned char **decrypted)
{
	RSA *rsa = NULL;
	BIO *keybio = NULL;
	int ret = 0, offset = 0;
	int block_size = 0;

	keybio = BIO_new_mem_buf(g_private_key, -1);

	if (keybio == NULL) {
		printf("Failed to create key BIO");
		return 0;
	}

	PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);

	if (!rsa) {
		printf("Failed to get rsa\n");
		goto done;
	}

	block_size = RSA_size(rsa);

	if (data_len % block_size != 0) {
		printf("rsa data len error %d\n", data_len);
		goto done;
	}

	*decrypted = malloc(data_len);

	if (*decrypted == NULL) {
		printf("failed to malloc %d bytes\n", data_len);
		goto done;
	}

	while (offset < data_len) {
		if (RSA_private_decrypt(block_size, data + offset, *decrypted + offset, rsa, RSA_NO_PADDING) < 0) {
			printf("failed to decrypt offset %d\n", offset);
			free(*decrypted);
			goto done;
		}

		offset += block_size;
	}

	ret = _pkcs7_undo(*decrypted, data_len, block_size);

done:
	BIO_free(keybio);
	RSA_free(rsa);

	return ret;
}

int _do_b64_cmd(unsigned char *src, int srclen, unsigned char **dst, int encode)
{
	int dstlen = 0;

	if (encode)
		dstlen = B64_ENCODE_LEN(srclen);
	else
		dstlen = B64_DECODE_LEN(srclen);

	*dst = malloc(dstlen);

	if (!*dst) {
		printf("%s: malloc failed\n", __func__);
		return -1;
	}

	if (encode)
		dstlen = base64_encode(src, srclen, *dst, dstlen);
	else
		dstlen = base64_decode(src, *dst, dstlen);

	return dstlen;
}


int _get_int_json_value(cJSON *obj, char *key, int type, void *buf)
{
	cJSON *obj_value = cJSON_GetObjectItem(obj, key);

	if (!obj_value) {
		//ULOG_WARN("%s failed to get key %s, type %d\n", __func__, key, type);
		return -1;
	}

	if (json_typeof(obj_value) != type) {
		printf("Invalid JSON type for %s, expect %d, is %d\n", key, type, json_typeof(obj_value));
		return -1;
	}

	*((int *)buf) = json_integer_value(obj_value);

	return 0;
}

int _get_string_json_value(cJSON *obj, char *key, char **buf)
{
	int len = -1;
	char *temp = NULL;

	cJSON *obj_value = cJSON_GetObjectItem(obj, key);

	if (!obj_value)
		return len;

	if (json_typeof(obj_value) != cJSON_String) {
		if (json_typeof(obj_value) == cJSON_NULL) {
			*buf = NULL;
			len = 0;
		} else {
			printf("JSON type for %s is %d (not string)\n", key, json_typeof(obj_value));
		}

		return len;
	}

	temp = (char *)json_string_value(obj_value);

	if (temp) {
		len = strlen(temp);
		*buf = strdup(temp);
	} else {
		*buf = NULL;
	}

	return len;
}

int _get_sys_time(char **buf)
{
	time_t timer;
	struct tm *t_tm;
	int len = 0;

	time(&timer);
	t_tm = localtime(&timer);

	*buf = (char*)malloc(32);
	len = sprintf(*buf, "%4d%02d%02d%02d%02d%02d", t_tm->tm_year + 1900, t_tm->tm_mon + 1, t_tm->tm_mday,
					t_tm->tm_hour, t_tm->tm_min, t_tm->tm_sec);

	return len;
}

int _get_uuid_string(char **uuid)
{
	uuid_t id;
	char *pid = NULL;

	uuid_generate_random(id);

	pid = malloc(32 + 5 + 1);

	if (pid)
		uuid_unparse(id, pid);
	else
		return -1;

	*uuid = pid;
	return 0;
}


cJSON *_config_to_jobj(const char *file)
{
	FILE *fp = NULL;
	char line[1024] = {0};
	int len = 0;
	cJSON *obj = NULL;

	fp = fopen(ELINKCC_CONFIG, "r");

	if (fp) {
		if (fgets(line, sizeof line, fp)) {
			len = strlen(line);

			if (line[len] == '\n')
				line[len] = '\0';

			obj = cJSON_Parse(line);
		}

		fclose(fp);
	}

	return obj;
}

int _convert_mac(char *mac)
{
	int i = 0;
	if (mac == NULL) return -1;

	if (strlen(mac) == 17) {
		if (mac[2] != '-' && mac[2] != ':') return -1;

		if (mac[5] != '-' && mac[5] != ':') return -1;

		if (mac[8] != '-' && mac[8] != ':') return -1;

		if (mac[11] != '-' && mac[11] != ':') return -1;

		if (mac[14] != '-' && mac[14] != ':') return -1;

		mac[2] = mac[3];
		mac[3] = mac[4];
		mac[4] = mac[6];
		mac[5] = mac[7];
		mac[6] = mac[9];
		mac[7] = mac[10];
		mac[8] = mac[12];
		mac[9] = mac[13];
		mac[10] = mac[15];
		mac[11] = mac[16];
		mac[12] = '\0';
	}

	if (strlen(mac) == 12) {
		for ( i = 0; i < 12; i++) {
			if ('a' <= mac[i] && mac[i] <= 'f') {
				mac[i] += 'A' - 'a';
			}

			if (!(('0' <= mac[i] && mac[i] <= '9')
				  || ('A' <= mac[i] && mac[i] <= 'F'))) return -1;
		}
	} else {
		return -1;
	}

	return 0;
}

