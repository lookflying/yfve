LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE	:= test_yfve
LOCAL_CXXFLAGS := -DHAVE_PTHREADS
LOCAL_SRC_FILES := YfveDemo.cpp \
									 log4z.cpp \
									 Connection.cpp
include $(BUILD_EXECUTABLE)
