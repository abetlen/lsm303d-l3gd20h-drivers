#include<linux/init.h>
#include<linux/module.h>
#include<linux/i2c.h>

#include"lsm303d.h"

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
	struct lsm303d_data tmp, *data = i2c_get_clientdata(client);

	mutex_lock(&data->lock);
	memcpy(&tmp, data, sizeof(tmp));
	mutex_unlock(&data->lock);

	switch (reg) {
	case CTRL0:
		err = I2C_WRITE(client, reg,
				(tmp.fifo_enable << 6) |
				(tmp.fifo_threshold_enable << 5));
		break;
	case CTRL1:
		err = I2C_WRITE(client, reg,
				0x07 |
				(tmp.acc_rate << 4) |
				(tmp.block_data_update << 3));
		break;
	case CTRL2:
		err = I2C_WRITE(client, reg,
				(tmp.acc_bandwidth << 6) |
				(tmp.acc_sensitivity << 3));
		break;
	case CTRL5:
		err = I2C_WRITE(client, reg,
				(tmp.mag_resolution << 5) |
				(tmp.mag_rate << 2));
		break;
	case CTRL6:
		err = I2C_WRITE(client, reg,
				(tmp.mag_sensitivity << 5));
		break;
	case CTRL7:
		err = I2C_WRITE(client, reg, 0x00);
		break;
	case FIFO_CTRL:
		err = I2C_WRITE(client, reg,
				tmp.fifo_threshold |
				(tmp.fifo_mode << 5));
		break;
	default:
		return 0;
	}

	return err;
}

static ssize_t show_read(struct device *dev, struct device_attribute *dev_attr, char *buf)
{
	u8 tmp[6];
	s32 status_m, status_a, err;
	struct i2c_client *client = to_i2c_client(dev);
	struct lsm303d_data *data = i2c_get_clientdata(client);
	struct point3 acc, mag;

	if ((err = status_m = I2C_READ(client, STATUS_M)) < 0 ||
			(err = status_a = I2C_READ(client, STATUS_A)) < 0)
		return err;

	if (DATA_AVAILABLE(status_m)) {
		if ((err = I2C_READ_BLOCK(client,
				SUBADDRESS(OUT_X_L_M), (u8) 6, tmp)) < 0)
			return err;

		mag.x = (tmp[1] << 8) | tmp[0];
		mag.y = (tmp[3] << 8) | tmp[2];
		mag.z = (tmp[5] << 8) | tmp[4];

		mutex_lock(&data->lock);
		memcpy(&data->mag, &mag, sizeof(data->mag));
		mutex_unlock(&data->lock);
	}

	if (DATA_AVAILABLE(status_a)) {
		if ((err = I2C_READ_BLOCK(client,
				SUBADDRESS(OUT_X_L_A), (u8) 6, tmp)) < 0)
			return err;

		acc.x = (tmp[1] << 8) | tmp[0];
		acc.y = (tmp[3] << 8) | tmp[2];
		acc.z = (tmp[5] << 8) | tmp[4];

		mutex_lock(&data->lock);
		memcpy(&data->acc, &acc, sizeof(data->acc));
		mutex_unlock(&data->lock);
	}

	return scnprintf(buf, PAGE_SIZE, "%d %d %d %d %d %d\n",
			(s16) mag.x, (s16) mag.y, (s16) mag.z,
			(s16) acc.x, (s16) acc.y, (s16) acc.z);
}
static DEVICE_ATTR(read, S_IRUGO, show_read, NULL);

static ssize_t store_mag_resolution(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)
{
	s32 err;
	unsigned val;
	struct i2c_client *client = to_i2c_client(dev);
	struct lsm303d_data *data = i2c_get_clientdata(client);

	if (sscanf(buf, "%u", &val) != 1)
		return -EINVAL;

	if (val != MRES_LOW && val != MRES_HIGH)
		return -EINVAL;

	if ((err = set_reg(client, CTRL5)))
		return -EIO;

	mutex_lock(&data->lock);
	data->mag_resolution = (u8) val;
	mutex_unlock(&data->lock);

	return count;
}

static ssize_t show_mag_resolution(struct device *dev, struct device_attribute *dev_attr, char *buf)
{
	ssize_t count;
	struct lsm303d_data *data = i2c_get_clientdata(to_i2c_client(dev));

	mutex_lock(&data->lock);
	count = scnprintf(buf, PAGE_SIZE, "%un", data->mag_resolution);
	mutex_unlock(&data->lock);

	return count;
}
static DEVICE_ATTR(mag_resolution, S_IRUGO, show_mag_resolution, store_mag_resolution);


