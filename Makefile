
GCC_PATH_PREFIX = /home/flus/arm-2010.09/bin/arm-none-linux-gnueabi-
PATH := $(PWD)/l4re-snapshot-2011081207/bin:$(PATH)

export GCC_PATH_PREFIX PATH

all: l4re-snapshot-2011081207/obj/l4/arm-beagleboard/images/bootstrap_L4Linux_ARM.uimage

l4re-snapshot-2011081207/obj/l4/arm-beagleboard/images/bootstrap_L4Linux_ARM.uimage: l4re-snapshot-2011081207/obj/l4/arm-beagleboard/ext-pkg/home/flus/Mestrado/onda_fiasco/pkg/onda/OBJ-arm_armv6-l4f/onda
	make -C l4re-snapshot-2011081207/obj/l4/arm-beagleboard uimage E="L4Linux ARM"

l4re-snapshot-2011081207/obj/l4/arm-beagleboard/ext-pkg/home/flus/Mestrado/onda_fiasco/pkg/onda/OBJ-arm_armv6-l4f/onda: l4re-snapshot-2011081207/.built pkg/onda/main.c
	make -C pkg/onda/ O=$(PWD)/l4re-snapshot-2011081207/obj/l4/arm-beagleboard/

l4re-snapshot-2011081207/.built: l4re-snapshot-2011081207/.extract
	make -j4 -C l4re-snapshot-2011081207/
	touch $@

l4re-snapshot-2011081207/.extract:
	tar xjf dl/l4re-snapshot-2011081207.tar.bz2
	ln -sf $(PWD)/conf/modules.list l4re-snapshot-2011081207/src/l4/conf/modules.list
	ln -s $(PWD)/conf/{arm-omap3.devs,l4onda.cfg} l4re-snapshot-2011081207/files/cfg/
	make -j4 -C l4re-snapshot-2011081207/ setup
	touch $@

teste:
	echo $(PATH)

#(cd l4re-snapshot-2011081207/; patch -p1 < ../l4.patch ) && \