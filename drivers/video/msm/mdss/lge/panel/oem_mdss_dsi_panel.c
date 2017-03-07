/* Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/leds.h>

#include <soc/qcom/lge/lge_boot_mode.h>

#include "mdss_dsi.h"
#include "mdss_mdp.h"

#include "oem_mdss_dsi_common.h"
#include "oem_mdss_dsi.h"
#include "oem_mdss_dsi_panel.h"

#if defined (CONFIG_MFD_RT4832)
#include <linux/mfd/rt4832.h>
#endif

#if IS_ENABLED(CONFIG_LGE_READER_MODE)
#include "lge/common/reader_mode.h"
#endif

#if IS_ENABLED(CONFIG_LGE_READER_MODE)
static struct dsi_panel_cmds reader_mode_step1_cmds;
static struct dsi_panel_cmds reader_mode_step2_cmds;
static struct dsi_panel_cmds reader_mode_step3_cmds;
static struct dsi_panel_cmds reader_mode_off_cmds;
//static struct dsi_panel_cmds reader_mode_mono_enable_cmds;
//static struct dsi_panel_cmds reader_mode_mono_disable_cmds;
#endif

#define PIN_DDVDH "DDVDH"
#define PIN_RESET "RESET"
#define PANEL_SEQUENCE(name, state) pr_info("[PanelSequence][%s] %d\n", name, state);

bool oem_shutdown_pending;

int tianma_hvga_cmd_mdss_dsi_request_gpios(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc = 0;
	struct mdss_panel_info *pinfo = NULL;
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		rc = gpio_request(ctrl_pdata->disp_en_gpio,
						"disp_enable");
		if (rc) {
			pr_err("request disp_en gpio failed, rc=%d\n",
				       rc);
			goto disp_en_gpio_err;
		}
	}

	rc = gpio_request(ctrl_pdata->rst_gpio, "disp_rst_n");
	if (rc) {
		pr_err("request reset gpio failed, rc=%d\n",
			rc);
		goto rst_gpio_err;
	}

	if (gpio_is_valid(ctrl_pdata->bklt_en_gpio)) {
		rc = gpio_request(ctrl_pdata->bklt_en_gpio,
						"bklt_enable");
		if (rc) {
			pr_err("request bklt gpio failed, rc=%d\n",
				       rc);
			goto bklt_en_gpio_err;
		}
	}
	if (gpio_is_valid(ctrl_pdata->mode_gpio)) {
		rc = gpio_request(ctrl_pdata->mode_gpio, "panel_mode");
		if (rc) {
			pr_err("request panel mode gpio failed,rc=%d\n",
								rc);
			goto mode_gpio_err;
		}
	}
	return rc;

mode_gpio_err:
	if (gpio_is_valid(ctrl_pdata->bklt_en_gpio))
		gpio_free(ctrl_pdata->bklt_en_gpio);
bklt_en_gpio_err:
	gpio_free(ctrl_pdata->rst_gpio);
rst_gpio_err:
	if (gpio_is_valid(ctrl_pdata->disp_en_gpio))
		gpio_free(ctrl_pdata->disp_en_gpio);
disp_en_gpio_err:
	return rc;

}

int tianma_hvga_cmd_mdss_dsi_panel_reset(struct mdss_panel_data *pdata, int enable)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;
	int i, rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (!gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
	}

	if (!gpio_is_valid(ctrl_pdata->rst_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return rc;
	}

	pr_info("%s: enable = %d\n", __func__, enable);
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (enable) {
		if (lge_mdss_dsi.lge_mdss_dsi_request_gpios)
			lge_mdss_dsi.lge_mdss_dsi_request_gpios(ctrl_pdata);
		if (rc) {
			pr_err("gpio request failed\n");
			return rc;
		}

		if (!pinfo->cont_splash_enabled) {
			if (gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
				gpio_set_value((ctrl_pdata->disp_en_gpio), 1);
				PANEL_SEQUENCE(PIN_DDVDH, 1);
			}

		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		PANEL_SEQUENCE(PIN_RESET, 0);
		mdelay(5);

		for (i = 0; i < pdata->panel_info.rst_seq_len; ++i) {
			gpio_set_value((ctrl_pdata->rst_gpio),
				pdata->panel_info.rst_seq[i]);
				PANEL_SEQUENCE(PIN_RESET, pdata->panel_info.rst_seq[i]);
			if (pdata->panel_info.rst_seq[++i])
				usleep(pinfo->rst_seq[i] * 1000);
		}

		if (gpio_is_valid(ctrl_pdata->bklt_en_gpio))
			gpio_set_value((ctrl_pdata->bklt_en_gpio), 1);
		}

		if (gpio_is_valid(ctrl_pdata->mode_gpio)) {
			if (pinfo->mode_gpio_state == MODE_GPIO_HIGH)
				gpio_set_value((ctrl_pdata->mode_gpio), 1);
			else if (pinfo->mode_gpio_state == MODE_GPIO_LOW)
				gpio_set_value((ctrl_pdata->mode_gpio), 0);
		}
		if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
			pr_debug("%s: Panel Not properly turned OFF\n",
						__func__);
			ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
			pr_debug("%s: Reset panel done\n", __func__);
		}
	} else {
		if (gpio_is_valid(ctrl_pdata->bklt_en_gpio)) {
			gpio_set_value((ctrl_pdata->bklt_en_gpio), 0);
			gpio_free(ctrl_pdata->bklt_en_gpio);
		}

		if (gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
			gpio_set_value((ctrl_pdata->disp_en_gpio), 0);
			PANEL_SEQUENCE(PIN_DDVDH, 0);
			gpio_free(ctrl_pdata->disp_en_gpio);
		}

		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		PANEL_SEQUENCE(PIN_RESET, 0);
		gpio_free(ctrl_pdata->rst_gpio);

		if (gpio_is_valid(ctrl_pdata->mode_gpio))
			gpio_free(ctrl_pdata->mode_gpio);
	}
	return rc;

}

int jdi_qhd_dual_cmd_mdss_dsi_request_gpios(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc = 0;
	return rc;
}

int lgd_qhd_dual_cmd_mdss_dsi_request_gpios(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc = 0;
	return rc;
}

int tianma_hvga_cmd_mdss_panel_parse_dts(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	return 0;
}

int tianma_hvga_cmd_panel_device_create(struct device_node *node,
	struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{

	return 0;
}

int lgd_jdi_qhd_cmd_panel_device_create(struct device_node *node,
	struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{

	return 0;
}

#if defined(CONFIG_LGD_SSD2068_FWVGA_VIDEO_PANEL)
int lgd_fwvga_video_mdss_dsi_request_gpios(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc = 0;
	struct mdss_panel_info *pinfo = NULL;
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (pinfo->cont_splash_enabled) {
		rc = gpio_request(ctrl_pdata->rst_gpio, "disp_rst_n");
		if (rc) {
			pr_err("request reset gpio failed, rc=%d\n",
					rc);
			goto rst_gpio_err;
		}

		if (gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
			rc = gpio_request(ctrl_pdata->disp_en_gpio,
					"disp_enable");
			if (rc)
				pr_err("request disp_en gpio failed, rc=%d\n", rc);
		}

		if (gpio_is_valid(ctrl_pdata->disp_en_p_gpio)) {
			rc = gpio_request(ctrl_pdata->disp_en_p_gpio, "dsv_p_enable");
			if (rc)
				pr_err("request disp_en_p gpio failed, rc=%d\n", rc);
		}

		if (gpio_is_valid(ctrl_pdata->disp_en_n_gpio)) {
			rc = gpio_request(ctrl_pdata->disp_en_n_gpio,
					"dsv_n_enable");
			if (rc)
				pr_err("request disp_en_n gpio failed, rc=%d\n", rc);
		}
	}

	if (gpio_is_valid(ctrl_pdata->bklt_en_gpio)) {
		rc = gpio_request(ctrl_pdata->bklt_en_gpio,
				"bklt_enable");
		if (rc) {
			pr_err("request bklt gpio failed, rc=%d\n",
					rc);
			goto bklt_en_gpio_err;
		}
	}

	if (gpio_is_valid(ctrl_pdata->mode_gpio)) {
		rc = gpio_request(ctrl_pdata->mode_gpio, "panel_mode");
		if (rc) {
			pr_err("request panel mode gpio failed,rc=%d\n",
					rc);
			goto mode_gpio_err;
		}
	}

	return rc;

mode_gpio_err:
	if (gpio_is_valid(ctrl_pdata->bklt_en_gpio))
		gpio_free(ctrl_pdata->bklt_en_gpio);
bklt_en_gpio_err:
	gpio_free(ctrl_pdata->rst_gpio);
rst_gpio_err:
	if (gpio_is_valid(ctrl_pdata->disp_en_gpio))
		gpio_free(ctrl_pdata->disp_en_gpio);
	return rc;
}

int lgd_fwvga_video_mdss_dsi_panel_reset(struct mdss_panel_data *pdata, int enable)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;
	int i, rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (!gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		pr_debug("%s:%d, enable gpio is not configured\n",
			   __func__, __LINE__);
	}

	if (!gpio_is_valid(ctrl_pdata->rst_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return rc;
	}

	pr_info("%s: enable = %d\n", __func__, enable);
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (enable) {
		if (lge_mdss_dsi.lge_mdss_dsi_request_gpios)
			lge_mdss_dsi.lge_mdss_dsi_request_gpios(ctrl_pdata);
		if (rc) {
			pr_err("gpio request failed\n");
			return rc;
		}

		if (!pinfo->cont_splash_enabled) {
#if defined (CONFIG_MFD_RT4832)
			if (ctrl_pdata->dsv_toggle_enabled)
				rt4832_periodic_ctrl(enable);
#endif
			gpio_set_value((ctrl_pdata->rst_gpio), 0);
			PANEL_SEQUENCE(PIN_RESET, 0);
			mdelay(5);

			for (i = 0; i < pdata->panel_info.rst_seq_len; ++i) {
				gpio_set_value((ctrl_pdata->rst_gpio),
						pdata->panel_info.rst_seq[i]);
				PANEL_SEQUENCE(PIN_RESET, pdata->panel_info.rst_seq[i]);
				if (pdata->panel_info.rst_seq[++i])
					usleep(pinfo->rst_seq[i] * 1000);
			}

			if (gpio_is_valid(ctrl_pdata->bklt_en_gpio))
				gpio_set_value((ctrl_pdata->bklt_en_gpio), 1);
		}

		if (gpio_is_valid(ctrl_pdata->mode_gpio)) {
			if (pinfo->mode_gpio_state == MODE_GPIO_HIGH)
				gpio_set_value((ctrl_pdata->mode_gpio), 1);
			else if (pinfo->mode_gpio_state == MODE_GPIO_LOW)
				gpio_set_value((ctrl_pdata->mode_gpio), 0);
		}
		if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
			pr_debug("%s: Panel Not properly turned OFF\n",
						__func__);
			ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
			pr_debug("%s: Reset panel done\n", __func__);
		}
	} else {
		if (gpio_is_valid(ctrl_pdata->bklt_en_gpio)) {
			gpio_set_value((ctrl_pdata->bklt_en_gpio), 0);
			gpio_free(ctrl_pdata->bklt_en_gpio);
		}

		if (lge_get_boot_mode() == LGE_BOOT_MODE_CHARGERLOGO) {
			PANEL_SEQUENCE(PIN_RESET, 0);
			gpio_set_value((ctrl_pdata->rst_gpio), 0);
		}

		if (oem_shutdown_pending) {
			if (gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
				gpio_set_value((ctrl_pdata->disp_en_gpio), 0);
				gpio_free(ctrl_pdata->disp_en_gpio);
			}

			if (gpio_is_valid(ctrl_pdata->disp_en_p_gpio)) {
				gpio_set_value((ctrl_pdata->disp_en_p_gpio), 0);
				gpio_free(ctrl_pdata->disp_en_p_gpio);
			}

			if (gpio_is_valid(ctrl_pdata->disp_en_n_gpio)) {
				gpio_set_value((ctrl_pdata->disp_en_n_gpio), 0);
				gpio_free(ctrl_pdata->disp_en_n_gpio);
			}
			gpio_set_value((ctrl_pdata->rst_gpio), 0);
			PANEL_SEQUENCE(PIN_RESET, 0);
			gpio_free(ctrl_pdata->rst_gpio);
		}
#if defined (CONFIG_MFD_RT4832)
		if (ctrl_pdata->dsv_toggle_enabled)
			rt4832_periodic_ctrl(enable);
#endif
		if (gpio_is_valid(ctrl_pdata->mode_gpio))
			gpio_free(ctrl_pdata->mode_gpio);
	}
	return rc;
}

int lgd_fwvga_video_mdss_panel_parse_dts(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	return 0;
}

int lgd_fwvga_video_panel_device_create(struct device_node *node,
	struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{

	return 0;
}
#elif defined (CONFIG_LGD_DB7400_HD_VIDEO_PANEL)
int lgd_db7400_hd_video_mdss_dsi_request_gpios(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc = 0;
	struct mdss_panel_info *pinfo = NULL;
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		rc = gpio_request(ctrl_pdata->disp_en_gpio,
						"disp_enable");
		if (rc) {
			pr_err("request disp_en gpio failed, rc=%d\n",
				       rc);
		}
	}

	rc = gpio_request(ctrl_pdata->rst_gpio, "disp_rst_n");
	if (rc) {
		pr_err("request reset gpio failed, rc=%d\n",
			rc);
		goto rst_gpio_err;
	}

	if (gpio_is_valid(ctrl_pdata->touch_en_gpio)) {
		rc = gpio_request(ctrl_pdata->touch_en_gpio,
				"touch_enable");
		if (rc)
			pr_err("request touch_enable gpio failed, rc=%d\n", rc);
	}

	return rc;

rst_gpio_err:
	if (gpio_is_valid(ctrl_pdata->disp_en_gpio))
		gpio_free(ctrl_pdata->disp_en_gpio);
	return rc;

}

int lgd_db7400_hd_video_mdss_dsi_panel_reset(struct mdss_panel_data *pdata, int enable)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;
	int i, rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (!gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		pr_debug("%s:%d, enable gpio is not configured\n",
			   __func__, __LINE__);
	}

	if (!gpio_is_valid(ctrl_pdata->rst_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return rc;
	}

	pinfo = &(ctrl_pdata->panel_data.panel_info);
	pr_info("%s: enable = %d\n", __func__, enable);

	if (enable) {
		if (lge_mdss_dsi.lge_mdss_dsi_request_gpios)
			lge_mdss_dsi.lge_mdss_dsi_request_gpios(ctrl_pdata);
		if (rc) {
			pr_err("gpio request failed\n");
			return rc;
		}

		if (!pinfo->cont_splash_enabled) {
			gpio_set_value((ctrl_pdata->rst_gpio), 0);
			PANEL_SEQUENCE(PIN_RESET, 0);
			mdelay(5);

			for (i = 0; i < pdata->panel_info.rst_seq_len; ++i) {
				gpio_set_value((ctrl_pdata->rst_gpio),
						pdata->panel_info.rst_seq[i]);
				PANEL_SEQUENCE(PIN_RESET, pdata->panel_info.rst_seq[i]);
				if (pdata->panel_info.rst_seq[++i])
					usleep(pinfo->rst_seq[i] * 1000);
			}

			if (gpio_is_valid(ctrl_pdata->touch_en_gpio)){
				gpio_set_value((ctrl_pdata->touch_en_gpio), 0);
				mdelay(1);
				gpio_set_value((ctrl_pdata->touch_en_gpio), 1);
				mdelay(10);
			}
		}

		if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
			pr_debug("%s: Panel Not properly turned OFF\n",
						__func__);
			ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
			pr_debug("%s: Reset panel done\n", __func__);
		}
	} else {
		if (oem_shutdown_pending) {
			gpio_set_value((ctrl_pdata->rst_gpio), 0);
			PANEL_SEQUENCE(PIN_RESET, 0);
			gpio_free(ctrl_pdata->rst_gpio);
		}
	}

	return rc;

}

int lgd_db7400_hd_video_mdss_panel_parse_dts(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	return 0;
}

int lgd_db7400_hd_video_panel_device_create(struct device_node *node,
	struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	return 0;
}
#elif defined(CONFIG_TCL_ILI9806E_FWVGA_VIDEO_PANEL)
int tcl_ili9806e_fwvga_video_mdss_dsi_request_gpios(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc = 0;
	struct mdss_panel_info *pinfo = NULL;
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		rc = gpio_request(ctrl_pdata->disp_en_gpio,
						"disp_enable");
		if (rc) {
			pr_err("request disp_en gpio failed, rc=%d\n",
				       rc);
			goto disp_en_gpio_err;
		}
	}

	rc = gpio_request(ctrl_pdata->rst_gpio, "disp_rst_n");
	if (rc) {
		pr_err("request reset gpio failed, rc=%d\n",
			rc);
		goto rst_gpio_err;
	}

	if (gpio_is_valid(ctrl_pdata->bklt_en_gpio)) {
		rc = gpio_request(ctrl_pdata->bklt_en_gpio,
						"bklt_enable");
		if (rc) {
			pr_err("request bklt gpio failed, rc=%d\n",
				       rc);
			goto bklt_en_gpio_err;
		}
	}
	if (gpio_is_valid(ctrl_pdata->mode_gpio)) {
		rc = gpio_request(ctrl_pdata->mode_gpio, "panel_mode");
		if (rc) {
			pr_err("request panel mode gpio failed,rc=%d\n",
								rc);
			goto mode_gpio_err;
		}
	}
	return rc;

mode_gpio_err:
	if (gpio_is_valid(ctrl_pdata->bklt_en_gpio))
		gpio_free(ctrl_pdata->bklt_en_gpio);
bklt_en_gpio_err:
	gpio_free(ctrl_pdata->rst_gpio);
rst_gpio_err:
	if (gpio_is_valid(ctrl_pdata->disp_en_gpio))
		gpio_free(ctrl_pdata->disp_en_gpio);
disp_en_gpio_err:
	return rc;

}

int tcl_ili9806e_fwvga_video_mdss_dsi_panel_reset(struct mdss_panel_data *pdata, int enable)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;
	int i, rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (!gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
	}

	if (!gpio_is_valid(ctrl_pdata->rst_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return rc;
	}

	pr_info("%s: enable = %d\n", __func__, enable);
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (enable) {
		if (lge_mdss_dsi.lge_mdss_dsi_request_gpios)
			lge_mdss_dsi.lge_mdss_dsi_request_gpios(ctrl_pdata);
		if (rc) {
			pr_err("gpio request failed\n");
			return rc;
		}

		if (!pinfo->cont_splash_enabled) {
			if (gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
				gpio_set_value((ctrl_pdata->disp_en_gpio), 1);
				PANEL_SEQUENCE(PIN_DDVDH, 1);
			}

		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		PANEL_SEQUENCE(PIN_RESET, 0);
		mdelay(5);

		for (i = 0; i < pdata->panel_info.rst_seq_len; ++i) {
			gpio_set_value((ctrl_pdata->rst_gpio),
				pdata->panel_info.rst_seq[i]);
				PANEL_SEQUENCE(PIN_RESET, pdata->panel_info.rst_seq[i]);
			if (pdata->panel_info.rst_seq[++i])
				usleep(pinfo->rst_seq[i] * 1000);
		}

		if (gpio_is_valid(ctrl_pdata->bklt_en_gpio))
			gpio_set_value((ctrl_pdata->bklt_en_gpio), 1);
		}

		if (gpio_is_valid(ctrl_pdata->mode_gpio)) {
			if (pinfo->mode_gpio_state == MODE_GPIO_HIGH)
				gpio_set_value((ctrl_pdata->mode_gpio), 1);
			else if (pinfo->mode_gpio_state == MODE_GPIO_LOW)
				gpio_set_value((ctrl_pdata->mode_gpio), 0);
		}
		if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
			pr_debug("%s: Panel Not properly turned OFF\n",
						__func__);
			ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
			pr_debug("%s: Reset panel done\n", __func__);
		}
	} else {
		if (gpio_is_valid(ctrl_pdata->bklt_en_gpio)) {
			gpio_set_value((ctrl_pdata->bklt_en_gpio), 0);
			gpio_free(ctrl_pdata->bklt_en_gpio);
		}

		if (gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
			gpio_set_value((ctrl_pdata->disp_en_gpio), 0);
			PANEL_SEQUENCE(PIN_DDVDH, 0);
			gpio_free(ctrl_pdata->disp_en_gpio);
		}

		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		PANEL_SEQUENCE(PIN_RESET, 0);
		gpio_free(ctrl_pdata->rst_gpio);

		if (gpio_is_valid(ctrl_pdata->mode_gpio))
			gpio_free(ctrl_pdata->mode_gpio);
	}
	return rc;

}

int tcl_ili9806e_fwvga_video_mdss_panel_parse_dts(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	return 0;
}

int tcl_ili9806e_fwvga_video_panel_device_create(struct device_node *node,
	struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{

	return 0;
}
#endif

#if IS_ENABLED(CONFIG_LGE_READER_MODE)
int lge_mdss_dsi_parse_reader_mode_cmds(struct device_node *np, struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_step1_cmds,
		"qcom,panel-reader-mode-step1-command", "qcom,mdss-dsi-reader-mode-command-state");
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_step2_cmds,
		"qcom,panel-reader-mode-step2-command", "qcom,mdss-dsi-reader-mode-command-state");
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_step3_cmds,
		"qcom,panel-reader-mode-step3-command", "qcom,mdss-dsi-reader-mode-command-state");
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_off_cmds,
		"qcom,panel-reader-mode-off-command", "qcom,mdss-dsi-reader-mode-command-state");
	//mdss_dsi_parse_dcs_cmds(np, &reader_mode_mono_enable_cmds,
		//"qcom,panel-reader-mode-mono-enable-command", "qcom,mdss-dsi-reader-mode-command-state");
	//mdss_dsi_parse_dcs_cmds(np, &reader_mode_mono_disable_cmds,
		//"qcom,panel-reader-mode-mono-disable-command", "qcom,mdss-dsi-reader-mode-command-state");
	return 0;
}

int lge_mdss_dsi_panel_send_on_cmds(struct mdss_dsi_ctrl_pdata *ctrl, struct dsi_panel_cmds *default_on_cmds, int cur_mode)
{
	if (default_on_cmds->cmd_cnt) {
		msleep(50);
		mdss_dsi_panel_cmds_send(ctrl, default_on_cmds);
	} else if (cur_mode != READER_MODE_OFF) {
		msleep(50);
	}

	pr_info("%s: reader_mode (%d).\n", __func__, cur_mode);

	switch(cur_mode)
	{
		case READER_MODE_STEP_1:
			pr_info("%s: reader_mode STEP1\n", __func__);
			if (reader_mode_step1_cmds.cmd_cnt) {
				pr_info("%s: reader_mode_step1_cmds: cnt = %d \n",
					__func__, reader_mode_step1_cmds.cmd_cnt);
				mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step1_cmds);
			}
			break;
		case READER_MODE_STEP_2:
			pr_info("%s: reader_mode STEP2\n", __func__);
			if (reader_mode_step2_cmds.cmd_cnt) {
				pr_info("%s: reader_mode_step2_cmds: cnt = %d \n",
					__func__, reader_mode_step2_cmds.cmd_cnt);
				mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step2_cmds);
			}
			break;
		case READER_MODE_STEP_3:
			pr_info("%s: reader_mode STEP3\n", __func__);
			if (reader_mode_step3_cmds.cmd_cnt) {
				pr_info("%s: reader_mode_step3_cmds: cnt = %d \n",
					__func__, reader_mode_step3_cmds.cmd_cnt);
				mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step3_cmds);
			}
			break;
		case READER_MODE_MONO:
			pr_info("%s: reader_mode MONO \n", __func__);
			/*
			if(lge_get_panel_type() == PH2_JDI) {
				if (reader_mode_initial_mono_enable_cmds.cmd_cnt) {
					pr_info("%s: reader_mode_mono_enable_cmds: cnt = %d \n",
						__func__, reader_mode_initial_mono_enable_cmds.cmd_cnt);
					mdss_dsi_panel_cmds_send(ctrl, &reader_mode_initial_mono_enable_cmds, CMD_REQ_COMMIT);
				}
			} else if(lge_get_panel_type() == PH2_SHARP) {
				if (reader_mode_initial_step2_cmds.cmd_cnt) {
					// 5500K
					pr_info("%s: reader_mode_initial_step2_cmds: cnt = %d \n",
						__func__, reader_mode_initial_step2_cmds.cmd_cnt);
					mdss_dsi_panel_cmds_send(ctrl, &reader_mode_initial_step2_cmds, CMD_REQ_COMMIT);
				}
			}
			*/
			if (reader_mode_step2_cmds.cmd_cnt) {
				pr_info("%s: reader_mode_step2_cmds: cnt = %d \n",
					__func__, reader_mode_step2_cmds.cmd_cnt);
				mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step2_cmds);
			}
			break;
		case READER_MODE_OFF:
		default:
			break;
	}

	return 0;
}

