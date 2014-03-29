


LOCAL_MODULE_CLASS ?= SHARED_LIBRARIES


LOCAL_MODULE := libJetOMX_Basecomponent



LOCAL_SRC_FILES := \
	common/JetOMX_Baseport.c \
	common/JetOMX_Basecomponent.c \



COBJS	:= $(LOCAL_SRC_FILES:%.c=%.o)
OBJS	:= $(addprefix $(obj),$(COBJS))



LOCAL_C_INCLUDES += -I. -I../include/khronos -I../include/jetomx -I../osal -I../component/common


LOCAL_CFLAGS := -Wall -g

LOCAL_CFLAGS += -DNDEBUG -DANDROID -D_REENTRANT -D_FILE_OFFSET_BITS=64  -D_GNU_SOURCE


#LOCAL_STATIC_LIBRARIES := libpthread


LDFLAGS    = -shared -lpthread -o
RANLIB     = ranlib



ifeq ($(strip $(LOCAL_MODULE_CLASS)),SHARED_LIBRARIES)
LIB	= $(LOCAL_MODULE).so
LOCAL_CFLAGS    += -fPIC
endif

ifeq ($(strip $(LOCAL_MODULE_CLASS)),STATIC_LIBRARIES)
LIB	= $(LOCAL_MODULE).a
endif

LOCAL_ALL_FLAGS = $(LOCAL_CFLAGS) $(LOCAL_C_INCLUDES)


BIN = dumy_test

all: $(OBJS) $(LIB)


$(obj)%.o:	%.c
	$(CC)  $(LOCAL_ALL_FLAGS) -o $@ $< -c


$(LIB): $(OBJS)
	$(CXX) -shared -Wall -o $@ $^ -lrt -ldl -lpthread
	mv $(LIB) ../release/$(LIB)

$(BIN): dumy.o
	$(CXX) $(CXXFLAGS) $(EXT_FLAGS) -o $@ $^ -L../release -lJetOMX_OSAL
	mv $(BIN) ../release/$(BIN)

clean:
	@rm -f $(OBJS) $(LIB) $(BIN)
	@rm -f *.o
