/***************************************************************************
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *    File  	: lgtp_platform_api.c
 *    Author(s)   : D3 BSP Touch Team < d3-bsp-touch@lge.com >
 *    Description :
 *
 ***************************************************************************/
#define LGTP_MODULE "[PLATFORM]"

/****************************************************************************
* Include Files
****************************************************************************/
#include <linux/input/unified_driver_2/lgtp_common.h>

#include <linux/input/unified_driver_2/lgtp_common_driver.h>
#include <linux/input/unified_driver_2/lgtp_platform_api.h>
#include <linux/input/unified_driver_2/lgtp_model_config.h>
#include <soc/qcom/lge/lge_boot_mode.h>

/****************************************************************************
* Manifest Constants / Defines
****************************************************************************/

/****************************************************************************
 * Macros
 ****************************************************************************/

/****************************************************************************
* Type Definitions
****************************************************************************/

/****************************************************************************
* Variables
****************************************************************************/
static int nIrq_num;
static atomic_t irq_state = ATOMIC_INIT(1);

/****************************************************************************
* Extern Function Prototypes
****************************************************************************/


/****************************************************************************
* Local Function Prototypes
****************************************************************************/


/****************************************************************************
* Local Functions
****************************************************************************/
#if 0 /* TBD */
#if defined ( TOUCH_PLATFORM_QCT )
#define istate core_internal_state__do_not_mess_with_it
#define IRQS_PENDING 0x00000200
#endif
static void TouchClearPendingIrq(void)
{
	#if defined ( TOUCH_PLATFORM_QCT )
	unsigned long flags;
	struct irq_desc *desc = irq_to_desc(nIrq_num);

	if (desc) {
		raw_spin_lock_irqsave(&desc->lock, flags);
		desc->istate &= ~IRQS_PENDING;
		raw_spin_unlock_irqrestore(&desc->lock, flags);
	}
	#endif
}
#endif

static void TouchMaskIrq(void)
{
	TOUCH_WARN("Insider TouchMaskIrq\n");
	#if defined (TOUCH_DEVICE_LU202X)
	struct irq_desc *desc = irq_to_desc(nIrq_num);
	
	if(desc->irq_data.chip->irq_mask) {
		desc->irq_data.chip->irq_mask(&desc->irq_data);
	}
	#endif
}

static void TouchUnMaskIrq(void)
{
	TOUCH_WARN("Inside TouchUnMaskIrq\n");
	#if defined (TOUCH_DEVICE_LU202X)
	struct irq_desc *desc = irq_to_desc(nIrq_num);
	
	if(desc->irq_data.chip->irq_unmask) {
		desc->irq_data.chip->irq_unmask(&desc->irq_data);
	}
	#endif
}

static int i2c_read(struct i2c_client *client, u8 *reg, int regLen, u8 *buf, int dataLen)
{
	int result = TOUCH_SUCCESS;
	int ret = 0;
	
	struct i2c_msg msgs[2] = {
		{ .addr = client->addr, .flags = 0, .len = regLen, .buf = reg, },
		{ .addr = client->addr, .flags = I2C_M_RD, .len = dataLen, .buf = buf, },
	};

	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret < 0)
	{
		result = TOUCH_FAIL;
	}

	return result;
}

static int i2c_write(struct i2c_client *client, u8 *reg, int regLen, u8 *buf, int dataLen)
{
	int result = TOUCH_SUCCESS;
	int ret = 0;

	struct i2c_msg msg = {
		.addr = client->addr, .flags = 0, .len = (regLen+dataLen), .buf = NULL,
	};

	u8 *pTmpBuf = NULL;

	 pTmpBuf = (u8 *)kcalloc(1, regLen+dataLen, GFP_KERNEL);
	 if(pTmpBuf != NULL) {
		 memset(pTmpBuf, 0x00, regLen+dataLen);
	 } else {
		 return TOUCH_FAIL;
	 }

	memcpy(pTmpBuf, reg, regLen);
	memcpy((pTmpBuf+regLen), buf, dataLen);

	msg.buf = pTmpBuf;
	
	ret = i2c_transfer(client->adapter, &msg, 1);

	if (ret < 0) {
		result = TOUCH_FAIL;
	}

	kfree(pTmpBuf);
	
	return result;
	
}

/****************************************************************************
* Global Functions
****************************************************************************/

