ifeq ($(CONFIG_OF),y)

ifeq ($(CONFIG_LGE_HW_REV_DTS),y)

ifeq ($(CONFIG_MACH_MSM8909_M1_SPR_US),y)
DTS_TARGET := msm8909-m1_spr_us
endif

ifeq ($(CONFIG_MACH_MSM8909_M1_TMO_US),y)
DTS_TARGET := msm8909-m1_tmo_us
endif

ifeq ($(CONFIG_MACH_MSM8909_M1_MPCS_US),y)
DTS_TARGET := msm8909-m1_mpcs_us
endif

ifeq ($(CONFIG_MACH_MSM8909_M1_TRF_US),y)
DTS_TARGET := msm8909-m1_trf_us
endif

ifeq ($(CONFIG_MACH_MSM8909_M1_TRF_US_VZW),y)
DTS_TARGET := msm8909-m1_trf_us_vzw
endif

ifeq ($(CONFIG_MACH_MSM8909_M1DS_GLOBAL_COM),y)
DTS_TARGET := msm8909-m1ds_global_com
endif
ALL_DTS := $(notdir $(wildcard echo $(srctree)/$(src)/$(DTS_TARGET)/*.dts))
dtb-y += $(patsubst %.dts,%.dtb,$(ALL_DTS))

else
dtb-$(CONFIG_MACH_MSM8909_M1_SPR_US)        += msm8909-m1_spr_us.dtb
dtb-$(CONFIG_MACH_MSM8909_M1_TMO_US)        += msm8909-m1_tmo_us.dtb
dtb-$(CONFIG_MACH_MSM8909_M1_MPCS_US)        += msm8909-m1_mpcs_us.dtb
dtb-$(CONFIG_MACH_MSM8909_M1_TRF_US)        += msm8909-m1_trf_us.dtb
dtb-$(CONFIG_MACH_MSM8909_M1_TRF_US_VZW)        += msm8909-m1_trf_us_vzw.dtb
dtb-$(CONFIG_MACH_MSM8909_M1DS_GLOBAL_COM)        += msm8909-m1ds_global_com.dtb
endif

DTB_NAMES := $(subst $\",,$(CONFIG_BUILD_ARM_APPENDED_DTB_IMAGE_NAMES))
ifneq ($(DTB_NAMES),)
DTB_LIST := $(addsuffix .dtb,$(DTB_NAMES))
else
DTB_LIST := $(dtb-y)
endif

targets += dtbs
ifeq ($(CONFIG_LGE_HW_REV_DTS),y)
targets += $(addprefix ../, $(DTB_LIST))
endif
endif

ifneq ($(CONFIG_LGE_HW_REV_DTS),y)
DTS_TARGET := $(patsubst "%",%,$(call CONFIG_DTS_TARGET,$(1)))
endif

$(obj)/../%.dtb: $(src)/$(DTS_TARGET)/%.dts
	$(call if_changed_dep,dtc)

dtbs: $(addprefix $(obj)/../,$(DTB_LIST))

clean-files := *.dtb
