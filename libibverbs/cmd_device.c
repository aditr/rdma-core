/*
 * Copyright (c) 2018 Mellanox Technologies, Ltd.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <infiniband/cmd_write.h>

static void copy_query_port_resp_to_port_attr(struct ibv_port_attr *port_attr,
				       struct ib_uverbs_query_port_resp *resp)
{
	port_attr->state	   = resp->state;
	port_attr->max_mtu	   = resp->max_mtu;
	port_attr->active_mtu      = resp->active_mtu;
	port_attr->gid_tbl_len     = resp->gid_tbl_len;
	port_attr->port_cap_flags  = resp->port_cap_flags;
	port_attr->max_msg_sz      = resp->max_msg_sz;
	port_attr->bad_pkey_cntr   = resp->bad_pkey_cntr;
	port_attr->qkey_viol_cntr  = resp->qkey_viol_cntr;
	port_attr->pkey_tbl_len    = resp->pkey_tbl_len;
	port_attr->lid		   = resp->lid;
	port_attr->sm_lid	   = resp->sm_lid;
	port_attr->lmc		   = resp->lmc;
	port_attr->max_vl_num      = resp->max_vl_num;
	port_attr->sm_sl	   = resp->sm_sl;
	port_attr->subnet_timeout  = resp->subnet_timeout;
	port_attr->init_type_reply = resp->init_type_reply;
	port_attr->active_width    = resp->active_width;
	port_attr->active_speed    = resp->active_speed;
	port_attr->phys_state      = resp->phys_state;
	port_attr->link_layer      = resp->link_layer;
	port_attr->flags	   = resp->flags;
}

int ibv_cmd_query_port(struct ibv_context *context, uint8_t port_num,
		       struct ibv_port_attr *port_attr,
		       struct ibv_query_port *cmd, size_t cmd_size)
{
	DECLARE_FBCMD_BUFFER(cmdb, UVERBS_OBJECT_DEVICE,
			     UVERBS_METHOD_QUERY_PORT, 2, NULL);
	int ret;
	struct ib_uverbs_query_port_resp_ex resp_ex = {};

	fill_attr_const_in(cmdb, UVERBS_ATTR_QUERY_PORT_PORT_NUM, port_num);
	fill_attr_out_ptr(cmdb, UVERBS_ATTR_QUERY_PORT_RESP, &resp_ex);

	switch (execute_ioctl_fallback(context, query_port, cmdb, &ret)) {
	case TRY_WRITE: {
		struct ib_uverbs_query_port_resp resp;

		cmd->port_num = port_num;
		memset(cmd->reserved, 0, sizeof(cmd->reserved));
		memset(&resp, 0, sizeof(resp));

		ret = execute_cmd_write(context,
					IB_USER_VERBS_CMD_QUERY_PORT, cmd,
					cmd_size, &resp, sizeof(resp));
		if (ret)
			return ret;

		copy_query_port_resp_to_port_attr(port_attr, &resp);
		break;
	}
	case SUCCESS:
		copy_query_port_resp_to_port_attr(port_attr,
						  &resp_ex.legacy_resp);
		port_attr->port_cap_flags2 = resp_ex.port_cap_flags2;
		break;
	default:
		return ret;
	};

	return 0;
}

