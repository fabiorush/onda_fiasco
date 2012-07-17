
GCC_PATH_PREFIX = /home/flus/arm-2010.09/bin/arm-none-linux-gnueabi-
PATH := $(PWD)/l4re-snapshot-2011081207/bin:$(PATH)

export GCC_PATH_PREFIX PATH

all: l4re-snapshot-2011081207/obj/l4/arm-beagleboard/images/bootstrap_L4Linux_ARM.uimage

l4re-snapshot-2011081207/obj/l4/arm-beagleboard/images/bootstrap_L4Linux_ARM.uimage: l4re-snapshot-2011081207/obj/l4/arm-beagleboard/ext-pkg/home/flus/Mestrado/onda_fiasco/pkg/onda/OBJ-arm_armv6-l4f/onda l4re-snapshot-2011081207/.built conf/arm-omap3.devs conf/l4onda.cfg conf/rootfs.squashfs
	make -C l4re-snapshot-2011081207/obj/l4/arm-beagleboard uimage E="L4Linux ARM"

l4re-snapshot-2011081207/obj/l4/arm-beagleboard/ext-pkg/home/flus/Mestrado/onda_fiasco/pkg/onda/OBJ-arm_armv6-l4f/onda: l4re-snapshot-2011081207/.built pkg/onda/main.cc
	make -C pkg/onda/ O=$(PWD)/l4re-snapshot-2011081207/obj/l4/arm-beagleboard/

l4re-snapshot-2011081207/.built: l4re-snapshot-2011081207/.extract pkg/client/app_comm.c
	make -C l4re-snapshot-2011081207/
	touch $@

l4re-snapshot-2011081207/.extract:
	tar xjf dl/l4re-snapshot-2011081207.tar.bz2
	ln -sf $(PWD)/conf/modules.list l4re-snapshot-2011081207/src/l4/conf/modules.list
	ln -s $(PWD)/conf/{arm-omap3.devs,l4onda.cfg,rootfs.squashfs} l4re-snapshot-2011081207/files/cfg/
	ln -s $(PWD)/pkg/client/app_comm.c l4re-snapshot-2011081207/src/l4linux/drivers/char/onda.c
	echo "obj-y     += onda.o" >> l4re-snapshot-2011081207/src/l4linux/drivers/char/Makefile
	make -C l4re-snapshot-2011081207/ setup
	ln -sf $(PWD)/conf/arm-up_config ./l4re-snapshot-2011081207/obj/l4linux/arm-up/.config
	ln -sf $(PWD)/conf/arm-mp_config ./l4re-snapshot-2011081207/obj/l4linux/arm-mp/.config
	touch $@

