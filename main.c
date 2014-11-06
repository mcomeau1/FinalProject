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
	double ADC_value;
        char value[8];
        char Val1[8], Val2[8];
        TMR2 = 0;
        PR2 = 512;
        IFS0bits.T2IF = 0;
        IEC0bits.T2IE = 1;
        T2CON = 0x8030;

        OC1CON = 0x6; //0110
        OC1RS = 0;
        OC2CON = 0x6;
        OC2RS = 0;

	LCDInitialize( );

	AD1PCFG &= 0xFFDF;	 	// AN5 input pin is analog ^:
	AD1CON2 = 0; 		// Configure A/D voltage reference
	AD1CON3 = 0x0101;
	AD1CON1 = 0x20E4;
	AD1CHS = 5; 		// Configure input channels
	AD1CSSL = 0; 		// No inputs is scanned

	IFS0bits.AD1IF = 0; // Clear A/D conversion interrupt.
	AD1CON1bits.ADON = 1; // Turn on A/D
        TRISBbits.TRISB5 = 1;
        CNEN2bits.CN27IE = 1;
        IFS1bits.CNIF = 0;
        IEC1bits.CNIE = 1;
	while(1){
        switch(state)
        {
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
            TRISBbits.TRISB8 = 0;
            TRISBbits.TRISB9 = 0;
            RPOR5bits.RP10R = 18; //OC1 (10 - foward, 8 - backward)
            RPOR5bits.RP11R = 19; //OC2 (11 - foward, 9 - backward)
            nextState = 2;
            break;

        case 2:
            TRISBbits.TRISB10 = 0;
            TRISBbits.TRISB11 = 0;
            TRISBbits.TRISB8 = 0;
            TRISBbits.TRISB9 = 0;
            RPOR4bits.RP8R = NULL;
            RPOR4bits.RP9R = NULL;
            RPOR5bits.RP10R = NULL;
            RPOR5bits.RP11R = NULL;
            nextState = 3;
            break;

        case 3:
            TRISBbits.TRISB10 = 0;
            TRISBbits.TRISB11 = 0;
            RPOR4bits.RP8R = 18; //OC1 (10 - foward, 8 - backward)
            RPOR4bits.RP9R = 19; //OC2 (11 - foward, 9 - backward)
            nextState = 0;
            break;
       }

            while (IFS0bits.AD1IF ==0){};     // AD1CON1bits.DONE can be checked instead
            IFS0bits.AD1IF = 0;

            ADC_value = ADC1BUF0 * 3.3 / 1023;   // 0 <= ADC <= 3.3
            sprintf(value, "%.3f", ADC_value );
            LCDMoveCursor(0,0); LCDPrintString(value);

            OC1RS = ADC1BUF0;
            OC2RS = 1023 - ADC1BUF0;

            if(OC1RS > 512)
            {
                OC1RS = 512;
            }
            else if(OC2RS > 512)
            {
                OC2RS = 512;
            }
            sprintf(Val1, "%3d", 100 * OC1RS / 512);
            LCDMoveCursor(1,0); LCDPrintString(Val1);
            sprintf(Val2, "%3d", 100 * OC2RS / 512);
            LCDMoveCursor(1,4); LCDPrintString(Val2);
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