# BME280
Demo program that uses SPI to communicate between Bosch BME280 and MSP430X5XX/MSP430X6XX family of MCUs

<p><b>BME280_SPI.c</b>    Read and display temperature and relative humidity with BME280 sensor on F5529 Launchpad.  
 Sensor polled in forced mode using the SPI interface. Internal trimming parameters must be
 read from device to perform conversion of raw data.  Data displayed on terminal program. 
Set serial port for 9600 baud, 8-bits, 1 stop, no parity, no flow control. UART interface is on TXD (P3.3) and RXD (P3.4); these ports are reversed on the receiving device, ie. the RX-TX are switched on the PC interface cable. UART polling with no RX interrupt. Green LED on P4.7 illuminates during data transmission. SPI lines are on P3.0 (MOSI), P3.1 (MISO), P3.2 (CLK) of UCB0 module
and P1.5 (CS). SPI clock 1 MHz. The CS line must be configured consistently in the accompanying BME280.h header file. Main loop runs with timed interrupt from LPM3 and VLO clock. IDE with CCS 6.1.3 and nofloat printf support. Launchpad pins:
<p>P3.0  MOSI  SPI
<br>P3.1  MISO  SPI
<br>P3.2  CLK  SPI
<br>P3.3  TXD  UART
<br>P3.4  RXD  UART
<br>P1.5  CS  SPI
 
 <p><b>BME280.h</b>    Library of 6 functions to extract and process data from the BME280 temperature-humidity sensor using 
	the SPI protocol.

