// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2020-2021, STMicroelectronics
 */

#include <assert.h>
#include <errno.h>

#include <libfdt.h>

#include <common/debug.h>
#include <drivers/regulator.h>
#include <drivers/st/regulator_fixed.h>

#ifndef PLAT_NB_FIXED_REGS
#error "Missing PLAT_NB_FIXED_REGS"
#endif

#define FIXED_NAME_LEN 32U

struct fixed_data {
	char name[FIXED_NAME_LEN];
	uint16_t volt;
	struct regul_description desc;
};

static struct fixed_data data[PLAT_NB_FIXED_REGS];

static int fixed_set_state(const struct regul_description *desc, bool state)
{
	return 0;
}

static int fixed_get_state(const struct regul_description *desc)
{
	return 1;
}

static struct regul_ops fixed_ops = {
	.set_state = fixed_set_state,
	.get_state = fixed_get_state,
};

int fixed_regulator_register(void)
{
	uint32_t count = 0;
	void *fdt;
	int node = 0;

	VERBOSE("fixed reg init!\n");

	if (fdt_get_address(&fdt) == 0) {
		return -FDT_ERR_NOTFOUND;
	}

	do {
		size_t len __unused;
		int ret;
		struct fixed_data *d = &data[count];
		const char *reg_name;

		node = fdt_node_offset_by_compatible(fdt, node, "regulator-fixed");
		if (node < 0) {
			break;
		}

		reg_name = fdt_get_name(fdt, node, NULL);

		VERBOSE("register fixed reg %s!\n", reg_name);

		len = snprintf(d->name, FIXED_NAME_LEN - 1, "%s", reg_name);
		assert((len > 0) && (len < (FIXED_NAME_LEN - 1)));

		d->desc.node_name = d->name;
		d->desc.driver_data = d;
		d->desc.ops = &fixed_ops;

		ret = regulator_register(&d->desc, node);
		if (ret != 0) {
			WARN("%s:%d failed to register %s\n", __func__,
			     __LINE__, reg_name);
			return ret;
		}

		count++;
		assert(count <= PLAT_NB_FIXED_REGS);

	} while (node > 0);

	return 0;
}
