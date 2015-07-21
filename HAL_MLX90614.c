/*
 * HAL_MLX90614.c
 *
 *  Created on: 2015-7-19
 *      Author: hp
 */
#include "HAL_MLX90614.h"
#include <stdint.h>
#include "msp430.h"
uint8_t g_mlxValBytes[3];		// Recieved value byte storage
float g_objectTemp;						// Temperature of object in field of view
float g_ambientTemp;
float temps[100] = {0};
int counter = 0;

void I2C(void)
{
	P3SEL |= BIT0 | BIT1; //I2C Pins P3.0; P3.1
	UCB0CTL1 |= UCSWRST;		// Software reset enabled
	UCB0CTL0 |= UCMODE_3 | UCMST | UCSYNC; // I2C mode, master, sync
	UCB0BRW = 60;				// Baudrate = SMCLK / 20;
	UCB0CTL1 |= UCSSEL_2 | UCTR;		// SMCLK, TX
	UCB0I2CSA = MLX90615_I2C_ADDRESS;	// I2C Address
	UCB0CTL1 &= ~UCSWRST;		// Clear reset
	UCB0IE &= ~UCRXIE;			// Ensure Interrupts off
	UCB0IE &= ~UCTXIE;
}
void ReadTemp(void)
{
	// Send object temperature read command
	UCB0CTL1 |= UCTXSTT;			// Send start
	while(!(UCB0IFG & UCTXIFG));	// Wait for TX interrupt flag
	UCB0TXBUF = 0x07;		// Send object temperature command
	while(!(UCB0IFG & UCTXIFG));	// Wait for TX interrupt flag
	UCB0CTL1 &= ~UCTR;				// Change to receive
	UCB0CTL1 |= UCTXSTT;			// Send restart
	while(UCB0CTL1 & UCTXSTT);		// Wait for restart
	// Receive Bytes
	while(!(UCB0IFG & UCRXIFG));	// Wait for RX interrupt flag
	g_mlxValBytes[0] = UCB0RXBUF;	// 0th byte
	while(!(UCB0IFG & UCRXIFG));	// Wait for RX interrupt flag
	g_mlxValBytes[1] = UCB0RXBUF;	// 1st byte
	while(!(UCB0IFG & UCRXIFG));	// Wait for RX interrupt flag
	g_mlxValBytes[2] = UCB0RXBUF;	// 2nd byte
	UCB0CTL1 |= UCTXSTP;			// Send stop
	while(UCB0CTL1 & UCTXSTP);		// Wait for stop
	g_mlxValBytes[2] = UCB0RXBUF;	// 2nd byte
	// Calculate temperature
	g_objectTemp = 0.0;
	uint16_t tempVals = ( ((uint16_t) g_mlxValBytes[1]) << 8 ) | ( (uint16_t) g_mlxValBytes[0] );
	g_objectTemp = ((float) tempVals) * 0.02 - 273.15;
	temps[counter] = g_objectTemp;
	counter = (counter+1)%100;
}
void ReadAmbi(void)
{
	// Send Ambient temperature read command
	UCB0CTL1 |= UCTXSTT;			// Send start
	while(!(UCB0IFG & UCTXIFG));	// Wait for TX interrupt flag
	UCB0TXBUF = 0x06;		// Send object temperature command
	while(!(UCB0IFG & UCTXIFG));	// Wait for TX interrupt flag
	UCB0CTL1 &= ~UCTR;				// Change to receive
	UCB0CTL1 |= UCTXSTT;			// Send restart
	while(UCB0CTL1 & UCTXSTT);		// Wait for restart
	// Receive Bytes
	while(!(UCB0IFG & UCRXIFG));	// Wait for RX interrupt flag
	g_mlxValBytes[0] = UCB0RXBUF;	// 0th byte
	while(!(UCB0IFG & UCRXIFG));	// Wait for RX interrupt flag
	g_mlxValBytes[1] = UCB0RXBUF;	// 1st byte
	while(!(UCB0IFG & UCRXIFG));	// Wait for RX interrupt flag
	g_mlxValBytes[2] = UCB0RXBUF;	// 2nd byte
	UCB0CTL1 |= UCTXSTP;			// Send stop
	while(UCB0CTL1 & UCTXSTP);		// Wait for stop
	g_mlxValBytes[2] = UCB0RXBUF;	// 2nd byte
	g_ambientTemp = 0.0;
	uint16_t tempVals = ( ((uint16_t) g_mlxValBytes[1]) << 8 ) | ( (uint16_t) g_mlxValBytes[0] );
	g_ambientTemp = ((float) tempVals) * 0.02 - 273.15;
}

