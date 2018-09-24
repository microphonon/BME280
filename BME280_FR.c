#include <msp430.h> 
#include <stdio.h>
#include <stdint.h>

#include "BME280.h"

/*
 Read and display temperature, relative humidity, and pressure with BME280 sensor on FR5969 Launchpad.
 Sensor polled in forced mode using the SPI interface. Internal trimming parameters must be
 read from device to perform conversion of raw data.  Data displayed on terminal program.
 Set serial port for 9600 baud, 8-bits, 1 stop, no parity, no flow control. UART interface is
 on TXD (P2.5) and RXD (P2.6); these ports are reversed on the receiving device, ie. the RX-TX
 are switched on the PC interface cable. UART polling with no RX interrupt. Green LED on P1.0
 illuminates during data transmission. SPI lines are on P1.6 (MOSI), P1.7 (MISO), P2.2 (CLK),
 and P1.5 (CS) of UCB0 module. SPI clock 1 MHz.The CS line must be configured consistently in
 the BME280.h file. Main loop runs with timed interrupt from LPM3 and VLO clock. IDE with
 CCS 6.1.3 and nofloat printf support. Launchpad pins:

	P1.6  UCB0 MOSI
	P1.7  UCB0 MISO
	P2.2  UCB0 SPI CLK
	P1.5  CS for BME280
	P2.5  UCA1TXD
	P2.6  UCA1RXD
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
volatile int32_t CorT;
volatile uint32_t CorH, CorP;

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
    	P4OUT |= BIT6;
    	while(1){}
    }

    //Get the compensation coefficients from device for raw data conversion
    GetCompData();

    while(1)
    {
    	 TA0CCR0 = PERIOD; // Polling period
    	 LPM3;		//Wait in low power mode
    	 P1OUT |= BIT0; //Timeout. Turn on green LED on Launchpad
    	 //Burst read on SPI to get 3 press data bytes, 3 temp bytes and 2 humidity bytes
    	 ReadTHsensor();
    	 //Apply cal factors to raw data
    	 CorT = CalcTemp(); //Corrected temperature
    	 CorH = CalcHumid(); //Corrected humidity
    	 CorP = CalcPress(); //Corrected pressure
    	 //Send data to serial port for display
    	 sprintf(str,"%s %lu.%.2lu%s %lu.%.2lu%s %lu.%.2lu%s", "Temperature:", CorT/100, CorT%100,"C Rel Humidity:",
    			 CorH/1000, CorH%100,"% Pressure:",CorP/100, CorP%100," hPa\r\n\n");
    	 count = sizeof str;
    	 for (i=0; i < count; i++)
    	 {
    		 while (!(UCA1IFG & UCTXIFG)); // USCI_A0 TX buffer ready?
    		 UCA1TXBUF = str[i]; //Send data 1 byte at a time
    	 }
    	 P1OUT &= ~BIT0; //Turn off LED
    }
}

#pragma vector=TIMER0_A0_VECTOR
 __interrupt void timerfoo (void)
{
	LPM3_EXIT;
}

void SetPins(void)
  {
	 	PM5CTL0 &= ~LOCKLPM5; //Unlocks GPIO pins at power-up
 	 /* Port 1
 	  	P1.0 Green LED
 	  	P1.1 Launchpad switch
 	  	P1.5 Chip select.  Pull this line low to enable BME280 communication
 			 CS_TH must also be defined in BME280.h
 	    P1.6 MOSI
 	    P1.7 MISO
 	    */
 	    P1DIR |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5;
 	    P1SEL1 |= BIT6 + BIT7; //Configure pins for SPI on UCB0
 	    P1OUT &= ~BIT0; //LED off

 	    /* Port 2
 	    P2.1  Button on Launchpad
 	    P2.2 SPI CLK
 	    P2.5 TXD UART
 	    P2.6 RXD UART
 		*/
 	   	P2DIR |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT7;
 	    P2SEL1 |= BIT2 + BIT5 + BIT6; //Configure pins for SPI CLK; UART on UCA1

 	    /* Port 3 */
 	    P3DIR |=  BIT0 + BIT1 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7;

 	    /* Port 4
 	   	P4.6 Red LED
 	   	*/
 	    P4DIR |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7;
 	    P4OUT &= ~BIT6; //LED off
  }

 void SetVLO(void)
 {
	 CSCTL0 = CSKEY; //Password to unlock the clock registers
	 //Default frequency ~ 10 kHz
	 CSCTL2 |= SELA__VLOCLK;  //Set ACLK to VLO
	 CSCTL0_H = 0xFF; //Re-lock the clock registers
    }

 void SetTimer(void)
 {
	 //Enable the timer interrupt, MC_1 to count up to TA0CCR0, Timer A set to ACLK (VLO)
	 TA0CCTL0 = CCIE;
	 TA0CTL |= MC_1 + TASSEL_1;
 }

 void SetUART(void) //UCA1 module; do simple polling instead of interrupts
  {
 	 UCA1CTLW0 |= UCSWRST;
 	//Next line selects SMCLK which is DCO (default frequency: 1 MHz)
 	 UCA1CTLW0 |=  UCSSEL1; //This writes 0x80 which sets BIT7
 	 //Next two lines divide 1 MHz to get 9600 baud
 	 UCA1BRW = 0x06;
 	 UCA1MCTLW |= UCOS16 + UCBRF3 + UCBRS5;
 	 UCA1CTLW0 &= ~UCSWRST;
  }

 void SetSPI(void)
   {
	 // Configure the eUSCI_B0 module for 3-pin SPI at 1 MHz
	 UCB0CTLW0 |= UCSWRST;
  	 // Use SMCLK at 1 MHz;
	 UCB0CTLW0 |= UCSSEL__SMCLK + UCMODE_0 + UCMST + UCSYNC + UCMSB + UCCKPH;
	 //UCB0BR0 |= 0x02; //Divide SMCLK by 2 to clock at 500 kHz
	 UCB0BR0 |= 0x01; //1 MHz SPI clock
	 UCB0BR1 |= 0x00;
	 UCB0CTLW0 &= ~UCSWRST;
   }


