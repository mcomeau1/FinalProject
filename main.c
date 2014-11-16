// ******************************************************************************************* //
// Include file for PIC24FJ64GA002 microcontroller. This include file defines
// MACROS for special function registers (SFR) and control bits within those
// registers.

#include "p24fj64ga002.h"
#include <stdio.h>
#include <string.h>
#include "lcd.h"


// ******************************************************************************************* //
// Configuration bits for CONFIG1 settings.
//
// Make sure "Configuration Bits set in code." option is checked in MPLAB.
//
// These settings are appropriate for debugging the PIC microcontroller. If you need to
// program the PIC for standalone operation, change the COE_ON option to COE_OFF.

_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF &
		 BKBUG_ON & COE_ON & ICS_PGx1 &
		 FWDTEN_OFF & WINDIS_OFF & FWPSA_PR128 & WDTPS_PS32768 )

// ******************************************************************************************* //
// Configuration bits for CONFIG2 settings.
// Make sure "Configuration Bits set in code." option is checked in MPLAB.

_CONFIG2( IESO_OFF & SOSCSEL_SOSC & WUTSEL_LEG & FNOSC_PRIPLL & FCKSM_CSDCMD & OSCIOFNC_OFF &
		 IOL1WAY_OFF & I2C1SEL_PRI & POSCMOD_XT )

#define XTFREQ          7372800         	  // On-board Crystal frequency
#define PLLMODE         4               	  // On-chip PLL setting (Fosc)
#define FCY             (XTFREQ*PLLMODE)/2    // Instruction Cycle Frequency (Fosc/2)
// ******************************************************************************************* //
volatile int state = 0;
volatile int nextState = 1;

