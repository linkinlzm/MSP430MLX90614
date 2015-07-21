/*
 * HAL_MLX90614.h
 *
 *  Created on: 2015-7-19
 *      Author: hp
 */

#ifndef HAL_MLX90614_H_
#define HAL_MLX90614_H_

#define MLX90615_I2C_ADDRESS 0x5A   //slave address
extern float g_objectTemp;						// Temperature of object in field of view
extern float g_ambientTemp;
extern float temps[100];
extern int counter;
extern void I2C(void);    //I2C init function
extern void ReadTemp(void);   //read object temperature
extern void ReadAmbi(void);   //read sensor temperature

#endif /* HAL_MLX90614_H_ */
