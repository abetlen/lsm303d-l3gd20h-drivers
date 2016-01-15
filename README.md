LSM303D / L3GD20H I2C Drivers
====================

Raspberry Pi Installation
------------

Compile the drivers and install the device tree overlay to the
`/boot/overlays` folder:

```
  sudo make
  cd board
  sudo make install
```

Insert the drivers:

```
  sudo insmod lsm303d/lsm303d.ko
  sudo insmod l3gd20h/l3gd20h.ko
```
