LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE	:= yz_comm
LOCAL_CFLAGS := -DNDEBUG
LOCAL_CXXFLAGS := -DHAVE_PTHREADS
LOCAL_SRC_FILES := vehicle_CVS.cpp	\
					message/message.cpp \
					log4z.cpp \
					YzHelper.cpp \
					Connection.cpp \
					ConnectionManager.cpp
LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog									
include $(BUILD_SHARED_LIBRARY)
