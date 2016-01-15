#ifndef l3GD20H_H
#define l3GD20H_H

#define WHO_AM_I 0X0F
#define CTRL1 0x20
#define CTRL2 0x21
#define CTRL3 0x22
#define CTRL4 0x23
#define CTRL5 0x24
#define OUT_TEMP 0x26
#define STATUS 0x27
#define OUT_X_L 0x28
#define OUT_Y_L 0x2A
#define OUT_Z_L 0x2C
#define FIFO_CTRL 0x2E
#define FIFO_SRC 0x2F
#define LOW_ODR 0x39

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
	BYPASS_TO_STREAM,
	DYNAMIC_STREAM,
	BYPASS_TO_FIFO
};

enum {
	/* FIFO Threshold Range */
	FIFO_THRESHOLD_MIN,
	FIFO_THRESHOLD_MAX=0x1F
};

/*
 * Gyroscope
 */
enum {
	/* Gyroscope Data Rate Range */
	GODR_MIN,
	GODR_MAX=0x03
};

enum {
	/* Gyroscope Low Data Rate Mode */
	LOW_ODR_OFF,
	LOW_ODR_ON
};

enum {
	/* Gyroscope Full-Scale Range */
	GFS_245, /* 8.75 mdps/digit */
	GFS_500, /* 17.50 mdps/digit */
	GFS_2000 /* 70.00 mdps/digit */
};

enum {
	/* Gyroscope Bandwidth Range */
	GBW_MIN,
	GBW_MAX=0x03
};

struct l3gd20h_data {
	struct mutex lock;

	struct point3 {
		s16 x, y, z;
	} gyro, gyro_offset;

	u8 block_data_update;

	u8 fifo_enable;
	u8 fifo_threshold_enable;
	u8 fifo_mode;
	u8 fifo_threshold;

	u8 gyro_rate;
	u8 gyro_low_odr;
	u8 gyro_sensitivity;
	u8 gyro_bandwidth;

};

#endif