//void wait()
//{
//    TMR1 = 0;
//    PR1 = 580;
//    IFS0bits.T1IF = 0;
//    T1CON = 0x8030;
//    while(IFS0bits.T1IF == 0);
//    T1CONbits.TON = 0;
//}
int main(void){

        int adc1 = 0;
        int adc2 = 0;
        int adc3 = 0;
        int left = 0;
        int right = 0;
        int counter = 0;
        int travelState = 2;
        int reachedEnd = 0;

	double ADC_value;
        char value[8];
        char Val1[8], Val2[8];

        TMR2 = 0;
        PR2 = 512;
        IFS0bits.T2IF = 0;
        IEC0bits.T2IE = 1;
        T2CON = 0x8030;

        OC1CON = 0x6;
        OC1RS = 0;

        OC2CON = 0x6;
        OC2RS = 0;

	LCDInitialize( );

        /**********************************************************************/
        //  AN0, AN1, AN4, and AN5 pins are analog:
	AD1PCFG &= 0xFFCC;

        /**********************************************************************/
        //  Scan each of the following input channels AN0, AN1, AN4, and AN5
        AD1CSSL = 0x0033;
        
        /**********************************************************************/
        //  Non-functional when device is idle, 
        //  Internal counter trigers conversion,
        //  Auto start after convert
        AD1CON1 = 0x20E4;

        /**********************************************************************/
        //  Configure A/D voltage reference,
        //  Set AD1IF ever 4 samples
	AD1CON2 = 0x043C;

        /**********************************************************************/
        //  4*TAD
        //  2*TCY
	AD1CON3 = 0x0406;

        /**********************************************************************/
        //  Unused reminants leftover from lab3
	//AD1CHS = 5; 		
	//AD1CSSL = 0;

	AD1CON1bits.ADON = 1;   // Turn on A/D
        IFS0bits.AD1IF = 0;     // Clear A/D conversion interrupt.

        TRISBbits.TRISB5 = 1;   // SW1 -> Used in lab three to change direction
        CNEN2bits.CN27IE = 1;   // SW1 change notification enabled
        IFS1bits.CNIF = 0;
        IEC1bits.CNIE = 1;



	while(1) {

            switch(state) {
                case 0:
                    TRISBbits.TRISB10 = 0;
                    TRISBbits.TRISB11 = 0;
                    TRISBbits.TRISB8 = 0;
                    TRISBbits.TRISB9 = 0;
                    RPOR4bits.RP8R = NULL;
                    RPOR4bits.RP9R = NULL;
                    RPOR5bits.RP10R = NULL;
                    RPOR5bits.RP11R = NULL;
                    nextState = 1;
                    break;
                case 1:
                    TRISBbits.TRISB10 = 0;
                    TRISBbits.TRISB11 = 0;
                    RPOR4bits.RP8R = 18; //OC1 (10 - foward, 8 - backward)
                    RPOR4bits.RP9R = 19; //OC2 (11 - foward, 9 - backward)
                    if (ADC1BUF2 < 384 && ADC1BUF1 < 384 && ADC1BUF0 < 384) {
                        adc1 = 0;
                        adc2 = 0;
                        adc3 = 768;
                    } else {
                        state = 2;
                    }
                    break;
                case 2:
                    TRISBbits.TRISB10 = 0;
                    TRISBbits.TRISB11 = 0;
                    RPOR4bits.RP8R = 18; //OC1 (10 - foward, 8 - backward)
                    RPOR4bits.RP9R = 19; //OC2 (11 - foward, 9 - backward)
                    if (reachedEnd == 0) {
                        if (ADC1BUF2 > 384 && ADC1BUF1 > 384 && ADC1BUF0 > 384 && travelState == 2) {
                            if (left == 1 && right == 0) {
                                adc1 = 250;
                                adc2 = 0;
                                adc3 = 768;
                            }
                            else if (right == 1 && left == 0) {
                                adc1 = 0;
                                adc2 = 250;
                                adc3 = 768;
                            }
                            else {
                                adc1 = 0;
                                adc2 = 0;
                                adc3 = 768;
                            }
                            travelState = 0;
                        }
                        else if (ADC1BUF2 < 382 && left == 1) {
                            adc1 = 0;
                            adc2 = 600;
                            adc3 = 768;
                        }
                        else if (ADC1BUF2 < 382 && right == 1) {
                            adc1 = 600;
                            adc2 = 0;
                            adc3 = 768;
                        }
                        else {
                            if (ADC1BUF2 > 384) {
                                adc3 = 768;
                            }
                            else {
                                adc3 = 0;
                            }

                            if (ADC1BUF1 > 384) {
                                adc2 = 768;
                            }
                            else {
                                adc2 = 0;
                                left = 1;
                                right = 0;
                            }

                            if (ADC1BUF0 > 384) {
                                adc1 = 768;
                            }
                            else {
                                adc1 = 0;
                                right = 1;
                                left = 0;
                            }
                        }
                    }
                    else {
                        int i = 0;
                        for (i = 0; i < 5000; i++) {
                            TRISBbits.TRISB11 = 0;
                            TRISBbits.TRISB8 = 0;
                            RPOR4bits.RP8R = 18;
                            RPOR4bits.RP9R = 0;
                            RPOR5bits.RP10R = 0;
                            RPOR5bits.RP11R = 19;
                            adc1 = 0;
                            adc2 = 0;
                            adc3 = 768;
                        }
                        reachedEnd = 0;
                    }
                    break;
//                    TRISBbits.TRISB10 = 0;
//                    TRISBbits.TRISB11 = 0;
//                    TRISBbits.TRISB8 = 0;
//                    TRISBbits.TRISB9 = 0;
//                    RPOR4bits.RP8R = NULL;
//                    RPOR4bits.RP9R = NULL;
//                    RPOR5bits.RP10R = NULL;
//                    RPOR5bits.RP11R = NULL;
//                    nextState = 3;
//                    break;
//                case 3:
//                    TRISBbits.TRISB8 = 0;
//                    TRISBbits.TRISB9 = 0;
//                    RPOR5bits.RP10R = 18; //OC1 (10 - foward, 8 - backward)
//                    RPOR5bits.RP11R = 19; //OC2 (11 - foward, 9 - backward)
//                    nextState = 0;
//                    break;
//                case 4:
//                    TRISBbits.TRISB11 = 0;
//                    TRISBbits.TRISB8 = 0;
//                    RPOR4bits.RP8R = 18;
//                    RPOR4bits.RP9R = 0;
//                    RPOR5bits.RP10R = 0;
//                    RPOR5bits.RP11R = 19;
//                    break;
            }

            switch (travelState) {
                case 0:
                    counter++;
                    if (counter == 3) {
                        reachedEnd = 1;
                    }
                    travelState = 1;
                    break;
                case 1:
                    if (ADC1BUF0 < 384 && ADC1BUF1 < 384) {
                        travelState = 2;
                    }

                    break;
                case 2:
                    break;
            }

            while (IFS0bits.AD1IF ==0){};   // AD1CON1bits.DONE can be checked instead
            IFS0bits.AD1IF = 0;

            ADC_value = ADC1BUF0 * 3.3 / 1023;   // 0 <= ADC <= 3.3
            sprintf(value, "%.3f", ADC_value );
            LCDMoveCursor(0,0); LCDPrintString(value);

            OC1RS = adc3 - adc2;   //OC1RS -> ADC1BUF0 -> AN0 -> left IR
            OC2RS = adc3 - adc1;   //OC2RS -> ADC1BUF1 -> AN1 -> right IR

            sprintf(Val1, "%4d", counter);
            LCDMoveCursor(1,0);
            LCDPrintString(Val1);

            sprintf(Val2, "%4d", travelState);
            LCDMoveCursor(1,4);
            LCDPrintString(Val2);
	}
	return 0;
}

void __attribute__((interrupt,auto_psv)) _T2Interrupt(void){
    IFS0bits.T2IF = 0;
}

void __attribute__((interrupt,auto_psv)) _CNInterrupt(void){
    IFS1bits.CNIF = 0;
    state = nextState;
    while(PORTBbits.RB5 == 0);
}