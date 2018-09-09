# BME280
Demo program that uses SPI to communicate between Bosch BME280 and MSP430X5XX/MSP430X6XX

<p>Read and display temperature with BME280 sensor on F5529 Launchpad.  Sensor polled on the SPI interface.
 Temperature data sent to terminal at 9600 baud.  Display with terminal program such as putty.
 Set serial port for 8-bits, 1 stop, no parity, no flow control. UART interface is on TXD (P3.3) and RXD (P3.4).
 Note that these ports are reversed on the receiving device, ie. the RX-TX are switched on the PC interface cable.
 No RX interrupt is needed. Green LED on P4.7 illuminates during data transmission. Main loop runs with
 timer interrupt.  SPI lines are on P3.0 (MOSI), P3.1 (MISO), P3.2 (CLK), and P1.5 (CS) of UCB0 module.
 Raw data conversion done using unique internal compensation data that gets read from BME280.
