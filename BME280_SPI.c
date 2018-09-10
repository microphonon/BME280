#include <msp430.h> 
#include <stdio.h>
#include <stdint.h>
#include "BME280.h"

/*
 Read and display temperature and relative humidity with BME280 sensor on F5529 Launchpad.  
 Sensor polled in forced mode using the SPI interface. Internal trimming parameters must be
 read from device to perform conversion of raw data.  Data displayed on terminal program. 
 Set serial port for 9600 baud, 8-bits, 1 stop, no parity, no flow control. UART interface is 
 on TXD (P3.3) and RXD (P3.4); these ports are reversed on the receiving device, ie. the RX-TX 
 are switched on the PC interface cable. UART polling with no RX interrupt. Green LED on P4.7 
 illuminates during data transmission. SPI lines are on P3.0 (MOSI), P3.1 (MISO), P3.2 (CLK), 
 and P1.5 (CS) of UCB0 module. SPI clock 1 MHz.The CS line must be configured consistently in
 the BME280.h file. Main loop runs with timed interrupt from LPM3 and VLO clock. IDE with 
 CCS 6.1.3 and nofloat printf support. Launchpad pins:

	P3.0  MOSI  SPI
	P3.1  MISO  SPI
	P3.2  CLK  SPI
	P3.3  TXD  UART
	P3.4  RXD  UART
	P1.5  CS  SPI

February 2018
 */

# define PERIOD 10000 //Samping period. 10000 count is approximately 1 second; maximum is 65535

void SetTimer(void);
void SetVLO(void);
void SetPins(void);
void SetUART(void);
void SetSPI(void);

//Variables for UART terminal display
char str[80];
volatile uint8_t i,count;

void main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    SetPins();
    SetVLO();
    SetTimer();
    SetUART();
    SetSPI();

    _BIS_SR(GIE); //Enable global interrupts.

    if(ReadTHid()); //Check for presence of sensor; read its ID code
    else  //Trap CPU and turn on red LED if not found
    {
    	P1OUT |= BIT0;
    	while(1){}
    }

    //Setup the BME280 for temp and humidity measurement in forced mode
    SetTHsensor();
    //Get the compensation coefficients from device for raw data conversion
    GetCompData();

    while(1)
    {
    	 TA0CCR0 = PERIOD; // Polling period
    	 LPM3;		//Wait in low power mode
    	 P4OUT |= BIT7; //Timeout. Turn on green LED on Launchpad
    	 ReadTHsensor(); //Burst read on SPI to get 3 temp data bytes and 2 humidity data bytes
    	 //Apply cal factors to raw data
    	 CorT = CalcTemp(); //Corrected temperature
    	 CorH = CalcHumid(); //Corrected humidity
    	 //Send data to serial port for display
    	 sprintf(str,"%s %lu.%.2lu%s %lu.%.2lu%s", "Temperature:", CorT/100, CorT%100,"C Rel Humidity:",
    			 (int32_t)(CorH/1000), (int32_t)(CorH%100),"%\r\n\n");
    	 count = sizeof str;
    	 for (i=0; i < count; i++)
    	 {
    		 while (!(UCA0IFG & UCTXIFG)); // USCI_A0 TX buffer ready?
    		 UCA0TXBUF = str[i]; //Send data 1 byte at a time
    	 }
    	 P4OUT &= ~BIT7; //Turn off LED
    }
}

#pragma vector=TIMER0_A0_VECTOR //This vector name is in header file
 __interrupt void timerfoo (void) //The name of this interrupt (timerfoo) is arbitrary
 {
	 LPM3_EXIT;
 }

 void SetPins(void)
  {
	 	 /* Port 1
			P1.0 Red LED
 			P1.5 TH Select.  Pull this CSB line low to enable BME280 communication
 			CS_TH must also be defined in BME280.h
	 	  */
 	    	P1DIR |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7;
 	    	P1OUT &= ~BIT0; //LED off

 	    	/* Port 2
 	    	P2.1  Button on Launchpad
 	    	 */
 	    	P2DIR |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7;

 	    	/* Port 3 */
 	    	P3SEL = BIT0 + BIT1 + BIT2 + BIT3 + BIT4; //SPI + UART lines
 	    	P3DIR |= BIT5 + BIT6 + BIT7; //Unused lines

 	    	/* Port 4
 	   		P4.0--4.6 unused
 	   		P4.7 Green LED
 	    	 */
 	   	    P4DIR |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7;
 	   	    P4OUT &= ~BIT7;

 	   	 /* Port 5
 	   	    P5.0 Unused
 	   	    P5.1 Unused
 	   	    P5.2--P5.5 grounded or open as per spec sheet
 	   	  */
 	   	    P5DIR |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7;

 	   	/* Port 6
 	   		P6.0--6.7 unused
 	   	*/
 	   	    P6DIR |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7;
  }

 void SetVLO(void)
    { //Default frequency ~ 10 kHz
	 	 UCSCTL4 |= SELA_1;  //Set ACLK to VLO
    }

 void SetTimer(void)
     {
 	 	TA0CCTL0 |= CCIE;  //Enable timer interrupt
 	 	TA0CTL = TASSEL_1 | MC_1;  //Set Timer A to ACLK; MC_1 to count up to TA0CCR0.
     }

 void SetUART(void)
  {
 	 UCA0CTL1 |= UCSWRST;                      // Reset to configure
 	 UCA0CTL1 |= UCSSEL_2;                     // SMCLK
 	 UCA0BR0 = 6;                              // 1MHz 9600
 	 UCA0BR1 = 0;                              // 1MHz 9600
 	 UCA0MCTL = UCBRS_0 + UCBRF_13 + UCOS16;   // Modln UCBRSx=0, UCBRFx=0,
 	 UCA0CTL1 &= ~UCSWRST;                     // Initialize
  }

 void SetSPI(void)
   {
  	 // Configure the USCI module: 3-pin SPI
	 UCB0CTL1 |= UCSSEL_2 + UCSWRST;  //Select SMCLK; should be in reset state at PUC but set anyway
	 UCB0CTL0 |= UCMST + UCSYNC + UCMSB + UCCKPH; //Set as master, synchronous, MSB first, clock phase adjust
  	 //UCB0BR0 |= 0x02; //Divide SMCLK by 2 to clock at 500 kHz
	 UCB0BR0 |= 0x01; //1 MHz SPI clock
	 UCB0BR1 |= 0x00;
   }

