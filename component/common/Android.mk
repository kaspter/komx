LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	JetOMX_Basecomponent.c \
	JetOMX_Baseport.c

LOCAL_MODULE := libJetOMX_Basecomponent

LOCAL_CFLAGS :=

LOCAL_STATIC_LIBRARIES := libJetOMX_OSAL

LOCAL_C_INCLUDES := $(EXYNOS_OMX_INC)/khronos \
	$(EXYNOS_OMX_INC)/exynos \
	$(EXYNOS_OMX_TOP)/osal

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	JetOMX_Resourcemanager.c

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libJetOMX_Resourcemanager

LOCAL_CFLAGS :=

LOCAL_STATIC_LIBRARIES := libJetOMX_OSAL
LOCAL_SHARED_LIBRARIES := libcutils libutils

LOCAL_C_INCLUDES := $(EXYNOS_OMX_INC)/khronos \
	$(EXYNOS_OMX_INC)/exynos \
	$(EXYNOS_OMX_TOP)/osal

include $(BUILD_SHARED_LIBRARY)