bool lge_change_reader_mode(struct mdss_dsi_ctrl_pdata *ctrl, int old_mode, int new_mode)
{
	if(new_mode == READER_MODE_OFF) {
		switch(old_mode)
		{
			case READER_MODE_STEP_1:
			case READER_MODE_STEP_2:
			case READER_MODE_STEP_3:
				if(reader_mode_off_cmds.cmd_cnt) {
					pr_info("%s: sending reader mode OFF commands: cnt = %d\n",
						__func__, reader_mode_off_cmds.cmd_cnt);
					//mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);
					mdss_dsi_panel_cmds_send(ctrl, &reader_mode_off_cmds);
					//mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
				}
			    break;
			case READER_MODE_MONO:
				if(reader_mode_off_cmds.cmd_cnt) {
					pr_info("%s: sending reader mode OFF commands: cnt = %d\n",
						__func__, reader_mode_off_cmds.cmd_cnt);
					//mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);
					mdss_dsi_panel_cmds_send(ctrl, &reader_mode_off_cmds);
					//mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
				}
				break;
			default:
				pr_err("%s: Invalid old status : %d\n", __func__, old_mode);
				break;
		}
	} else {
		switch(old_mode)
		{
			case READER_MODE_OFF:
			case READER_MODE_STEP_1:
			case READER_MODE_STEP_2:
			case READER_MODE_STEP_3:
				if(old_mode == new_mode)
				{
					pr_info("%s: Same as older mode\n", __func__);
					break;
				}
				//mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);
				switch(new_mode)
				{
					case READER_MODE_STEP_1:
						if(reader_mode_step1_cmds.cmd_cnt) {
							pr_info("%s: sending reader mode STEP1 commands: cnt = %d\n",
								__func__, reader_mode_step1_cmds.cmd_cnt);
							mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step1_cmds);
						}
						break;
					case READER_MODE_STEP_2:
						if(reader_mode_step2_cmds.cmd_cnt) {
							pr_info("%s: sending reader mode STEP2 commands: cnt = %d\n",
								__func__, reader_mode_step2_cmds.cmd_cnt);
							mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step2_cmds);
						}
						break;
					case READER_MODE_STEP_3:
						if (reader_mode_step3_cmds.cmd_cnt) {
							pr_info("%s: sending reader mode STEP3 commands: cnt = %d\n",
								__func__, reader_mode_step3_cmds.cmd_cnt);
							mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step3_cmds);
						}
						break;
					case READER_MODE_MONO:
						// D-IC does not support MONO Mode
						if (reader_mode_step2_cmds.cmd_cnt) {
							pr_info("%s: sending reader mode STEP2 commands: cnt : %d \n",
								__func__, reader_mode_step2_cmds.cmd_cnt);
							mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step2_cmds);
						}
						break;
					default:
						pr_err("%s: Input Invalid parameter: %d \n", __func__, new_mode);
						break;
				}
				//mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
				break;
			case READER_MODE_MONO:
				//mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);
				switch(new_mode)
				{
					case READER_MODE_STEP_1:
						if (reader_mode_step1_cmds.cmd_cnt) {
							pr_info("%s: sending reader mode STEP1 commands: cnt : %d \n",
								__func__, reader_mode_step1_cmds.cmd_cnt);
							mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step1_cmds);
						}
						break;
					case READER_MODE_STEP_2:
						if (reader_mode_step2_cmds.cmd_cnt) {
							pr_info("%s: sending reader mode STEP2 commands: cnt : %d \n",
								__func__, reader_mode_step2_cmds.cmd_cnt);
							mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step2_cmds);
						}
						break;
					case READER_MODE_STEP_3:
						if (reader_mode_step3_cmds.cmd_cnt) {
							pr_info("%s: sending reader mode STEP3 commands: cnt = %d \n",
								__func__, reader_mode_step3_cmds.cmd_cnt);
							mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step3_cmds);
						}
						break;
					case READER_MODE_MONO:
						break;
					default:
						pr_err("%s: Input Invalid parameter : %d \n", __func__, new_mode);
						break;
				}
				//mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
				break;
			default:
				pr_err("%s: Invalid old status : %d\n", __func__, old_mode);
				break;
		}
	}

	return true;
}
#endif
