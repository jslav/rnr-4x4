
PATH=$PATH:/usr/local/angstrom/armv7linaro/bin/
PRJ_LIB_ROOT ?= /home/ubobrov/develop/projects/intercom/build/pjsip_call/lib

CROSS_COMPILE = arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc
CP = /usr/bin/sudo /bin/cp
DEL = /bin/rm -f

MKFILE=Makefile
APP = supervisor
 
PJSIP_VER=2.9
#2.7.2

BIN_PATH = /home/ubobrov/develop/projects/intercom/rootfs/root

include $(PRJ_LIB_ROOT)/build_$(PJSIP_VER).mak

PJ_LIBS = $(PJ_LIB_FILES)
OTHER_LIBS= -lm -lrt -lpthread -ldl -lubox -luci

LIBS= $(PJ_LIBS) $(OTHER_LIBS)

LIBDIR= -L/home/ubobrov/develop/projects/intercom/rootfs/root/ZERO/old_cedrus/uci/2/libubox \
	    -L/home/ubobrov/develop/projects/intercom/rootfs/root/ZERO/old_cedrus/uci/2/uci

INCDIR= -I/home/ubobrov/develop/projects/intercom/rootfs/root/ZERO/old_cedrus/uci/2/libubox \
	    -I/home/ubobrov/develop/projects/intercom/rootfs/root/ZERO/old_cedrus/uci/2/uci

INC_DIR = $(patsubst %,-I%,$(INCDIR))

SRC = main.c uci_config.c video_device.c

OBJS = $(SRC:.c=.o)
DEPS = $(SRC:.c=.d)

LDFLAGS = -g -O2 -Wl,-EL $(LIBS) $(LIBDIR) 
#-Wl,-rpath=/home/ubobrov/develop/projects/intercom/build/pjsip_call/lib
CPFLAGS = $(INC_DIR) $(APP_CFLAGS)

all: $(OBJS) $(MKFILE)
	@echo "Linking $@"
	@$(CC) $(OBJS) -o $(APP) -Wl,-Map=app_le.map,--cref,--no-warn-mismatch $(LDFLAGS)
	-$(CP) $(APP) $(BIN_PATH)
	

%.o: %.c $(MKFILE)
	@echo "Compiling '$<'"
	$(CC) -c $(CPFLAGS) -I . $< -o $@

%.d: %.c $(MKFILE)
	@echo "Building dependencies for '$<'"
	@$(CC) -E -MM -MQ $(<:.c=.o) $(CPFLAGS) $< -o $@
	@$(DEL) $(<:.c=.o)
	
clean:
	-$(DEL) $(OBJS:/=\)
	-$(DEL) $(DEPS:/=\)
	-$(DEL) $(APP:/=\)

	
.PHONY: dep
dep: $(DEPS) $(SRC)
	@echo "##########################"
	@echo "### Dependencies built ###"
	@echo "##########################"

-include $(DEPS)
