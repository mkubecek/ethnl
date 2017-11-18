#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <libmnl/libmnl.h>
#include <linux/if.h>
#include <linux/genetlink.h>
#include "ethtool_netlink.h"

struct mnl_socket *nlsk;
unsigned int port;
unsigned int seq;
int ethnl_fam;

char buff[4096];

static void socket_init(void)
{
	seq = time(NULL);

	nlsk = mnl_socket_open(NETLINK_GENERIC);
	if (!nlsk) {
		fprintf(stderr, "mnl_socket_open() failed\n");
		exit(2);
	}
	if (mnl_socket_bind(nlsk, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(2);
	}
	port = mnl_socket_get_portid(nlsk);
}

static void socket_done(void)
{
	mnl_socket_close(nlsk);
}

static struct nlmsghdr *__msg_init(int family, int cmd, unsigned int flags,
				   int version)
{
	struct nlmsghdr *nlhdr;
	struct genlmsghdr *ghdr;

	seq++;
	memset(buff, '\0', sizeof(buff));

	nlhdr = mnl_nlmsg_put_header(buff);
	nlhdr->nlmsg_type = family;
	nlhdr->nlmsg_flags = flags;
	nlhdr->nlmsg_seq = seq;

	ghdr = mnl_nlmsg_put_extra_header(nlhdr, sizeof(*ghdr));
	ghdr->cmd = cmd;
	ghdr->version = version;

	return nlhdr;
}

static struct nlmsghdr *msg_init(int cmd, unsigned int flags,
				 const char *ifname, unsigned int req_mask,
				 unsigned int req_flags)
{
	struct ethtool_nl_msghdr *ehdr;
	struct nlmsghdr *nlhdr;

	nlhdr = __msg_init(ethnl_fam, cmd, flags, ETHTOOL_GENL_VERSION);
	ehdr = mnl_nlmsg_put_extra_header(nlhdr, sizeof(*ehdr));

	memset(ehdr, '\0', sizeof(*ehdr));
	ehdr->flags = req_flags;
	ehdr->info_mask = req_mask;
	strncpy(ehdr->ifname, ifname, sizeof(ehdr->ifname) - 1);

	return nlhdr;
}

static ssize_t ethnl_sendto(struct nlmsghdr *nlhdr)
{
	ssize_t len;

	len = mnl_socket_sendto(nlsk, nlhdr, nlhdr->nlmsg_len);
	if (len < 0) {
		perror("mnl_socket_sendto");
		exit(2);
	}

	return len;
}

static int ethnl_process_reply(mnl_cb_t reply_cb, void *data)
{
	ssize_t len;
	int ret;

	do {
		len = mnl_socket_recvfrom(nlsk, buff, sizeof(buff));
/*		printf("received %ld bytes\n", len); */
		if (len <= 0)
			return (ret ? -EFAULT : 0);
		mnl_nlmsg_fprintf(stdout, buff, len,
				  sizeof(struct genlmsghdr) + sizeof(struct ethtool_nl_msghdr));
		ret = mnl_cb_run(buff, len, seq, port, reply_cb, NULL);
	} while (ret > 0);

	return  ret;
}

/* get_family */

static int ethnl_family_cb(const struct nlmsghdr *nlhdr, void *data)
{
	struct nlattr *attr;

	ethnl_fam = 0;
	mnl_attr_for_each(attr, nlhdr, sizeof(struct genlmsghdr)) {
		if (mnl_attr_get_type(attr) == CTRL_ATTR_FAMILY_ID) {
			ethnl_fam = mnl_attr_get_u16(attr);
			break;
		}
	}

	return (ethnl_fam ? MNL_CB_OK : MNL_CB_ERROR);
}

void get_ethnl_family(void)
{
	struct nlmsghdr *nlhdr;

	nlhdr = __msg_init(GENL_ID_CTRL, CTRL_CMD_GETFAMILY,
			   NLM_F_REQUEST | NLM_F_ACK, 1);
	mnl_attr_put_strz(nlhdr, CTRL_ATTR_FAMILY_NAME, ETHTOOL_GENL_NAME);

	ethnl_sendto(nlhdr);
	ethnl_process_reply(ethnl_family_cb, NULL);

	if (!ethnl_fam) {
		fprintf(stderr, "ethtool generic netlink family unavailable\n");
		exit(2);
	}
}

/* GET_DRVINFO */

static int attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	tb[type] = attr;

	return MNL_CB_OK;
}

