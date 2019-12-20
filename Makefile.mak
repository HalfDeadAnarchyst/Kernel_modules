obj-m += krishna.o
all:
	make -C /lib/modules/4.4.0-142-generic/build M=/opt/module modules
clean:
	make -C /lib/modules/4.4.0-142-generic/build M=/opt/module clean