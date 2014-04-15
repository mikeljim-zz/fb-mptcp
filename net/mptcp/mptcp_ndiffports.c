#include <linux/module.h>

#include <net/mptcp.h>
#include <net/mptcp_v4.h>

struct ndiffports_priv {
	/* Worker struct for subflow establishment */
	struct work_struct subflow_work;

	struct mptcp_cb *mpcb;
};

static int sysctl_mptcp_ndiffports __read_mostly = 2;

static void ndiffports_new_session(struct sock *meta_sk, u8 id)
{
	struct mptcp_cb *mpcb = tcp_sk(meta_sk)->mpcb;
	struct ndiffports_priv *fmp = (struct ndiffports_priv *)&mpcb->mptcp_pm[0];

	/* Initialize workqueue-struct */
	fmp->mpcb = mpcb;
}

static void ndiffports_create_subflows(struct sock *meta_sk)
{
}

static int ndiffports_get_local_id(sa_family_t family, union inet_addr *addr,
				  struct net *net)
{
	return 0;
}

static struct mptcp_pm_ops ndiffports __read_mostly = {
	.new_session = ndiffports_new_session,
	.fully_established = ndiffports_create_subflows,
	.get_local_id = ndiffports_get_local_id,
	.name = "ndiffports",
	.owner = THIS_MODULE,
};

static struct ctl_table ndiff_table[] = {
	{
		.procname = "mptcp_ndiffports",
		.data = &sysctl_mptcp_ndiffports,
		.maxlen = sizeof(int),
		.mode = 0644,
		.proc_handler = &proc_dointvec
	},
	{ }
};

struct ctl_table_header *mptcp_sysctl;

/* General initialization of MPTCP_PM */
static int __init ndiffports_register(void)
{
	BUILD_BUG_ON(sizeof(struct ndiffports_priv) > MPTCP_PM_SIZE);

	mptcp_sysctl = register_net_sysctl(&init_net, "net/mptcp", ndiff_table);
	if (!mptcp_sysctl)
		goto exit;

	if (mptcp_register_path_manager(&ndiffports))
		goto pm_failed;

	return 0;

pm_failed:
	unregister_net_sysctl_table(mptcp_sysctl);
exit:
	return -1;
}

static void ndiffports_unregister(void)
{
	mptcp_unregister_path_manager(&ndiffports);
	unregister_net_sysctl_table(mptcp_sysctl);
}

module_init(ndiffports_register);
module_exit(ndiffports_unregister);

MODULE_AUTHOR("Christoph Paasch");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("NDIFF-PORTS MPTCP");
MODULE_VERSION("0.88");
