#include "showip.c"

int main(int argc, const char **argv)
{
	struct ifaddrs *ifaddr;
	struct opts *opts = parse_flags(argc, argv);

	errno = 0;
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	print_filtered(ifaddr, opts);

	freeifaddrs(ifaddr);
	free(opts);
	exit(EXIT_SUCCESS);
}
