#include <msp430.h>
int ECGsample;
int check;
int heartBeat=0;
int filteredECG=0;
int temp=0;
double rPeaks=0;
double tInt=0;
//Low pass variables
double prevECG[4]={0,0,0,0,0};
int outECGlp=0;
//high pass
double xECGValues[4] = {0,0,0,0};
double yECGValues[4] = {0,0,0,0};
int outECG=0;

void highPassFilter(long ECGcIn){
    unsigned int i;
    for(i=0;i<3;i++){
        xECGValues[i]=xECGValues[i+1];
        yECGValues[i]=yECGValues[i+1];
    }
    xECGValues[3]=ECGcIn;
    yECGValues[3]=((xECGValues[0]*(-0.9710))+(xECGValues[1]*2.9130)+(xECGValues[2]*(-2.9130))+(xECGValues[3]*0.9710))-((yECGValues[0]*(-0.9423))+(yECGValues[1]*2.8840)+(yECGValues[2]*(-2.9417)));
    tInt++;
    if ((yECGValues[3] > yECGValues[2]) && (yECGValues[3] > 1.5)){
       rPeaks++;
    }
    heartBeat = rPeaks/tInt;
    outECG=(int)yECGValues[3];
}
void lowPassFilter(long ECGIn){
    unsigned int i;
    for(i=0;i<3;i++){
        prevECG[i]=prevECG[i+1];
    }
    prevECG[3]=ECGIn;
    outECGlp = 0.17350*prevECG[0]+0.32649*prevECG[1]+0.32649*prevECG[2]+0.17350*prevECG[3];
    outECGlp=(int)outECGlp;
}

int main(void)
{
    check = 1;
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
    UCA0BR0 = 104;              // 1MHz/104=9615, consider doing faster 115200
    UCA0BR1 = 0;                // 1MHz 115200
    UCA0MCTL = UCBRS_1;         // Modulation UCBRSx = 5
    UCA0CTL1 &= ~UCSWRST;       // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;            // Enable USCI_A0 RX interrupt
    // TIMER CONFIGS
    TA0CTL |= TASSEL_2;         // SMCLK
    //Timer MC_0:stop MC_2:starts counting in continous mode
    while(1){
        __bis_SR_register(LPM0_bits + GIE);  // Enter LPM0, interrupts enabled
        if (check == 0){
            temp = UCA0RXBUF;            // LSB
        }
        __bis_SR_register(LPM0_bits + GIE);  // Enter LPM0, interrupts enabled
        if (check == 1) {
            ECGsample = UCA0RXBUF; // MSB
            ECGsample = ECGsample << 8; // shift MSB to left
            ECGsample |= temp;          // new = MSB + LSB
            TA0CTL |= MC_2;             // start timer
            lowPassFilter(ECGsample);
            highPassFilter(outECGlp);
            TA0CTL |= MC_0;             // end timer

            filteredECG = (int) outECG;
            while (!(IFG2 & UCA0TXIFG));                // USCI_A0 TX buffer ready?
            UCA0TXBUF = filteredECG;                    // send LSB
            filteredECG = filteredECG >> 8;             // send MSB
            while (!(IFG2 & UCA0TXIFG));                // USCI_A0 TX buffer ready?
            UCA0TXBUF = filteredECG;

            while (!(IFG2 & UCA0TXIFG));                // USCI_A0 TX buffer ready?
            UCA0TXBUF = TAR;                            // send LSB
            TAR = TAR >> 8;                             // send MSB
            while (!(IFG2 & UCA0TXIFG));                // USCI_A0 TX buffer ready?
            UCA0TXBUF = TAR;

            heartBeat = (int) heartBeat;
            while (!(IFG2 & UCA0TXIFG));                // USCI_A0 TX buffer ready?
            UCA0TXBUF = heartBeat;                      // send LSB
            heartBeat = heartBeat >> 8;                 // send MSB
            while (!(IFG2 & UCA0TXIFG));                // USCI_A0 TX buffer ready?
            UCA0TXBUF = heartBeat;
            TAR = 0;                                    // reset
        }
    }
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer0_A0(void){
    if(TAR < 0){
        TAR += 0xFFFF; // handle overflow
    }
}

#pragma vector =USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void){
    if (check == 0){
        check = 1;    // read next byte when new data point comes in
        __bic_SR_register_on_exit(LPM0_bits + GIE); // check w user guide
    }
    else{
        check = 0;
        __bic_SR_register_on_exit(LPM0_bits + GIE);
    }
}

