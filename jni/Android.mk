LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE	:= yz_comm
LOCAL_CXXFLAGS := -DHAVE_PTHREADS
LOCAL_SRC_FILES := YzJni.cpp	\
					message/message.cpp \
					log4z.cpp \
					YzHelper.cpp \
					Connection.cpp
					
LOCAL_LDLIBS    := -llog
LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog									
include $(BUILD_SHARED_LIBRARY)
