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
 *    File  	: lgtp_common.h
 *    Author(s)   : D3 BSP Touch Team < d3-bsp-touch@lge.com >
 *    Description :
 *
 ***************************************************************************/

#if !defined ( _LGTP_COMMON_H_ )
#define _LGTP_COMMON_H_

/****************************************************************************
* Nested Include Files
****************************************************************************/
#include <linux/types.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/jiffies.h>
#include <linux/version.h>

#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/platform_device.h>

#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>

#include <linux/mutex.h>
#include <linux/async.h>
#include <linux/atomic.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>

#include <linux/firmware.h>

#if defined ( CONFIG_HAS_EARLYSUSPEND )
#include <linux/earlysuspend.h>
#elif defined (CONFIG_FB)
#include <linux/notifier.h>
#include <linux/fb.h>
#endif

/****************************************************************************
* Mainfest Constants / Defines
****************************************************************************/
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 4, 67))
#define KERNEL_ABOVE_3_4_67
#endif

#include <linux/input/unified_driver_2/lgtp_project_setting.h>
#include <soc/qcom/lge/lge_boot_mode.h>

/*TBD - NSM Don't have below file*/
//#include <mach/board.h>
//#include <mach/board_lge.h>

//==========================================================
// Driver Capability
//==========================================================
#define MAX_DEVICE_SUPPORT 5

#define MAX_FINGER	10
#define MAX_KEY		4
#define MAX_KNOCK	16

#define MAX_FILENAME 256

#define MAX_FW_UPGRADE_RETRY 3

#define SENSOR_DETECT

/****************************************************************************
* Type Definitions
****************************************************************************/
enum {
	TOUCH_FALSE = 0,
	TOUCH_TRUE,
};

enum {
	TOUCH_SUCCESS = 0,
	TOUCH_FAIL = -1,
};

enum {
	MT_PROTOCOL_A = 0,
	MT_PROTOCOL_B,
};

enum {
	KEY_RELEASED = 0,
	KEY_PRESSED,
	KEY_CANCELED = 0xFF,
};

enum {
	HOVER_FAR = 0,
	HOVER_NEAR,
};

enum {
	UEVENT_IDLE = 0,
	UEVENT_BUSY,
};

enum {
	READ_IC_REG = 0,
	WRITE_IC_REG,
};

enum {
	BOOT_NORMAL = 0,
	BOOT_OFF_CHARGING,
	BOOT_MINIOS,
};

enum {
	PM_RESUME = 0,
	PM_SUSPEND,
	PM_SUSPEND_IRQ,
};

enum {
	LPWG_MODE_UNKNOWN = -1,
		
	LPWG_MODE_OFF = 0x0,
	LPWG_MODE_KNOCK_ON = 0x1,
	LPWG_MODE_KNOCK_CODE = 0x2,
	LPWG_MODE_SIGNATURE = 0x4,
};

enum {
	LCD_OFF = 0,
	LCD_ON,
};

enum {
	PROXYMITY_NEAR = 0,
	PROXYMITY_FAR,
};

enum {
	FINGER_RELEASED	= 0,
	FINGER_PRESSED	= 1,
	FINGER_UNUSED   = 2,
};

enum {
	COVER_OPENED = 0,
	COVER_CLOSED,
};

enum {
	CALL_END = 0,
	CALL_RINGING,
	CALL_RECEIVED,
};

typedef enum
{
	NOTIFY_UNKNOWN = 0,
	
	NOTIFY_CALL,
	NOTIFY_Q_COVER,

} TouchNotify;

typedef enum
{
	STATE_UNKNOWN = 0,
	
	STATE_BOOT,
	STATE_NORMAL,
	STATE_OFF,
	STATE_KNOCK_ON_ONLY,
	STATE_KNOCK_ON_CODE,
	STATE_NORMAL_HOVER,
	STATE_HOVER,
	STATE_UPDATE_FIRMWARE,
	STATE_SELF_DIAGNOSIS,

} TouchState;

typedef struct TouchModelConfigTag
{
	u32	button_support;
	u32	number_of_button;
	u32	button_name[MAX_KEY];
	u32	protocol_type;
	u32	max_x;
	u32	max_y;
	u32	max_pressure;
	u32	max_width;
	u32	max_orientation;
	u32	max_id;
	
} TouchModelConfig;

typedef struct TouchDeviceInfoTag
{
	u16 moduleMakerID;
	u16 moduleVersion;
	u16 modelID;
	u16 isOfficial;
	u16 version;
	u16 productID;
	u8 checksum[6];
	
} TouchFirmwareInfo;