static void print_drvinfo_str(const struct nlattr **tb, unsigned int idx,
			      const char *label)
{
	printf("%s: %s\n", label, tb[idx] ? mnl_attr_get_str(tb[idx]) : "");
}

static void print_drvinfo_u32b(const struct nlattr **tb, unsigned int idx,
			       const char *label)
{
	if (tb[idx])
		printf("%s: %s\n", label,
		       mnl_attr_get_u32(tb[idx]) ? "yes" : "no");
}

static int drvinfo_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	struct ethtool_nl_msghdr *ehdr =
		mnl_nlmsg_get_payload_offset(nlhdr, sizeof(struct genlmsghdr));
	const struct nlattr *tb[ETHA_DRVINFO_MAX + 1] = {};
	int ret;
/*
	unsigned i;
	unsigned len = nlhdr->nlmsg_len;
	for (i = 0; i < len; i++) {
		if (i % 8 == 0) {
			if (i > 0)
				putchar('\n');
			printf("%04x ", i);
		}
		printf(" %02x", ((unsigned char *)nlhdr)[i]);
	}
	putchar('\n');
*/
	ret = mnl_attr_parse(nlhdr, sizeof(struct genlmsghdr) + sizeof(*ehdr),
			     attr_cb, tb);
	if (ret < 0)
		return ret;

/*	printf("device: %s (%u)\n", ehdr->ifname, ehdr->ifindex); */
	print_drvinfo_str(tb, ETHA_DRVINFO_DRIVER, "driver");
	print_drvinfo_str(tb, ETHA_DRVINFO_VERSION, "version");
	print_drvinfo_str(tb, ETHA_DRVINFO_FWVERSION, "firmware-version");
	print_drvinfo_str(tb, ETHA_DRVINFO_EROM_VER, "expansion-rom-version");
	print_drvinfo_str(tb, ETHA_DRVINFO_BUSINFO, "businfo");
	print_drvinfo_u32b(tb, ETHA_DRVINFO_N_STATS, "supports-statistics");
	print_drvinfo_u32b(tb, ETHA_DRVINFO_TESTINFO_LEN, "supports-test");
	print_drvinfo_u32b(tb, ETHA_DRVINFO_EEDUMP_LEN,
			  "supports-eeprom-access");
	print_drvinfo_u32b(tb, ETHA_DRVINFO_REGDUMP_LEN,
			  "supports-register-dump");
	print_drvinfo_u32b(tb, ETHA_DRVINFO_N_PRIV_FLAGS, "supports-priv-flags");

	return MNL_CB_OK;
}

static void get_drvinfo(const char *ifname)
{
	struct nlmsghdr *nlhdr;

	nlhdr = msg_init(ETHTOOL_CMD_GET_DRVINFO, NLM_F_REQUEST | NLM_F_ACK,
			 ifname, 0, 0);
	ethnl_sendto(nlhdr);
	ethnl_process_reply(drvinfo_reply_cb, NULL);
}

/* GET_SETTINGS */

static int settings_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	return MNL_CB_OK;
}

static void get_settings(const char *ifname)
{
	struct nlmsghdr *nlhdr;

	nlhdr = msg_init(ETHTOOL_CMD_GET_SETTINGS, NLM_F_REQUEST | NLM_F_ACK,
			 ifname, 0, 0);
	ethnl_sendto(nlhdr);
	ethnl_process_reply(settings_reply_cb, NULL);
}



int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "missing argument\n");
		return 1;
	}

	socket_init();
	get_ethnl_family();

/*	get_drvinfo(argv[1]); */
	get_settings(argv[1]);

	socket_done();

	return 0;
}