#define LSM303D_ATTR(name, lbound, ubound, reg)\
static ssize_t store_ ## name(struct device *dev, struct device_attribute *dev_attr, const char *buf, size_t count)\
{\
	s32 err;\
	unsigned val;\
	struct i2c_client *client = to_i2c_client(dev);\
	struct lsm303d_data *data = i2c_get_clientdata(client);\
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
	struct lsm303d_data *data = i2c_get_clientdata(to_i2c_client(dev));\
\
	mutex_lock(&data->lock);\
	count = scnprintf(buf, PAGE_SIZE, "%u\n", data->name);\
	mutex_unlock(&data->lock);\
\
	return count;\
}\
static DEVICE_ATTR(name, S_IRUGO, show_ ## name, store_ ## name)

LSM303D_ATTR(block_data_update, BDU_OFF, BDU_ON, CTRL1);
LSM303D_ATTR(acc_rate, AODR_OFF, AODR_1600HZ, CTRL1);
LSM303D_ATTR(acc_sensitivity, AFS_2G, AFS_16G, CTRL2);
LSM303D_ATTR(acc_bandwidth, ABW_50HZ, ABW_773HZ, CTRL2);
LSM303D_ATTR(mag_rate, MODR_3_125HZ, MODR_100HZ, CTRL5);
LSM303D_ATTR(mag_sensitivity, MFS_2G, MFS_12G, CTRL6);

static struct attribute *lsm303d_attributes[] = {
	&dev_attr_read.attr,
	&dev_attr_block_data_update.attr,
	&dev_attr_acc_rate.attr,
	&dev_attr_acc_sensitivity.attr,
	&dev_attr_acc_bandwidth.attr,
	&dev_attr_mag_rate.attr,
	&dev_attr_mag_sensitivity.attr,
	&dev_attr_mag_resolution.attr,
	NULL
};

static const struct attribute_group lsm303d_attr_group = {
	.attrs = lsm303d_attributes,
};

static int lsm303d_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err;
	struct device *dev = &client->dev;

	struct lsm303d_data *data;

	data = devm_kzalloc(dev, sizeof(struct lsm303d_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);

	mutex_init(&data->lock);

	data->block_data_update = BDU_ON;
	data->fifo_enable = FIFO_DISABLED;
	data->fifo_threshold_enable = FIFO_THRESHOLD_DISABLED;
	data->fifo_mode = BYPASS;
	data->fifo_threshold = FIFO_THRESHOLD_MIN;

	data->acc_rate = AODR_3_125HZ;
	data->acc_sensitivity = AFS_2G;
	data->acc_bandwidth = ABW_50HZ;

	data->mag_rate = MODR_3_125HZ;
	data->mag_sensitivity = MFS_4G;
	data->mag_resolution = MRES_HIGH;

	if ((err = set_reg(client, CTRL2)) ||
			(err = set_reg(client, CTRL1)) ||
			(err = set_reg(client, CTRL5)) ||
			(err = set_reg(client, CTRL6)) ||
			(err = set_reg(client, CTRL7)))
		goto err_return;

	err = sysfs_create_group(&dev->kobj, &lsm303d_attr_group);
	if (err)
		goto err_remove_attr;

	printk(KERN_INFO "lsm303d_probe\n");
	return 0;

err_remove_attr:
	sysfs_remove_group(&dev->kobj, &lsm303d_attr_group);
err_return:
	return err;
}

static int lsm303d_remove(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &lsm303d_attr_group);
	printk(KERN_INFO "lsm303d_remove\n");
	return 0;
}

static const struct i2c_device_id lsm303d_id[] = {
	{ "lsm303d", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, lsm303d_id);

#if defined(CONFIG_OF)
static const struct of_device_id lsm303d_of_match[] = {
	{ .compatible = "st,lsm303d" },
	{ },
};
MODULE_DEVICE_TABLE(of, lsm303d_of_match);
#endif

static struct i2c_driver lsm303d_driver = {
	.driver = {
		.name = "lsm303d",
		.of_match_table = of_match_ptr(lsm303d_of_match),
	},
	.probe = lsm303d_probe,
	.remove = lsm303d_remove,
	.id_table = lsm303d_id,
};
module_i2c_driver(lsm303d_driver);

MODULE_LICENSE("Dual MIT/GPL");