#if defined (TOUCH_DEVICE_MIT200_PROC4)
int Mit200_proc4_I2C_Read ( struct i2c_client *client, u8 *addr, u8 addrLen, u8 *rxbuf, int len )
{
	int ret = 0;

	ret = i2c_read(client, addr, addrLen, rxbuf, len);
	if( ret == TOUCH_FAIL ) {
		if (printk_ratelimit()) {
			TOUCH_ERR("failed to read i2c ( reg = %d )\n", (u16)((addr[0]<<7)|addr[1]));
		}
		return TOUCH_FAIL;
	}

	return TOUCH_SUCCESS;
}

int Mit200_proc4_I2C_Write ( struct i2c_client *client, u8 *writeBuf, u32 write_len )
{
	int ret = 0;

	ret = i2c_write(client, writeBuf, 2, writeBuf+2, write_len-2);
	if( ret == TOUCH_FAIL ) {
		if (printk_ratelimit()) {
			TOUCH_ERR("failed to write i2c ( reg = %d )\n", (u16)((writeBuf[0]<<7)|writeBuf[1]));
		}
		return TOUCH_FAIL;
	}

	return TOUCH_SUCCESS;
}
#endif

int Touch_I2C_Read_Byte ( struct i2c_client *client, u8 addr, u8 *rxbuf )
{
	int ret = 0;
	u8 regValue = addr;

	ret = i2c_read(client, &regValue, 1, rxbuf, 1);
	if( ret == TOUCH_FAIL ) {
		if (printk_ratelimit()) {
			TOUCH_ERR("failed to read i2c byte ( reg = %d )\n", addr);
		}
		return TOUCH_FAIL;
	}

	return TOUCH_SUCCESS;	
}

int Touch_I2C_Write_Byte ( struct i2c_client *client, u8 addr, u8 txbuf )
{
	int ret = 0;
	u8 regValue = addr;
	u8 data = txbuf;

	ret = i2c_write(client, &regValue, 1, &data, 1);
	if( ret == TOUCH_FAIL ) {
		if (printk_ratelimit()) {
			TOUCH_ERR("failed to write i2c byte ( reg = %d )\n", addr);
		}
		return TOUCH_FAIL;
	}

	return TOUCH_SUCCESS;	
}

int Touch_I2C_Read ( struct i2c_client *client, u8 addr, u8 *rxbuf, int len )
{
	int ret = 0;
	u8 regValue = addr;

	ret = i2c_read(client, &regValue, 1, rxbuf, len);
	if( ret == TOUCH_FAIL ) {
		if (printk_ratelimit()) {
			TOUCH_ERR("failed to read i2c ( reg = %d )\n", addr);
		}
		return TOUCH_FAIL;
	}
	
	return TOUCH_SUCCESS;	
}

int Touch_I2C_Write ( struct i2c_client *client, u8 addr, u8 *txbuf, int len )
{
	int ret = 0;
	u8 regValue = addr;

	ret = i2c_write(client, &regValue, 1, txbuf, len);
	if( ret == TOUCH_FAIL ) {
		if (printk_ratelimit()) {
			TOUCH_ERR("failed to write i2c ( reg = %d )\n", addr);
		}
		return TOUCH_FAIL;
	}
	
	return TOUCH_SUCCESS;	
}

#if 1 /* LGE_BSP_COMMON : branden.you@lge.com_20141105 : */
int touch_i2c_read_for_query(int slave_addr, u8 *reg, u8 regLen, u8 *buf, u8 dataLen)
{
	int ret = 0;
 	struct i2c_adapter *adap = NULL;

	struct i2c_msg msgs[2] = {
		{ .addr = slave_addr, .flags = 0, .len = regLen, .buf = reg, },
		{ .addr = slave_addr, .flags = I2C_M_RD, .len = dataLen, .buf = buf, },
	};

	adap = i2c_get_adapter(TOUCH_I2C_BUS_NUM);
	if ( adap == NULL ) {
		return TOUCH_FAIL;
	}
	
	ret = i2c_transfer(adap, msgs, 2);
	if (ret < 0)
	{
		if (printk_ratelimit())
		{
			TOUCH_ERR("failed to read i2c data ( error = %d )\n", ret);
		}
		return TOUCH_FAIL;
	}

	return TOUCH_SUCCESS;

 }
#endif