typedef enum
{
	LPWG_CMD_UNKNOWN = 0,
	
	LPWG_CMD_MODE = 1,
	LPWG_CMD_LCD_PIXEL_SIZE = 2,
	LPWG_CMD_ACTIVE_TOUCH_AREA = 3,
	LPWG_CMD_TAP_COUNT = 4,
	LPWG_CMD_TAP_DISTANCE = 5,
	LPWG_CMD_LCD_STATUS = 6,
	LPWG_CMD_PROXIMITY_STATUS = 7,
	LPWG_CMD_FIRST_TWO_TAP = 8,
	LPWG_CMD_UPDATE_ALL = 9,
	LPWG_CMD_CALL = 10,
	
} LpwgCmd;

typedef struct LpwgSettingTag
{
	unsigned int mode; 		/* LPWG mode ( Bit field ) : 0 = disable, 1 = knock-on, 2 = knock-code, 4 = Signature(Not Used) */
	unsigned int lcdPixelSizeX;
	unsigned int lcdPixelSizeY;
	unsigned int activeTouchAreaX1;
	unsigned int activeTouchAreaX2;
	unsigned int activeTouchAreaY1;
	unsigned int activeTouchAreaY2;
	unsigned int tapCount; 	/* knock-code length */
	unsigned int isFirstTwoTapSame; 	/* 1 = first two code is same, 0 = not same */
	unsigned int lcdState; 		/* 1 = LCD ON, 0 = OFF */
	unsigned int proximityState; 	/* 1 = Proximity FAR, 0 = NEAR */
	unsigned int coverState; 	/* 1 = Cover Close, 0 = Open*/
	unsigned int callState; 	/* 0 = Call End, 1 = Ringing, 2 = Call Received */
	
} LpwgSetting;

typedef enum
{
	DATA_UNKNOWN = 0,
		
	DATA_FINGER,
	DATA_KEY,
	DATA_KNOCK_ON,
	DATA_KNOCK_CODE,
	DATA_HOVER,
#if defined(ENABLE_SWIPE_MODE)
	DATA_SWIPE,
#endif
} TouchDataType;

typedef struct TouchFingerDataTag
{
	u16	id; /* finger ID */
	u16	x;
	u16	y;
	u16	width_major;
	u16	width_minor;
	u16	orientation;
	u16	pressure; /* 0=Hover / 1~MAX-1=Finger / MAX=Palm */
	u16	type; /* finger, palm, pen, glove, hover */
	u16 status;
	
} TouchFingerData;

typedef struct TouchKeyDataTag
{
	u16	index; 		/* key index ( should be assigned from 1 ) */
	u16	pressed; 	/* 1 means pressed, 0 means released */
	
} TouchKeyData;

typedef struct TouchPointTag
{
	int x;
	int y;
	
} TouchPoint;

typedef struct TouchReadDataTag
{
	TouchDataType type;
	u16 count;
	TouchFingerData fingerData[MAX_FINGER+1];
	TouchKeyData keyData;
	TouchPoint knockData[MAX_KNOCK+1];
	int hoverState;
	
} TouchReadData;

typedef struct TouchReportDataTag
{
	u32 finger; 	/* bit field for each finger : 1 means pressed, 0 means released */
	u32 key; 	/* key index ( 0 means all key was released ) */
	u32 knockOn; 	/* 1 means event sent, 0 means event processed by CFW */
	u32 knockCode; 	/* 1 means event sent, 0 means event processed by CFW */
	u32 hover;	/* 0 means far, near means 1 */
	u32 knockCount;
	TouchPoint knockData[MAX_KNOCK+1];
	
} TouchReportData;

typedef struct TouchDriverDataTag
{
	struct input_dev		*input_dev;
	struct i2c_client		*client;
	struct kobject 		lge_touch_kobj;

	#if defined ( CONFIG_HAS_EARLYSUSPEND )
	struct early_suspend	early_suspend;
	#elif defined ( CONFIG_FB )
	struct notifier_block	fb_notif;
	#endif

	struct delayed_work		work_upgrade;
	struct delayed_work 	work_irq;
	struct delayed_work 	work_init;
	struct delayed_work		work_power_off_charging;
	struct mutex			thread_lock;
	struct wake_lock		lpwg_wake_lock;

	int isSuspend;
	int bootMode;
	int mfts_enable;
	
	TouchState currState;
	TouchState nextState;

	TouchModelConfig mConfig;

	LpwgSetting lpwgSetting;

	TouchFirmwareInfo icFwInfo;
	TouchFirmwareInfo binFwInfo;

	int useDefaultFirmware;
	char fw_image[MAX_FILENAME];

	TouchReportData reportData;

	u8 lpwg_debug_enable;

#if defined(SENSOR_DETECT)
	int sensor_value;
	int enable_sensor_interlock;
#endif
} TouchDriverData;

