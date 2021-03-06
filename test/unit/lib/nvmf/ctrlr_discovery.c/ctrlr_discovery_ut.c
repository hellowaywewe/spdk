/*-
 *   BSD LICENSE
 *
 *   Copyright (c) Intel Corporation.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "spdk/stdinc.h"

#include "spdk_cunit.h"

#include "ctrlr_discovery.c"

SPDK_LOG_REGISTER_TRACE_FLAG("nvmf", SPDK_TRACE_NVMF)

struct spdk_nvmf_tgt g_nvmf_tgt = {
	.subsystems = TAILQ_HEAD_INITIALIZER(g_nvmf_tgt.subsystems)
};

int
spdk_bdev_open(struct spdk_bdev *bdev, bool write, spdk_bdev_remove_cb_t remove_cb,
	       void *remove_ctx, struct spdk_bdev_desc **desc)
{
	return 0;
}

const char *
spdk_bdev_get_name(const struct spdk_bdev *bdev)
{
	return "test";
}

int
spdk_nvmf_transport_listen(struct spdk_nvmf_transport *transport,
			   const struct spdk_nvme_transport_id *trid)
{
	return 0;
}

void
spdk_nvmf_transport_listener_discover(struct spdk_nvmf_transport *transport,
				      struct spdk_nvme_transport_id *trid,
				      struct spdk_nvmf_discovery_log_page_entry *entry)
{
	entry->trtype = 42;
}

static struct spdk_nvmf_transport g_transport = {};

struct spdk_nvmf_transport *
spdk_nvmf_transport_create(struct spdk_nvmf_tgt *tgt,
			   enum spdk_nvme_transport_type type)
{
	if (type == SPDK_NVME_TRANSPORT_RDMA) {
		g_transport.tgt = tgt;
		return &g_transport;
	}

	return NULL;
}

struct spdk_nvmf_transport *
spdk_nvmf_tgt_get_transport(struct spdk_nvmf_tgt *tgt, enum spdk_nvme_transport_type trtype)
{
	return &g_transport;
}

bool
spdk_nvmf_transport_qpair_is_idle(struct spdk_nvmf_qpair *qpair)
{
	return false;
}

int
spdk_nvme_transport_id_parse_trtype(enum spdk_nvme_transport_type *trtype, const char *str)
{
	if (trtype == NULL || str == NULL) {
		return -EINVAL;
	}

	if (strcasecmp(str, "PCIe") == 0) {
		*trtype = SPDK_NVME_TRANSPORT_PCIE;
	} else if (strcasecmp(str, "RDMA") == 0) {
		*trtype = SPDK_NVME_TRANSPORT_RDMA;
	} else {
		return -ENOENT;
	}
	return 0;
}

int
spdk_nvme_transport_id_compare(const struct spdk_nvme_transport_id *trid1,
			       const struct spdk_nvme_transport_id *trid2)
{
	return 0;
}

void
spdk_nvmf_ctrlr_destruct(struct spdk_nvmf_ctrlr *ctrlr)
{
}

int
spdk_nvmf_ctrlr_poll(struct spdk_nvmf_ctrlr *ctrlr)
{
	return -1;
}

int
spdk_nvmf_subsystem_bdev_attach(struct spdk_nvmf_subsystem *subsystem)
{
	return -1;
}

void
spdk_nvmf_subsystem_bdev_detach(struct spdk_nvmf_subsystem *subsystem)
{
}

static bool
all_zero(const void *buf, size_t size)
{
	const uint8_t *b = buf;

	while (size--) {
		if (*b != 0) {
			return false;
		}
		b++;
	}

	return true;
}

static void
test_discovery_log(void)
{
	struct spdk_nvmf_subsystem *subsystem;
	uint8_t buffer[8192];
	struct spdk_nvmf_discovery_log_page *disc_log;
	struct spdk_nvmf_discovery_log_page_entry *entry;
	struct spdk_nvme_transport_id trid = {};

	/* Reset discovery-related globals */
	g_nvmf_tgt.discovery_genctr = 0;
	free(g_nvmf_tgt.discovery_log_page);
	g_nvmf_tgt.discovery_log_page = NULL;
	g_nvmf_tgt.discovery_log_page_size = 0;

	/* Add one subsystem and verify that the discovery log contains it */
	subsystem = spdk_nvmf_create_subsystem(&g_nvmf_tgt, "nqn.2016-06.io.spdk:subsystem1",
					       SPDK_NVMF_SUBTYPE_NVME, 0,
					       NULL, NULL, NULL);
	SPDK_CU_ASSERT_FATAL(subsystem != NULL);

	trid.trtype = SPDK_NVME_TRANSPORT_RDMA;
	trid.adrfam = SPDK_NVMF_ADRFAM_IPV4;
	snprintf(trid.traddr, sizeof(trid.traddr), "1234");
	snprintf(trid.trsvcid, sizeof(trid.trsvcid), "5678");
	SPDK_CU_ASSERT_FATAL(spdk_nvmf_subsystem_add_listener(subsystem, &trid) == 0);

	/* Get only genctr (first field in the header) */
	memset(buffer, 0xCC, sizeof(buffer));
	disc_log = (struct spdk_nvmf_discovery_log_page *)buffer;
	spdk_nvmf_get_discovery_log_page(&g_nvmf_tgt, buffer, 0, sizeof(disc_log->genctr));
	CU_ASSERT(disc_log->genctr == 1); /* one added subsystem */

	/* Get only the header, no entries */
	memset(buffer, 0xCC, sizeof(buffer));
	disc_log = (struct spdk_nvmf_discovery_log_page *)buffer;
	spdk_nvmf_get_discovery_log_page(&g_nvmf_tgt, buffer, 0, sizeof(*disc_log));
	CU_ASSERT(disc_log->genctr == 1);
	CU_ASSERT(disc_log->numrec == 1);

	/* Offset 0, exact size match */
	memset(buffer, 0xCC, sizeof(buffer));
	disc_log = (struct spdk_nvmf_discovery_log_page *)buffer;
	spdk_nvmf_get_discovery_log_page(&g_nvmf_tgt, buffer, 0,
					 sizeof(*disc_log) + sizeof(disc_log->entries[0]));
	CU_ASSERT(disc_log->genctr != 0);
	CU_ASSERT(disc_log->numrec == 1);
	CU_ASSERT(disc_log->entries[0].trtype == 42);

	/* Offset 0, oversize buffer */
	memset(buffer, 0xCC, sizeof(buffer));
	disc_log = (struct spdk_nvmf_discovery_log_page *)buffer;
	spdk_nvmf_get_discovery_log_page(&g_nvmf_tgt, buffer, 0, sizeof(buffer));
	CU_ASSERT(disc_log->genctr != 0);
	CU_ASSERT(disc_log->numrec == 1);
	CU_ASSERT(disc_log->entries[0].trtype == 42);
	CU_ASSERT(all_zero(buffer + sizeof(*disc_log) + sizeof(disc_log->entries[0]),
			   sizeof(buffer) - (sizeof(*disc_log) + sizeof(disc_log->entries[0]))));

	/* Get just the first entry, no header */
	memset(buffer, 0xCC, sizeof(buffer));
	entry = (struct spdk_nvmf_discovery_log_page_entry *)buffer;
	spdk_nvmf_get_discovery_log_page(&g_nvmf_tgt, buffer,
					 offsetof(struct spdk_nvmf_discovery_log_page, entries[0]),
					 sizeof(*entry));
	CU_ASSERT(entry->trtype == 42);
	spdk_nvmf_delete_subsystem(subsystem);
}

int main(int argc, char **argv)
{
	CU_pSuite	suite = NULL;
	unsigned int	num_failures;

	if (CU_initialize_registry() != CUE_SUCCESS) {
		return CU_get_error();
	}

	suite = CU_add_suite("nvmf", NULL, NULL);
	if (suite == NULL) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	if (
		CU_add_test(suite, "discovery_log", test_discovery_log) == NULL) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	num_failures = CU_get_number_of_failures();
	CU_cleanup_registry();
	return num_failures;
}
