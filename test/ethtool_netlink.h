/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */

#ifndef _UAPI_LINUX_ETHTOOL_NETLINK_H_
#define _UAPI_LINUX_ETHTOOL_NETLINK_H_

#include <linux/ethtool.h>

/* identifies the device to query/set
 * - use either ifindex or ifname, not both
 * - for dumps and messages not related to a particular devices, fill neither
 * - info_mask is a bitfield, interpretation depends on the command
 */
struct ethtool_nl_msghdr {
	__u32	ifindex;		/* device ifindex */
	__u16	flags;			/* request/response flags */
	__u16	info_mask;		/* request/response info mask */
	char	ifname[IFNAMSIZ];	/* device name */
};
#define ETHNL_HDRLEN NLMSG_ALIGN(sizeof(struct ethtool_nl_msghdr))

enum {
	ETHTOOL_CMD_NOOP,
	ETHTOOL_CMD_GET_DRVINFO,
	ETHTOOL_CMD_SET_DRVINFO,	/* only for reply */
	ETHTOOL_CMD_GET_SETTINGS,
	ETHTOOL_CMD_SET_SETTINGS,

	__ETHTOOL_CMD_MAX,
	ETHTOOL_CMD_MAX = (__ETHTOOL_CMD_MAX - 1),
};

/* bit sets */

enum {
	ETHA_BIT_UNSPEC,
	ETHA_BIT_INDEX,				/* u32 */
	ETHA_BIT_NAME,				/* string */
	ETHA_BIT_VALUE,				/* flag */

	__ETHA_BIT_MAX,
	ETHA_BIT_MAX = (__ETHA_BIT_MAX - 1),
};

enum {
	ETHA_BITS_UNSPEC,
	ETHA_BITS_BIT,

	__ETHA_BITS_MAX,
	ETHA_BITS_MAX = (__ETHA_BITS_MAX - 1),
};

enum {
	ETHA_BITSET_UNSPEC,
	ETHA_BITSET_SIZE,			/* u32 */
	ETHA_BITSET_BITS,			/* nest - ETHA_BITS_* */
	ETHA_BITSET_VALUES,			/* binary */
	ETHA_BITSET_MASK,			/* binary */

	__ETHA_BITSET_MAX,
	ETHA_BITSET_MAX = (__ETHA_BITSET_MAX - 1),
};

/* GET_DRVINFO / SET_DRVINFO */

enum {
	ETHA_DRVINFO_UNSPEC,
	ETHA_DRVINFO_DRIVER,			/* string */
	ETHA_DRVINFO_VERSION,			/* string */
	ETHA_DRVINFO_FWVERSION,			/* string */
	ETHA_DRVINFO_BUSINFO,			/* string */
	ETHA_DRVINFO_EROM_VER,			/* string */
	ETHA_DRVINFO_N_PRIV_FLAGS,		/* u32 */
	ETHA_DRVINFO_N_STATS,			/* u32 */
	ETHA_DRVINFO_TESTINFO_LEN,		/* u32 */
	ETHA_DRVINFO_EEDUMP_LEN,		/* u32 */
	ETHA_DRVINFO_REGDUMP_LEN,		/* u32 */

	__ETHA_DRVINFO_MAX,
	ETHA_DRVINFO_MAX = (__ETHA_DRVINFO_MAX - 1),
};

/* GET_SETTINGS / SET_SETTINGS */

enum {
	ETHA_SETTINGS_UNSPEC,
	ETHA_SETTINGS_SPEED,			/* u32 */
	ETHA_SETTINGS_DUPLEX,			/* s8 */
	ETHA_SETTINGS_PORT,			/* u32 */
	ETHA_SETTINGS_PHYADDR,			/* u32 */
	ETHA_SETTINGS_AUTONEG,			/* u8 */
	ETHA_SETTINGS_MDIO_SUPPORT,		/* bitfield32 */
	ETHA_SETTINGS_TP_MDIX,			/* u32 */
	ETHA_SETTINGS_TP_MDIX_CTRL,		/* u32 */
	ETHA_SETTINGS_WOL_MODES,		/* bitfield32 */
	ETHA_SETTINGS_SOPASS,			/* binary */
	ETHA_SETTINGS_MSGLVL,			/* bitfield32 */
	ETHA_SETTINGS_LINK_MODES,		/* bitset */
	ETHA_SETTINGS_PEER_MODES,		/* bitset */

	__ETHA_SETTINGS_MAX,
	ETHA_SETTINGS_MAX = (__ETHA_SETTINGS_MAX - 1),
};

#define ETH_SETTINGS_RF_COMPACT_BITSETS		0x1

#define ETH_SETTINGS_IM_LINKINFO		0x1
#define ETH_SETTINGS_IM_LINKMODES		0x2
#define ETH_SETTINGS_IM_WOLINFO			0x4

#define ETH_SETTINGS_IM_DEFAULT			0x7

/* generic netlink info */
#define ETHTOOL_GENL_NAME "ethtool"
#define ETHTOOL_GENL_VERSION 1

#endif /* _UAPI_LINUX_ETHTOOL_NETLINK_H_ */
