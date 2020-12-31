#include <ifaddrs.h>
#include <linux/if_addr.h> /* IFA_F_TEMPORARY */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strncmp */
#include <ctype.h> /* isspace */
#include <stdbool.h>
#include <errno.h>

/* needs _GNU_SOURCE */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define ADDRSIZE 32
#define MAX_NO_ELEMENTS 256

enum ARGFLAGS {
	IPV4 = 1<<0,
	IPV6 = 1<<1,
	GUA6 = 1<<2,
	LLv6 = 1<<3,
	ULA6 = 1<<4,
	TMP6 = 1<<5,
	NTMP = 1<<6,
};

struct opts {
	int flags;
	/* Windows interface names can be up to 256.
	 * Linux differs between 16 and 256.
	 */
	char interface[256];
};

__attribute__((noreturn))
static void usage()
{
	fprintf(stderr, "Usage: showip [-h] [-46gltTu] [interface]\n");

	exit(EXIT_FAILURE);
}

__attribute__((noreturn))
static void help()
{
	puts(
		"Usage: showip [-h] [-46gltTu] [interface]\n"
		"    -h    show this help\n"
		"    -4    print IPv4 addresses\n"
		"    -6    print IPv6 addresses (same as -gltu)\n"
		"    -g    print Global Unique Addresses (GUA)\n"
		"    -l    print Link-Local Addresses (LLA)\n"
		"    -t    print temporary addresses\n"
		"    -T    filter temporary addresses out\n"
		"    -u    print Unique Local Addresses (ULA)\n"
	);

	exit(EXIT_SUCCESS);
}

