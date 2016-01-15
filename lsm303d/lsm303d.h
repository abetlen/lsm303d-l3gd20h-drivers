#ifndef LSM303D_H
#define LSM303D_H

/* LSM303D Register Map */
#define TEMP_OUT_L 0x05
#define STATUS_M 0x07
#define OUT_X_L_M 0x08
#define OUT_Y_L_M 0x0A
#define OUT_Z_L_M 0x0C
#define WHO_AM_I 0X0F
#define OFFSET_X_L_M 0x16
#define OFFSET_Y_L_M 0x18
#define OFFSET_Z_L_M 0x1A
#define CTRL0 0x1F
#define CTRL1 0x20
#define CTRL2 0x21
#define CTRL3 0x22
#define CTRL4 0x23
#define CTRL5 0x24
#define CTRL6 0x25
#define CTRL7 0x26
#define STATUS_A 0x27
#define OUT_X_L_A 0x28
#define OUT_Y_L_A 0x2A
#define OUT_Z_L_A 0x2C
#define FIFO_CTRL 0x2E
#define FIFO_SRC 0x2F

enum {
	/* Block Data Update */
	BDU_OFF,
	BDU_ON
};

enum {
	/* FIFO Enable */
	FIFO_ENABLED,
	FIFO_DISABLED
};

enum {
	/* FIFO Threshold Enable */
	FIFO_THRESHOLD_ENABLED,
	FIFO_THRESHOLD_DISABLED
};

enum {
	/* FIFO Mode */
	BYPASS,
	FIFO,
	STREAM,
	STREAM_TO_FIFO,
	BYPASS_TO_STREAM
};

enum {
	/* FIFO Threshold Range */
	FIFO_THRESHOLD_MIN,
	FIFO_THRESHOLD_MAX=0x1F
};

/*
 * Accelerometer
 */

enum {
	/* Accelerometer Data Rate */
	AODR_OFF,
	AODR_3_125HZ,
	AODR_6_25HZ,
	AODR_12_5HZ,
	AODR_25HZ,
	AODR_50HZ,
	AODR_100HZ,
	AODR_200HZ,
	AODR_400HZ,
	AODR_800HZ,
	AODR_1600HZ,
};

enum {
	/* Accelerometer Full-Scale Range */
	AFS_2G, /* 0.061 mg/LSB */
	AFS_4G,  /* 0.122 mg/LSB */
	AFS_6G,  /* 0.183 mg/LSB */
	AFS_8G,  /* 0.244 mg/LSB */
	AFS_16G  /* 0.732 mg/LSB */
};

enum {
	/* Accelerometer Filter Bandwidth */
	ABW_50HZ,
	ABW_194HZ,
	ABW_362HZ,
	ABW_773HZ
};

/*
 * Magnetometer
 */

enum {
	/* Magnetometer Data Rate */
	MODR_3_125HZ,
	MODR_6_25HZ,
	MODR_12_5HZ,
	MODR_25HZ,
	MODR_50HZ,
	MODR_100HZ
};

enum {
	/* Magnetometer Full-Scale Range */
	MFS_2G,  /* 0.080 mgauss/LSB */
	MFS_4G,  /* 0.160 mgauss/LSB */
	MFS_8G,  /* 0.320 mgauss/LSB */
	MFS_12G,  /* 0.479 mgauss/LSB */
};

enum {
	/* Magnetometer Resolution */
	MRES_LOW=0,
	MRES_HIGH=0x03
};


struct lsm303d_data {
	struct mutex lock;

	struct point3 {
		s16 x, y, z;
	} mag, mag_offset, acc, acc_offset;

	u8 block_data_update;

	u8 fifo_enable;
	u8 fifo_threshold_enable;
	u8 fifo_mode;
	u8 fifo_threshold;

	u8 acc_rate;
	u8 acc_sensitivity;
	u8 acc_bandwidth;

	u8 mag_rate;
	u8 mag_sensitivity;
	u8 mag_resolution;
};

#endif
