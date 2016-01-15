#include<linux/init.h>
#include<linux/module.h>
#include<linux/timekeeping.h>
#include<linux/i2c.h>

#include"l3gd20h.h"

#define SUBADDRESS(addr) ((addr) | (1<<7))
#define DATA_AVAILABLE(status) ((status) | (1<<3))

#define I2C_WRITE(client, reg, val)\
	i2c_smbus_write_byte_data((client), (reg), (val))

#define I2C_READ(client, reg)\
	i2c_smbus_read_byte_data((client), (reg))

#define I2C_READ_BLOCK(client, reg, size, out)\
		i2c_smbus_read_i2c_block_data((client), (reg), (size), (out))

static s32 set_reg(struct i2c_client *client, int reg)
{
	s32 err;
	struct l3gd20h_data tmp, *data = i2c_get_clientdata(client);

	mutex_lock(&data->lock);
	memcpy(&tmp, data, sizeof(tmp));
	mutex_unlock(&data->lock);

	switch (reg) {
	case CTRL1:
		err = I2C_WRITE(client, reg,
				0x0F |
				(tmp.gyro_rate << 6) |
				(tmp.gyro_bandwidth << 4));
		break;
	case CTRL4:
		err = I2C_WRITE(client, reg,
				(tmp.block_data_update << 7) |
				(tmp.gyro_sensitivity << 4));
		break;
	case CTRL5:
		err = I2C_WRITE(client, reg,
				(tmp.fifo_enable << 6));
		break;
	case FIFO_CTRL:
		err = I2C_WRITE(client, reg,
				tmp.fifo_threshold |
				(tmp.fifo_mode << 5));
		break;
	case LOW_ODR:
		err = I2C_WRITE(client, reg,
				tmp.gyro_low_odr);
	default:
		return 0;
	}

	return err;
}

static ssize_t show_read(struct device *dev, struct device_attribute *dev_attr, char *buf)
{
	u8 tmp[6];
	s32 status_g, err;
	struct i2c_client *client = to_i2c_client(dev);
	struct l3gd20h_data *data = i2c_get_clientdata(client);
	struct point3 gyro;

	if ((err = status_g = I2C_READ(client, STATUS)) < 0)
		return err;

	if (DATA_AVAILABLE(status_g)) {
		if ((err = I2C_READ_BLOCK(client,
				SUBADDRESS(OUT_X_L), (u8) 6, tmp)) < 0)
			return err;

		gyro.x = (tmp[1] << 8) | tmp[0];
		gyro.y = (tmp[3] << 8) | tmp[2];
		gyro.z = (tmp[5] << 8) | tmp[4];

		mutex_lock(&data->lock);
		memcpy(&data->gyro, &gyro, sizeof(data->gyro));
		mutex_unlock(&data->lock);
	}

	return scnprintf(buf, PAGE_SIZE, "%d %d %d\n",
			(s16) gyro.x, (s16) gyro.y, (s16) gyro.z);
}
static DEVICE_ATTR(read, S_IRUGO, show_read, NULL);

#define L3GD20H_ATTR(name, lbound, ubound, reg)\
static ssize_t store_ ## name(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)\
{\
	s32 err;\
	unsigned val;\
	struct i2c_client *client = to_i2c_client(dev);\
	struct l3gd20h_data *data = i2c_get_clientdata(client);\
\
	if (sscanf(buf, "%u", &val) != 1)\
		return -EINVAL;\
\
	if (val < lbound || val > ubound)\
		return -EINVAL;\
\
	if ((err = set_reg(client, reg)))\
		return -EIO;\
\
	mutex_lock(&data->lock);\
	data->name = (u8) val;\
	mutex_unlock(&data->lock);\
\
	return count;\
}\
\
static ssize_t show_ ## name(struct device *dev, struct device_attribute *dev_attr, char *buf)\
{\
	ssize_t count;\
	struct l3gd20h_data *data = i2c_get_clientdata(to_i2c_client(dev));\
\
	mutex_lock(&data->lock);\
	count = scnprintf(buf, PAGE_SIZE, "%u\n", data->name);\
	mutex_unlock(&data->lock);\
\
	return count;\
}\
static DEVICE_ATTR(name, S_IRUGO, show_ ## name, store_ ## name)

L3GD20H_ATTR(block_data_update, BDU_OFF, BDU_ON, CTRL4);
L3GD20H_ATTR(gyro_rate, GODR_MIN, GODR_MIN, CTRL1);
L3GD20H_ATTR(gyro_low_odr, LOW_ODR_OFF, LOW_ODR_ON, LOW_ODR);
L3GD20H_ATTR(gyro_sensitivity, GFS_245, GFS_2000, CTRL4);
L3GD20H_ATTR(gyro_bandwidth, GBW_MIN, GBW_MAX, CTRL1);

static struct attribute *l3gd20h_attributes[] = {
	&dev_attr_read.attr,
	&dev_attr_block_data_update.attr,
	&dev_attr_gyro_rate.attr,
	&dev_attr_gyro_low_odr.attr,
	&dev_attr_gyro_sensitivity.attr,
	&dev_attr_gyro_bandwidth.attr,
	NULL
};

static const struct attribute_group l3gd20h_attr_group = {
	.attrs = l3gd20h_attributes,
};

static int l3gd20h_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err;
	struct device *dev = &client->dev;

	struct l3gd20h_data *data = devm_kzalloc(dev, sizeof(struct l3gd20h_data), GFP_KERNEL);

	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);

	mutex_init(&data->lock);

	data->block_data_update = BDU_ON;

	data->fifo_enable = FIFO_DISABLED;
	data->fifo_threshold_enable = FIFO_THRESHOLD_DISABLED;
	data->fifo_mode = BYPASS;
	data->fifo_threshold = FIFO_THRESHOLD_MIN;

	data->gyro_rate = GODR_MIN;
	data->gyro_low_odr = LOW_ODR_OFF;
	data->gyro_sensitivity = GFS_245;
	data->gyro_bandwidth = GBW_MIN;

	if ((err = set_reg(client, LOW_ODR)) ||
			(err = set_reg(client, CTRL4)) ||
			(err = set_reg(client, CTRL1)))
		goto err_return;

	err = sysfs_create_group(&dev->kobj, &l3gd20h_attr_group);
	if (err)
		goto err_remove_attr;

	printk(KERN_INFO "l3gd20h_probe\n");
	return 0;

err_remove_attr:
	sysfs_remove_group(&dev->kobj, &l3gd20h_attr_group);
err_return:
	return err;

}

static int l3gd20h_remove(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &l3gd20h_attr_group);
	printk(KERN_INFO "l3gd20h_remove\n");
	return 0;
}

static const struct i2c_device_id l3gd20h_id[] = {
	{ "l3gd20h", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, l3gd20h_id);

#if defined(CONFIG_OF)
static const struct of_device_id l3gd20h_of_match[] = {
	{ .compatible = "st,l3gd20h" },
	{ },
};
MODULE_DEVICE_TABLE(of, l3gd20h_of_match);
#endif

static struct i2c_driver l3gd20h_driver = {
	.driver = {
		.name = "l3gd20h",
		.of_match_table = of_match_ptr(l3gd20h_of_match),
	},
	.probe = l3gd20h_probe,
	.remove = l3gd20h_remove,
	.id_table = l3gd20h_id,
};
module_i2c_driver(l3gd20h_driver);

MODULE_LICENSE("Dual MIT/GPL");
