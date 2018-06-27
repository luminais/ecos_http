#include "uuid.h"

#include <string.h>
#include <sys/time.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
//#include <sys/types.h>
//#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


static const char *fmt_lower =
	"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x";

static const char *fmt_upper =
	"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X";

#ifdef UUID_UNPARSE_DEFAULT_UPPER
#define FMT_DEFAULT fmt_upper
#else
#define FMT_DEFAULT fmt_lower
#endif


void uuid_unpack(const uuid_t in, struct uuid *uu)
{
	const uint8_t	*ptr = in;
	uint32_t		tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->time_low = tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->time_mid = tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->time_hi_and_version = tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uu->clock_seq = tmp;

	memcpy(uu->node, ptr, 6);
}


static void uuid_unparse_x(const uuid_t uu, char *out, const char *fmt)
{
	struct uuid uuid;

	uuid_unpack(uu, &uuid);
	sprintf(out, fmt,
			uuid.time_low, uuid.time_mid, uuid.time_hi_and_version,
			uuid.clock_seq >> 8, uuid.clock_seq & 0xFF,
			uuid.node[0], uuid.node[1], uuid.node[2],
			uuid.node[3], uuid.node[4], uuid.node[5]);
}

void uuid_unparse_lower(const uuid_t uu, char *out)
{
	uuid_unparse_x(uu, out,	fmt_lower);
}

void uuid_unparse_upper(const uuid_t uu, char *out)
{
	uuid_unparse_x(uu, out,	fmt_upper);
}

void uuid_unparse(const uuid_t uu, char *out)
{
	uuid_unparse_x(uu, out, FMT_DEFAULT);
}


void uuid_pack(const struct uuid *uu, uuid_t ptr)
{
	uint32_t	tmp;
	unsigned char	*out = ptr;

	tmp = uu->time_low;
	out[3] = (unsigned char) tmp;
	tmp >>= 8;
	out[2] = (unsigned char) tmp;
	tmp >>= 8;
	out[1] = (unsigned char) tmp;
	tmp >>= 8;
	out[0] = (unsigned char) tmp;

	tmp = uu->time_mid;
	out[5] = (unsigned char) tmp;
	tmp >>= 8;
	out[4] = (unsigned char) tmp;

	tmp = uu->time_hi_and_version;
	out[7] = (unsigned char) tmp;
	tmp >>= 8;
	out[6] = (unsigned char) tmp;

	tmp = uu->clock_seq;
	out[9] = (unsigned char) tmp;
	tmp >>= 8;
	out[8] = (unsigned char) tmp;

	memcpy(out + 10, uu->node, 6);
}


int random_get_fd(void)
{
	int i, fd;
	struct timeval	tv;

	gettimeofday(&tv, 0);
#if 0
	fd = open("/dev/urandom", O_RDONLY);

	if (fd == -1)
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK);

	if (fd >= 0) {
		i = fcntl(fd, F_GETFD);

		if (i >= 0)
			fcntl(fd, F_SETFD, i | FD_CLOEXEC);
	}
#endif
	srand((getpid() << 16)^ tv.tv_sec ^ tv.tv_usec);

	/* Crank the random number generator a few times */
	gettimeofday(&tv, 0);

	for (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		(void)rand();

	return fd;
}



/*
 * Generate a stream of random nbytes into buf.
 * Use /dev/urandom if possible, and if not,
 * use glibc pseudo-random functions.
 */
void random_get_bytes(void *buf, size_t nbytes)
{
	size_t i, n = nbytes;
	int fd = random_get_fd();
	int lose_counter = 0;
	unsigned char *cp = (unsigned char *) buf;

	if (fd >= 0) {
		while (n > 0) {
			ssize_t x = read(fd, cp, n);

			if (x <= 0) {
				if (lose_counter++ > 16)
					break;

				continue;
			}

			n -= x;
			cp += x;
			lose_counter = 0;
		}

		close(fd);
	}

	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	for (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

	return;
}


void __uuid_generate_random(uuid_t out, int *num)
{
	uuid_t	buf;
	struct uuid uu;
	int i, n;

	if (!num || !*num)
		n = 1;
	else
		n = *num;

	for (i = 0; i < n; i++) {
		random_get_bytes(buf, sizeof(buf));
		uuid_unpack(buf, &uu);

		uu.clock_seq = (uu.clock_seq & 0x3FFF) | 0x8000;
		uu.time_hi_and_version = (uu.time_hi_and_version & 0x0FFF)
								 | 0x4000;
		uuid_pack(&uu, out);
		out += sizeof(uuid_t);
	}
}

void uuid_generate_random(uuid_t out)
{
	int	num = 1;
	/* No real reason to use the daemon for random uuid's -- yet */

	__uuid_generate_random(out, &num);
}
