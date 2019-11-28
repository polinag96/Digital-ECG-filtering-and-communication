#include <msp430.h>
long int ECGsample;
int temp;
long int runTime;
long int heartBeat;
int check = 0;

int main(void)
{
    // UART CONFIGS
    WDTCTL = WDTPW + WDTHOLD;  //Stop WDT
    if(CALBC1_1MHZ==0xFF)
    {
        while(1);
    }
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;      // Set DCO
    DCOCTL = CALDCO_1MHZ;
    P1SEL = BIT1 + BIT2 ;       // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2;       // port 2.1 and 2.2 select
    UCA0CTL1 |= UCSSEL_2;       // SMCLK
    UCA0BR0 = 104;              // 1MHz/104=9615
    UCA0BR1 = 0;                // 1MHz 115200
    UCA0MCTL = UCBRS_1;         // Modulation UCBRSx = 5
    UCA0CTL1 &= ~UCSWRST;       // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;            // Enable USCI_A0 RX interrupt
    // TIMER CONFIGS
    TA0CTL |= TASSEL_2; 		// SMCLK	
    //Timer MC_0:stop MC_2:starts counting in continous mode
    __bis_SR_register(LPM0_bits + GIE);  // Enter LPM0, interrupts enabled
    while(1){
    	if (check == 0){
    		__delay_cycles(1000000);	// delay for next byte
    		temp = UCA0RXBUF; 			// LSB
    	}
    	else {
    		__delay_cycles(1000000);	// delay for next byte
    		ECGsample = UCA0RXBUF;		// MSB
    		ECGsample = ECGsample << 8;	// shift MSB to left
    		ECGsample |= temp;			// new = MSB + LSB
    		TA0CTL |= MC_2; 			// start timer 	
    		// call functions
    		TA0CTL |= MC_0;				// end timer
    		runTime = TAR;				// init timer exec time
    		TAR = 0; 					// reset 

    	}
    	// send filtered EMG, exec time, and HR
    	while (!(IFG2 & UCA0TXIFG));                // USCI_A0 TX buffer ready?
    	UCA0TXBUF = filteredECG;					// send LSB 
    	filteredECG = filteredECG >> 8;				// send MSB
    	while (!(IFG2 & UCA0TXIFG));                // USCI_A0 TX buffer ready?
    	UCA0TXBUF = filteredECG;

    	while (!(IFG2 & UCA0TXIFG));                // USCI_A0 TX buffer ready?
    	UCA0TXBUF = runTime;						// send LSB 
    	runTime = runTime >> 8;						// send MSB
    	while (!(IFG2 & UCA0TXIFG));                // USCI_A0 TX buffer ready?
    	UCA0TXBUF = runTime;

    	while (!(IFG2 & UCA0TXIFG));                // USCI_A0 TX buffer ready?
    	UCA0TXBUF = heartBeat;						// send LSB 
    	heartBeat = heartBeat >> 8;					// send MSB
    	while (!(IFG2 & UCA0TXIFG));                // USCI_A0 TX buffer ready?
    	UCA0TXBUF = heartBeat;
    }
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	if (check == 0) check = 1;						// read next byte when new data point comes in
	else check = 0;
}
