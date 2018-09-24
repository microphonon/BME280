# BME280
Demo programs that use SPI to communicate between Bosch BME280 temperature-humidity-pressure sensor and MSP430FX5XX/FX6XX or MSP430FR59xx families of MCUs

Read and display temperature, relative humidity, and pressure with BME280 sensor on F5529 or FR5969 Launchpads.  Sensor polled through SPI in forced mode with 1x oversampling and no filter. Internal trimming parameters must be read from device to perform conversion of raw data.  Data displayed on terminal program. Set serial port for 9600 baud, 8-bits, 1 stop, no parity, no flow control. UART polling with no RX interrupt.  SPI clock 1 MHz. The CS line must be configured consistently in the accompanying BME280.h header file. Main loop runs with timed interrupt from LPM3 and VLO clock. IDE with CCS 6.1.3 and nofloat printf support. 

<p><b>BME280_F.c</b> Can be directly implemented on F5529 Launchpad using the following pins:
<p>P3.0  UCB0 MOSI  
<br>P3.1  UCB0 MISO
<br>P3.2  UCB0 CLK  SPI
<br>P3.3  UCA0TXD  
<br>P3.4  UCA0RXD
<br>P1.5  CS for BME280

<p><b>BME280_FR.c</b> Can be directly implemented on FR5969 Launchpad using the following pins:
<p>P1.6  UCB0 MOSI
<br>P1.7  UCB0 MISO
<br>P2.2  UCB0 SPI CLK
<br>P1.5  CS for BME280
<br>P2.5  UCA1TXD
<br>P2.6  UCA1RXD
 
<p><b>BME280.h</b>    Library of 6 functions to extract and process data from the BME280 sensor using the SPI protocol. Modify registers ctrl_hum, ctrl_meas, and config as needed.  Consult the BME280 data sheet.

