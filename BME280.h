/*
 BME280.h
 Library of 6 functions to extract and process data from the BME280 temperature-humidity sensor using 
 the SPI protocol.  Modify registers ctrl_hum, ctrl_meas, and config as needed.  Consult the BME280 device documentation.
 */

#ifndef BME280_H_
#define BME280_H_
#define BYTES 20 //Max number of bytes sent on SPI
#define CS_TH BIT5; //Set CSB to P1.5 on MCU.  Must also be set consistently in main program

volatile int32_t RawTemp, RawPress, RawHumid;
volatile uint16_t dig_T1, dig_P1;
volatile int16_t  dig_T2, dig_T3;
volatile int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
volatile int32_t t_fine;
volatile uint8_t dig_H1, dig_H3, i;
volatile int8_t dig_H6;
volatile int16_t dig_H2, dig_H4, dig_H5;
volatile unsigned char  Tempbuf[30], Humbuf[15];
char RXbuf[BYTES], RXdata[BYTES], TXbuf[BYTES], Data[BYTES];

uint8_t ReadTHid(void)  // Get the TH sensor chip ID: 0x60
    {
	 uint8_t FLAG;
	 UCB0CTL1 &= ~UCSWRST; //Start USCI
	 P1OUT &= ~CS_TH; //Pull CSB low
   	 unsigned char TID[]={0xD0,0x00}; //Read followed by dummy byte

   	 for (i=0; i < sizeof TID; i++)
   	 {
   		 while (!(UCB0IFG & UCTXIFG)); //Check if it is OK to write
   		 UCB0TXBUF = TID[i]; //Load data into transmit buffer
   		 while (!(UCB0IFG & UCRXIFG)); //Wait until complete RX byte is received
   		 RXbuf[i] = UCB0RXBUF; //Read buffer to clear RX flag
   	 }
   	  P1OUT |= CS_TH; //Pull CSB line high
  	  UCB0CTL1 |= UCSWRST; //Stop USCI
   	  if (RXbuf[1] == 0x60) FLAG =1;
   	  else FLAG=0;
   	  return FLAG;
    }

