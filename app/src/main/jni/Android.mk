# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
APP_ABI := all

LOCAL_MODULE    := native-lib
LOCAL_CPPFLAGS 	:= -O0 -D_UNICODE -DUNICODE -DUSE_DUMP -Wno-error=format-security
LOCAL_CPP_EXTENSION := .cpp
LOCAL_LDLIBS    := -lm -llog -lz
LOCAL_SHORT_COMMANDS := true
INC_DIRS = -I$(LOCAL_PATH)/jni
LOCAL_CPPFLAGS += $(INC_DIRS)

LOCAL_SRC_FILES	:= \
	jni_lib.cpp	\


LOCAL_SHARED_LIBRARIES += libandroid_runtime
	

include $(BUILD_SHARED_LIBRARY)