/* Pretty naïve arg parser */
/* struct opts has to be freed */
static struct opts *parse_flags(int argc, const char **argv)
{
	errno = 0;
	struct opts *options = malloc(sizeof *options);
	if (options == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	*options = (struct opts) {0, {'\0'}};

	if (argc < 2)
		return options;

	for (int i=1; i < argc; i++) {
		if (*argv[i] == '-') {
			for (const char *ch = argv[i]+1; *ch; ch++) {
				switch (*ch) {
					case 'h': help(); break; /* useless break but you never know */
					case '4': options->flags |= IPV4; break;
					case '6': options->flags |= IPV6; break;
					case 'g': options->flags |= GUA6; break;
					case 'l': options->flags |= LLv6; break;
					case 't': options->flags |= TMP6; break;
					case 'T': options->flags |= NTMP; break;
					case 'u': options->flags |= ULA6; break;
					default: usage(); break; /* useless break but you never know */
				}
			}
		} else {
			/* strncpy doesn't always terminate with '\0'... */
			(void) strncpy(options->interface, argv[i], sizeof options->interface - 1);
			options->interface[sizeof options->interface -1] = '\0';
		}
	}

	return options;
}

static char *reduce_v6(char *addr)
{
	char *ret = NULL;
	char *end = NULL;
	bool leading = true;
	size_t setter = 0;
	int maxZeroSeq = 0;
	int zeroSeqCount = 0;
	size_t step = 0;

	if (addr == NULL) {
		errno = EINVAL;
		return NULL;
	}

	errno = 0;
	ret = calloc(ADDRSIZE*2, sizeof *ret);
	if (ret == NULL) {
		return NULL;
	}


	/* Special case of first quartet */
	if (addr[step++] != '0') {
		ret[setter++] = addr[0];
		leading = false;
	}

	/* Iterate over string stripping leading zeros
	 * and calculating maximum zero sequence */
	for (size_t round=1; step < ADDRSIZE && addr[step]; step++,round++) {
		/* Separate each quartet with a ``:'' */
		if (round == 4) {
			round = 0;
			ret[setter++] = ':';
			leading = true;
		}

		/* strip at max 3 leading zeros. Else update Zerocounter */
		if (leading && addr[step] == '0') {
			if (round != 3) {
				continue;
			} else if (++zeroSeqCount > maxZeroSeq) {
				maxZeroSeq = zeroSeqCount;
				end = ret+setter+2;
			}
		}

		/* Reset Zerocounter as necessary */
		leading = false;
		if (addr[step] != '0') {
			zeroSeqCount = 0;
		}

		/* Copy chars */
		ret[setter++] = addr[step];
	}
	if (step != ADDRSIZE) {
		errno = EINVAL;
		return NULL;
	}

	/* Squash multiple zero quartets if needed */
	if (maxZeroSeq > 1) {
		/* 2x as 0:0:... */
		char *start = end-maxZeroSeq*2;
		if (start == ret) {
			*start++ = ':';
		}

		/* Copy chars over to strip 0:0:0... */
		for (*start++ = ':'; *(end-2); *(start++) = *(end++));
	}

	return ret;
}

static const char **parse_proc()
{
	const char **ret = NULL;
	size_t size = 0;
	ssize_t read = 0;
	char *lineptr = NULL;

	char *endptr = NULL;
	unsigned long interface_flags = 0x0;

	errno = 0;
	ret = malloc(MAX_NO_ELEMENTS * sizeof(char *));
	if (ret == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	errno = 0;
	FILE *file = fopen("/proc/net/if_inet6", "r");
	if (file == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	/* Read each line, split(' ') and add temporary addresses to list */
	size_t tmpNo = 0;
	while ((read = getline(&lineptr, &size, file)) != -1) {
		errno = 0;
		interface_flags = strtoul(lineptr+42, &endptr, 16);
		if (errno != 0) {
			perror("strtol");
			exit(EXIT_FAILURE);
		}
		if (endptr == lineptr+42) {
			fprintf(stderr, "No digits were found\n");
			exit(EXIT_FAILURE);
		}

		if (interface_flags & IFA_F_TEMPORARY) {
			ret[tmpNo++] = reduce_v6(lineptr);
		}
	}

	/* Cleanup */
	free(lineptr);
	fclose(file);

	/* Terminate list */
	ret[tmpNo] = NULL;
	return ret;
}


static bool containsAddr(const char *needle, const char **haystack)
{
	if (haystack == NULL) {
		return needle == NULL;
	}

	for (; *haystack; haystack++) {
		if (strcmp(needle, *haystack) == 0) {
			return true;
		}
	}

	return false;
}

static void print_filtered(const struct ifaddrs *ifa, const struct opts *options)
{
	int family, err;
	const int flags = options->flags;
	char host[NI_MAXHOST];
	const char **tmps = NULL;

	if (options->flags & TMP6 || options->flags & NTMP) {
		tmps = parse_proc();
	}

	/* Walk through linked list, maintaining head pointer so we
	   can free list later */
	for (; ifa != NULL; ifa = ifa->ifa_next) {
		/* Filter interfaces */
		if (
				(ifa->ifa_addr == NULL) ||
				(options->interface[0] != '\0' &&
				 strncmp(ifa->ifa_name, options->interface, sizeof options->interface) != 0)
		   ) {
			continue;
		}

		/* Only recognize IPv(4|6) */
		family = ifa->ifa_addr->sa_family;
		if (family == AF_INET || family == AF_INET6) {
			/* Get human readable address */
			err = getnameinfo(ifa->ifa_addr,
					(family == AF_INET) ? sizeof(struct sockaddr_in)
					                    : sizeof(struct sockaddr_in6),
					host, NI_MAXHOST,
					NULL, 0, NI_NUMERICHOST);
			if (err != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(err));
				exit(EXIT_FAILURE);
			}

			if (flags == 0) {
				puts(host);
			} else if (family == AF_INET6) {
				if ((flags & (TMP6|NTMP)) && containsAddr(host, tmps)) {
					if (flags & TMP6) {
						puts(host);
					}
				} else if (
						(flags & IPV6) ||
						/* Currently GUA is in the range of 2000::/3 2000... - 3fff... */
						((flags & GUA6) && (*host == '2' || *host == '3')) ||
						((flags & ULA6) && strncmp(host, "fd", 2) == 0) ||
						((flags & LLv6) && strncmp(host, "fe80", 4) == 0) ||
						((flags & ULA6) && strncmp(host, "fc", 2) == 0) ||
						(flags & NTMP)
					 ) {
					puts(host);
				}
			} else if ((family == AF_INET) && (flags & IPV4)) {
				puts(host);
			}
		}
	}

	if (tmps) {
		for (const char **t = tmps; *t; t++) {
			free((char *)*t);
		}
	}
	free(tmps);
}