void GetCompData(void)
     {
    	/* Compensation data can be read in sleep mode. Use burst read
    	to get temp, pressure, and humidity compensation bytes starting at first data byte 0X88.
    	Retrieve data while CSB is low. */

	UCB0CTL1 &= ~UCSWRST; //Start USCI
	P1OUT &= ~CS_TH; //Pull CSB line low

	while (!(UCB0IFG & UCTXIFG)); //Check if it is OK to write
	UCB0TXBUF = 0x88; //Load first address to be read.
	while (!(UCB0IFG & UCRXIFG)); //Wait until complete RX byte is received
	Tempbuf[0] = UCB0RXBUF; //Read buffer to clear RX flag. The first read byte is of no use

	//Burst mode read of temperature, pressure, and first humidity compensation bytes follows
	for (i=1; i < 27; i++)
	{
		while (!(UCB0IFG & UCTXIFG));
		UCB0TXBUF = 0xAA; //Load dummy data into transmit buffer
		while (!(UCB0IFG & UCRXIFG));
		Tempbuf[i] = UCB0RXBUF;
    	}
    	 P1OUT |= CS_TH; //Pull CSB line high
    	 UCB0CTL1 |= UCSWRST; //Stop USCI

    	// Build the temperature compensation coefficients
    	dig_T1 = ((uint16_t)(Tempbuf[2] << 8) | (uint16_t)Tempbuf[1]); //unsigned 16-bit int
    	dig_T2 = (int16_t)((uint16_t)(Tempbuf[4] << 8) | (uint16_t)Tempbuf[3]); //signed 16-bit int
    	dig_T3 = (int16_t)((uint16_t)(Tempbuf[6] << 8) | (uint16_t)Tempbuf[5]); //signed 16-bit int

    	// Build the pressure compensation coefficients
    	dig_P1 = ((uint16_t)(Tempbuf[8] << 8) | (uint16_t)Tempbuf[7]); //unsigned 16-bit int
    	dig_P2 = (int16_t)((uint16_t)(Tempbuf[10] << 8) | (uint16_t)Tempbuf[9]); //signed 16-bit int
    	dig_P3 = (int16_t)((uint16_t)(Tempbuf[12] << 8) | (uint16_t)Tempbuf[11]); //signed 16-bit int
    	dig_P4 = (int16_t)((uint16_t)(Tempbuf[14] << 8) | (uint16_t)Tempbuf[13]); //signed 16-bit int
    	dig_P5 = (int16_t)((uint16_t)(Tempbuf[16] << 8) | (uint16_t)Tempbuf[15]); //signed 16-bit int
    	dig_P6 = (int16_t)((uint16_t)(Tempbuf[18] << 8) | (uint16_t)Tempbuf[17]); //signed 16-bit int
    	dig_P7 = (int16_t)((uint16_t)(Tempbuf[20] << 8) | (uint16_t)Tempbuf[19]); //signed 16-bit int
    	dig_P8 = (int16_t)((uint16_t)(Tempbuf[22] << 8) | (uint16_t)Tempbuf[21]); //signed 16-bit int
    	dig_P9 = (int16_t)((uint16_t)(Tempbuf[24] << 8) | (uint16_t)Tempbuf[23]); //signed 16-bit int

    	Humbuf[1]=Tempbuf[26]; //This is byte 0xA1

    	UCB0CTL1 &= ~UCSWRST; //Start USCI
    	P1OUT &= ~CS_TH; //Pull CSB line low
    	//Burst read of remaining humidity bytes follows
    	while (!(UCB0IFG & UCTXIFG));
    	UCB0TXBUF = 0xE1; //Load first read address
    	while (!(UCB0IFG & UCRXIFG));
    	Humbuf[0] = UCB0RXBUF; //Read buffer to clear RX flag. This byte is of no use

    	for (i=2; i < 10; i++) //Write dummy variable to produce burst read
    	{
      		while (!(UCB0IFG & UCTXIFG));
             	UCB0TXBUF = 0xAA; //Load dummy data into transmit buffer
             	while (!(UCB0IFG & UCRXIFG));
             	Humbuf[i] = UCB0RXBUF;
    	}

    	 P1OUT |= CS_TH; //Pull CSB line high
    	 UCB0CTL1 |= UCSWRST; //Stop USCI

    	 // Build the humidity compensation coefficients
    	 dig_H1 = (uint8_t)Humbuf[1]; //0xA1
    	 dig_H2 = (int16_t)((uint16_t)(Humbuf[3] << 8) | (uint16_t)Humbuf[2]); //  0xE2/0xE1
    	 dig_H3 = (uint8_t)Humbuf[4];  //0xE3
    	 //dig_H4 and dig_H5 use the lower and upper nibbles of 0xE5, respectively. Split up Humbuf[6] for this purpose
    	 dig_H4 = (int16_t)((uint16_t)(Humbuf[5] << 4) | (uint16_t)(Humbuf[6] & 0x0F)); //0xE4 / low nibble of 0xE5 (12 bits)
    	 dig_H5 = (int16_t)((uint16_t)(Humbuf[7] << 4) | (uint16_t)((Humbuf[6] >>4) & 0x0F)); //0xE7 / high nibble of 0xE5 (12 bits)
    	 dig_H6 = (uint8_t)Humbuf[8]; //0xE7
     }