int TouchInitializePlatform( void )
{
	int ret = 0;

	TOUCH_FUNC();

	ret = gpio_request(TOUCH_GPIO_RESET, "touch_reset");
	if (ret < 0) {
		TOUCH_ERR("FAIL: touch reset gpio_request\n");
		if (ret < 0) {
			return TOUCH_FAIL;
		}
	}

	gpio_direction_output(TOUCH_GPIO_RESET, 1);

    #if defined (SENSOR_DETECT)
    ret = gpio_request(TOUCH_GPIO_CONTROL, "sensor_change");
    if (ret < 0) {
		TOUCH_ERR("FAIL: touch reset gpio_request\n");
		if( ret < 0) {
        	return TOUCH_FAIL;
    	}
	}
    gpio_direction_output(TOUCH_GPIO_CONTROL, 1);
    #endif

	ret = gpio_request(TOUCH_GPIO_INTERRUPT, "touch_int");
	if (ret < 0) {
		TOUCH_ERR("FAIL: touch int gpio_request\n");
		if(ret < 0) {
			return TOUCH_FAIL;
		}
	}

	gpio_direction_input(TOUCH_GPIO_INTERRUPT);

	#if defined (TOUCH_GPIO_MAKER_ID)
	ret = gpio_request(TOUCH_GPIO_MAKER_ID, "touch_maker");
	if (ret < 0) {
		TOUCH_ERR("FAIL: touch make id gpio_request\n");
		if( ret < 0) {
			return TOUCH_FAIL;
		}
	}

	gpio_direction_input(TOUCH_GPIO_MAKER_ID);
	#endif

	return TOUCH_SUCCESS;
	
}

int TouchPlatformFree(void)
{
	gpio_free(TOUCH_GPIO_RESET);
	gpio_free(TOUCH_GPIO_INTERRUPT);
#if defined (TOUCH_GPIO_MAKER_ID)
	gpio_free(TOUCH_GPIO_MAKER_ID);
#endif
	return 0;
}

void TouchPower(int isOn)
{
	/* to avoid prequently change of this file */
	TouchPowerModel(isOn);
}

void TouchAssertReset(void)
{
	/* to avoid prequently change of this file */
	TouchAssertResetModel();
}

void TouchSleepCtrl(int isHigh)
{
    if(isHigh) {
        gpio_set_value(TOUCH_GPIO_CONTROL, 1);
    } else {
        gpio_set_value(TOUCH_GPIO_CONTROL, 0);
    }

}

void TouchResetCtrl(int isHigh)
{
	
	if (isHigh) {
		gpio_set_value(TOUCH_GPIO_RESET, 1);
		TOUCH_LOG("Reset Pin was set to HIGH\n");
	} else {
		gpio_set_value(TOUCH_GPIO_RESET, 0);
		
		TOUCH_LOG("Reset Pin was set to LOW\n");
	}
	
}

int TouchRegisterIrq(TouchDriverData *pDriverData, irq_handler_t irqHandler)
{
	TOUCH_FUNC();

	int ret;

	ret = request_irq(pDriverData->client->irq, irqHandler, TOUCH_IRQ_FLAGS, pDriverData->client->name, pDriverData);
	if (ret < 0) {
		TOUCH_ERR("failed at request_irq() ( error = %d )\n", ret );
		return TOUCH_FAIL;
	}
	enable_irq_wake(pDriverData->client->irq);

	nIrq_num = pDriverData->client->irq;

	return TOUCH_SUCCESS;
	
}

void TouchEnableIrq(void)
{
	TouchUnMaskIrq();

	//TouchClearPendingIrq(); // TBD

	if (atomic_read(&irq_state) != 1) {
		enable_irq(nIrq_num);
		atomic_set(&irq_state, 1);
	}

	TOUCH_LOG("Interrupt Enabled\n");
	
}

void TouchDisableIrq(void)
{
	if (atomic_read(&irq_state) != 0) {
		disable_irq(nIrq_num);
		atomic_set(&irq_state, 0);
	}

	TouchMaskIrq();

	TOUCH_LOG("Interrupt Disabled\n");
}

int TouchReadMakerId(void)
{
	int pinState = 1; /* default value is "ONE" */
	
	#if defined (TOUCH_GPIO_MAKER_ID)
	pinState = gpio_get_value(TOUCH_GPIO_MAKER_ID);
	#endif

	return pinState;
}

int TouchReadInterrupt(void)
{
	int gpioState = 0;

	gpioState = gpio_get_value(TOUCH_GPIO_INTERRUPT);

	return gpioState;
}

int TouchGetBootMode(void)
{
	int mode = BOOT_NORMAL;
	
	enum lge_boot_mode_type lge_boot_mode;
	
	lge_boot_mode = lge_get_boot_mode();
	
	if (lge_boot_mode == LGE_BOOT_MODE_CHARGERLOGO) {
		mode = BOOT_OFF_CHARGING;
	} else if (lge_boot_mode == LGE_BOOT_MODE_MINIOS) {
		mode = BOOT_MINIOS;
	} else {
		mode = BOOT_NORMAL;
	}

	return mode;
	
}


/* End Of File */