typedef struct TouchDeviceSpecificFuncTag {

	int (*Initialize)(struct i2c_client *client);
	void (*Reset)(struct i2c_client *client);
	int (*Connect)(void);
	int (*InitRegister)(struct i2c_client *client);
	void (*ClearInterrupt)(struct i2c_client *client);
	int (*InterruptHandler)(struct i2c_client *client, TouchReadData *pData);
	int (*ReadIcFirmwareInfo)(struct i2c_client *client, TouchFirmwareInfo *pFwInfo);
	int (*GetBinFirmwareInfo)(struct i2c_client *client, char *pFilename, TouchFirmwareInfo *pFwInfo);
	int (*UpdateFirmware)(struct i2c_client *client, char *pFilename);
	int (*SetLpwgMode)(struct i2c_client *client, TouchState newState, LpwgSetting *pLpwgSetting);
	int (*DoSelfDiagnosis)(struct i2c_client *client, int* pRawStatus, int* pChannelStatus, char* pBuf, int bufSize, int* pDataLen);
	#if defined( TOUCH_DEVICE_MIT200_PROC4 )
	int (*AccessRegister)(struct i2c_client *client, int cmd, int reg1, int reg2, int *pValue);
	#else
	int (*AccessRegister)(struct i2c_client *client, int cmd, int reg, int *pValue);
	#endif
	void (*NotifyHandler)(struct i2c_client *client, TouchNotify notify, int data);
	struct attribute **device_attribute_list;
	int (*GetProduct_id)(struct i2c_client *client, char *buf);
	int (*Getraw_data_result)(struct i2c_client *client, char *buf);
	void (*SetLpwwgDebug_enable)(struct i2c_client *client, int value);

#if defined(SENSOR_DETECT)
	void(*mcu_sleep_control)(struct i2c_client *client);
#endif
} TouchDeviceSpecificFunction;


/* sysfs
 *
 */
struct lge_touch_attribute {
	struct attribute	attr;
	ssize_t (*show)(struct i2c_client *client, char *buf);
	ssize_t (*store)(struct i2c_client *client,
		const char *buf, size_t count);
};

#define LGE_TOUCH_ATTR(_name, _mode, _show, _store)	\
struct lge_touch_attribute lge_touch_attr_##_name 	\
	= __ATTR(_name, _mode, _show, _store)



/****************************************************************************
* Exported Variables
****************************************************************************/
#if defined(SENSOR_DETECT)
//extern void apds9930_register_lux_change_callback (void (*callback) (int));
extern void rpr0521_register_proximity_change_callback (void (*callback) (int));
#endif
extern int palm_state;

/****************************************************************************
* Macros
****************************************************************************/
#define LGTP_DEBUG 1

#define LGTP_TAG "[TOUCH]"

/* LGTP_MODULE : will be defined in each c-files */
#if defined (LGTP_DEBUG)
#define TOUCH_ERR(fmt, args...)		printk(KERN_ERR LGTP_TAG"[E]"LGTP_MODULE" %s() line=%d : "fmt, __FUNCTION__, __LINE__, ##args)
#define TOUCH_WARN(fmt, args...)		printk(KERN_ERR LGTP_TAG"[W]"LGTP_MODULE" %s() line=%d : "fmt, __FUNCTION__, __LINE__, ##args)
#define TOUCH_LOG(fmt, args...)		printk(KERN_ERR LGTP_TAG"[L]"LGTP_MODULE" " fmt, ##args)
#define TOUCH_DBG(fmt, args...)		printk(KERN_ERR LGTP_TAG"[D]"LGTP_MODULE" " fmt, ##args)
#define TOUCH_FUNC(f)  	   			printk(KERN_ERR LGTP_TAG"[F]"LGTP_MODULE" %s()\n", __FUNCTION__)
#define TOUCH_FUNC_EXT(fmt, args...) 	printk(KERN_ERR LGTP_TAG"[F]"LGTP_MODULE" %s() : "fmt, __FUNCTION__, ##args)
#else
#define TOUCH_ERR(fmt, args...)		printk(KERN_ERR LGTP_TAG"[E]"LGTP_MODULE" %s() line=%d : "fmt, __FUNCTION__, __LINE__, ##args)
#define TOUCH_WARN(fmt, args...)		printk(KERN_WARNING LGTP_TAG"[W]"LGTP_MODULE" %s() line=%d : "fmt, __FUNCTION__, __LINE__, ##args)
#define TOUCH_LOG(fmt, args...)		printk(KERN_INFO LGTP_TAG"[L]"LGTP_MODULE" " fmt, ##args)
#define TOUCH_DBG(fmt, args...)		printk(KERN_DEBUG LGTP_TAG"[D]"LGTP_MODULE" " fmt, ##args)
#define TOUCH_FUNC(f)  	   			printk(KERN_INFO LGTP_TAG"[F]"LGTP_MODULE" %s()\n", __FUNCTION__)
#define TOUCH_FUNC_EXT(fmt, args...) 	printk(KERN_INFO LGTP_TAG"[F]"LGTP_MODULE" %s() : "fmt, __FUNCTION__, ##args)
#endif

/****************************************************************************
* Global Function Prototypes
****************************************************************************/

#endif /* _LGTP_COMMON_H_ */

/* End Of File */