void ReadTHsensor(void)
   {
  	/* Read from sensor as follows: The ctrl_hum register is written with 0x72 followed by 0x01 for 1x oversampling.
 	The ctrl_meas register is written with 0x74; send 0x21 for temperature only, forced mode, 1x oversampling.
	Send 0x25 for pressure + temperature, forced mode, 1x oversampling.
	Sending 0x74 also wakes up the sensor and enables any changes written to the ctrl_humid register. The config 
	register 0x75 is forced to zero just in case. No filter is used as recommended for low rate polling.
  	Use recommended burst mode read of 8 data registers by sending address of first byte 0XF7. Retrieve
  	data while CSB is low. */

	 //unsigned char TH1[]={0x72,0x01,0x74,0x21,0x75,0x00,0xF7}; //Temperature only
  	 unsigned char TH1[]={0x72,0x01,0x74,0x25,0x75,0x00,0xF7}; //Temp + pressure
  	 UCB0CTL1 &= ~UCSWRST; //Start USCI
  	 P1OUT &= ~CS_TH; //Pull CSB line low

  	 for (i=0; i < sizeof TH1; i++) //Write above data via SPI.  No useful data is read but save anyway
  	 {
  		 while (!(UCB0IFG & UCTXIFG)); //Check if it is OK to write
  		 UCB0TXBUF = TH1[i]; //Load data into transmit buffer
  		 while (!(UCB0IFG & UCRXIFG)); //Wait until complete RX byte is received
  		 RXbuf[i] = UCB0RXBUF; //Read buffer to clear RX flag. These bytes are of no use
  	 }
  	// Now do a burst read of 8 data bytes (press, temp, humid) from BME280.
  	for (i=0; i < 9; i++) //Burst read.  Address 0x07 auto-increments
  	{
  		while (!(UCB0IFG & UCTXIFG)); //Check if it is OK to write
  		UCB0TXBUF = 0xAA; //Load dummy data into transmit buffer
  		while (!(UCB0IFG & UCRXIFG)); //Wait until complete RX byte is received
  		Data[i] = UCB0RXBUF; //Read buffer to clear RX flag
  	}
 	 P1OUT |= CS_TH; //Pull CSB line high
 	 UCB0CTL1 |= UCSWRST; //Stop USCI
	/*
	Data[0]: Pressure  MSB
  	Data[1]: Pressure LSB
  	Data[2]: Pressure XLSB
  	Data[3]: Temperature  MSB
  	Data[4]: Temperature LSB
  	Data[5]: Temperature XLSB
  	Data[6]: Humidity MSB
  	Data[7]: Humidity LSB

	Assemble the data bytes to make a 20-bit long integer for pressure and temperature and a 16-bit integer for humidity */
 	RawPress = ((uint32_t)Data[0] << 16 | (uint32_t)Data[1] << 8 | Data[2]) >> 4; //20-bit long unsigned integer
  	RawTemp = ((uint32_t)Data[3] << 16 | (uint32_t)Data[4] << 8 | Data[5]) >> 4; //20-bit long unsigned integer
 	RawHumid = ((uint32_t)Data[6] << 8) | (uint32_t)Data[7]; //16-bit unsigned integer
   }

//The following functions make integer data conversions appropriate for the MSP430

int32_t CalcTemp(void) //32-bit integer conversion formula from BME280 spec sheet
{
  	volatile int32_t var1, var2, T;
  	var1 = (((((int32_t)RawTemp >> 3) - ((int32_t)dig_T1 << 1))) * (int32_t)dig_T2) >> 11;
  	var2 = (((int32_t)RawTemp >> 4) - (int32_t)dig_T1);
  	var2 = (((var2*var2) >> 12) * (int32_t)dig_T3) >> 14;
  	t_fine = var1 + var2;
  	T = ((t_fine * 5) + 128) >> 8;
  	return T;
}

uint32_t CalcHumid(void) //Implement integer conversion formula from BME280 spec sheet
{
  	volatile int32_t var3;
  	var3 = t_fine - (int32_t)76800;
  	var3 = ((((((int32_t)RawHumid << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * var3)) +
  		((int32_t)16384)) >> 15) * (((((((var3 * ((int32_t)dig_H6)) >> 10) * (((var3 *
  		((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
  		((int32_t)dig_H2) + (int32_t)8192) >> 14));
  	var3 = (var3 - (((((var3 >> 15) * (var3 >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
  	if(var3 < 0) var3 = 0;
  	if(var3 > 419430400) var3 = 419430400;
  	return (uint32_t)(var3 >> 12);
}

uint32_t CalcPress(void) //32-bit integer conversion formula from BME280 spec sheet
{
	volatile int32_t var4, var5;
	volatile uint32_t p;
	var4 = (((int32_t)t_fine)>>1) - (int32_t)0xFA00;
	var5 = (((var4>>2) * (var4>>2)) >> 11 ) * ((int32_t)dig_P6);
	var5 = var5 + ((var4*((int32_t)dig_P5))<<1);
	var5 = (var5>>2)+(((int32_t)dig_P4)<<16);
	var4 = (((dig_P3 * (((var4>>2) * (var4>>2)) >> 13 )) >> 3) + ((((int32_t)dig_P2) * var4)>>1))>>18;
	var4 = ((((0x8000+var4))*((int32_t)dig_P1))>>15);
	if (var4 == 0)
	{
		return 0; // Avoid exception caused by division by zero
	}
	p = (((uint32_t)(((int32_t)0x100000)-RawPress)-(var5>>12)))*0xC35;
	if (p < 0x80000000)
	{
		p = (p << 1) / ((uint32_t)var4);
	}
	else
	{
		p = (p / (uint32_t)var4) * 2;
	}
	var4 = (((int32_t)dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var5 = (((int32_t)(p>>2)) * ((int32_t)dig_P8))>>13;
	p = (uint32_t)((int32_t)p + ((var4 + var5 + dig_P7) >> 4));
	return p;
}
#endif /* BME280_H_ */
