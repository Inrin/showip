#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
/* gai_strerror */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>

#include <stdio.h>  /* printing */
#include <stdlib.h> /* exit/EXIT_* */
#include <string.h> /* strncmp */

enum ARGFLAGS {
    IPV4 = 1<<0,
    IPV6 = 1<<1,
    GUA6 = 1<<2,
    LLv6 = 1<<3,
    ULA6 = 1<<4,
};

struct opts {
    int flags;
    /* Windows interface names can be up to 256. 
     * Linux differs between 16 and 256.
     */
    char interface[256];
};

static void usage()
{
    fprintf(stderr, "Usage: showip [-46glu] [interface]\n");

    exit(EXIT_FAILURE);
}

/* Pretty naïve arg parser */
/* struct opts has to be freed */
static struct opts *parse_flags(int argc, const char **argv)
{
    struct opts *options = malloc(sizeof *options);
    *options = (struct opts) {0, {'\0'}};

    if (argc < 2)
        return options;

    for (int i=1; i < argc; i++) {
        if (*argv[i] == '-') {
            for (const char *ch = argv[i]+1; *ch; ch++) {
                switch (*ch) {
                    case '4': options->flags |= IPV4; break;
                    case '6': options->flags |= IPV6; break;
                    case 'g': options->flags |= GUA6; break;
                    case 'l': options->flags |= LLv6; break;
                    case 'u': options->flags |= ULA6; break;
                    default: usage();
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

static void print_filtered(const struct ifaddrs *ifa, struct opts *options)
{
    int family, err;
    int flags = options->flags;
    char host[NI_MAXHOST];

    /* Walk through linked list, maintaining head pointer so we
       can free list later */
    for (; ifa != NULL; ifa = ifa->ifa_next) {
        /* Filter interfaces */
        if (
                (ifa->ifa_addr == NULL) || 
                (strncmp(ifa->ifa_name, "lo", 3) == 0) ||
                (options->interface[0] != '\0' && 
                 strncmp(ifa->ifa_name, options->interface, sizeof options->interface) != 0)
           )
            continue;

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

            /* Filter according to options */
            if (
                    (flags == 0) ||
                    ((flags & IPV4) && family == AF_INET) ||
                    /* -6 equals -gul */
                    ((flags & IPV6) && family == AF_INET6) ||
                    ((flags & LLv6) && strncmp(host, "fe80", 4) == 0) ||
                    ((flags & ULA6) && strncmp(host, "fd", 2) == 0) ||
                    ((flags & IPV6) && family == AF_INET6) ||
                    /* Currently GUA is in the range of 2000::/3 2000... - 3fff... */
                    ((flags & GUA6) && (*host == '2' || *host == '3'))
               ) {
                printf("%s\n", host);
            }
        }	
    }
}

int main(int argc, const char **argv)
{
    struct ifaddrs *ifaddr;
    struct opts *opts = parse_flags(argc, argv);

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    print_filtered(ifaddr, opts);

    freeifaddrs(ifaddr);
    free(opts);
    exit(EXIT_SUCCESS);
}
