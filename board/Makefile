DTC=/lib/modules/$(shell uname -r)/build/scripts/dtc/dtc
TARGET=altimu10_i2c

all:
	$(DTC) -@ -I dts -O dtb -o $(TARGET).dtb $(TARGET).dts

install:
	cp $(TARGET).dtb /boot/overlays

clean:
	- rm *.dtb
