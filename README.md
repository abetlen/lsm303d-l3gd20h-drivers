LSM303D / L3GD20H I2C Drivers
=============================

[![Travis branch](https://img.shields.io/travis/abetlen/lsm303d-l3gd20h-drivers/master.svg?maxAge=2592000)]()

Character device drivers for the LSM303D and L3GD20H over an I2C interface.

Usage
-----

The drivers use a sysfs interface. To read from the lsm303d and
l3gd20h respectively:

```
cat /sys/bus/i2c/devices/1-001d/read
cat /sys/bus/i2c/devices/1-006b/read
```

To convert these values to their respective quantities in SI units:

Accelerometer (g) | mg/LSB | Magnetometer (gauss) | mgauss/LSB | Gyroscope (dps) | mdps/LSB
------------------|--------|----------------------|------------|-----------------|---------
2                 | 0.061  | 2                    | 0.080      | 245             | 8.75
4                 | 0.122  | 4                    | 0.160      | 500             | 17.50
6                 | 0.183  | 8                    | 0.320      | 2000            | 70.00
8                 | 0.244  | 12                   | 0.479      |                 |
16                | 0.732  |                      |            |                 |

Installation
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
Resources
---------

- [L3GD20H
  Datasheet](https://www.pololu.com/file/download/L3GD20H.pdf?file_id=0J731)
- [LSM303D
  Datasheet](https://www.pololu.com/file/download/LSM303D.pdf?file_id=0J703)
- [Raspberry Pi Device
  Tree](https://www.raspberrypi.org/documentation/configuration/device-tree.md)
