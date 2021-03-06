/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "UWP_5661.h"

#define FLASH_WRITE_BLOCK_SIZE 0x1

/*
 * This is named flash_uwp_lock instead of flash_uwp_lock (and
 * similarly for flash_uwp_unlock) to avoid confusion with locking
 * actual flash pages.
 */

#if 0
static inline void flash_uwp_lock(struct device *dev)
{
	k_mutex_lock(&DEV_CFG(dev)->lock, K_FOREVER);
}

static inline void flash_uwp_unlock(struct device *dev)
{
	k_mutex_unlock(&DEV_CFG(dev)->lock);
}
static int flash_uwp_write_protection(struct device *dev, bool enable)
{
	struct flash_uwp_config *cfg = DEV_CFG(dev);
	struct spi_flash *flash = &(cfg->flash);

	int ret = 0;

	flash_uwp_lock(dev);

	if(enable)
		ret = flash->lock(flash, 0, flash->size);
	else
		ret = flash->unlock(flash, 0, flash->size);

	flash_uwp_unlock(dev);

	return ret;
}

static int flash_uwp_read(struct device *dev, off_t offset, void *data,
			    size_t len)
{
	int ret = 0;

	if (!len) {
		return 0;
	}

	flash_uwp_lock(dev);

	memcpy(data, (void *)((u32_t)CONFIG_FLASH_BASE_ADDRESS + offset), len);

	flash_uwp_unlock(dev);

	return ret;
}

static int flash_uwp_erase(struct device *dev, off_t offset, size_t len)
{
	int ret;
	struct flash_uwp_config *cfg = DEV_CFG(dev);
	struct spi_flash *flash = &(cfg->flash);

	if (!len) {
		return 0;
	}

	flash_uwp_lock(dev);

	ret = flash->erase(flash, CONFIG_FLASH_BASE_ADDRESS + offset, len);

	flash_uwp_unlock(dev);

	return ret;
}

static int flash_uwp_write(struct device *dev, off_t offset,
			     const void *data, size_t len)
{
	int ret;
	struct flash_uwp_config *cfg = DEV_CFG(dev);
	struct spi_flash *flash = &(cfg->flash);

	if (!len) {
		return 0;
	}

	flash_uwp_lock(dev);

	ret = flash->write(flash, CONFIG_FLASH_BASE_ADDRESS + offset, len, data);

	flash_uwp_unlock(dev);

	return ret;
}

static const struct flash_driver_api flash_uwp_api = {
	.write_protection = flash_uwp_write_protection,
	.erase = flash_uwp_erase,
	.write = flash_uwp_write,
	.read = flash_uwp_read,
#ifdef CONFIG_FLASH_PAGE_LAYOUT
	.page_layout = flash_uwp_page_layout,
#endif
#ifdef FLASH_WRITE_BLOCK_SIZE
	.write_block_size = FLASH_WRITE_BLOCK_SIZE,
#else
#error Flash write block size not available
	/* Flash Write block size is extracted from device tree */
	/* as flash node property 'write-block-size' */
#endif
};
#endif

int uwp_flash_init(struct spi_flash_struct *dev)
{
	int ret = 0;

	struct spi_flash_params *params = dev->params;
	struct spi_flash *flash = &(dev->flash);
	core_util_critical_section_enter();

	spiflash_select_xip(TRUE);

	SFCDRV_EnableInt();

	spiflash_reset_anyway();

	spiflash_set_clk();

	ret = uwp_spi_flash_init(flash, &params);
	if (ret) {
		mbed_error_printf("uwp spi flash init failed. ret:[%d]\n", ret);
		return ret;
	}
	
	core_util_critical_section_exit();
	return ret;
}

