LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE	:= yz_comm
LOCAL_CXXFLAGS := -DHAVE_PTHREADS
LOCAL_SRC_FILES := YzJni.cpp	\
					message/message.cpp \
					log4z.cpp
									
include $(BUILD_SHARED_LIBRARY)
