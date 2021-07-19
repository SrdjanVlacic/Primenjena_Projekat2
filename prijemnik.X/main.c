
#include <stdio.h>
#include <stdlib.h>
#include<p30fxxxx.h>

#include "timer.h"

_FOSC(CSW_FSCM_OFF & XT_PLL4);//instruction takt je isti kao i kristal
_FWDT(WDT_OFF);

#define PWM_MAX 2499
#define PWM_NORMAL 1500
#define TMR2_period 10000
#define TMR1_period 10000
#define TMR3_period 10000/*  Fosc = 10MHz,
					          1/Fosc = 0.1us !!!, 0.1us * 10000 = 1ms  */
#define EN1 OC1RS
#define EN2 OC2RS
unsigned char uneseno;
unsigned int tempRX, brojac_ms1, brojac_ms2, brojac_ms3, kombo=0, turbo_flag=PWM_NORMAL;
unsigned char prethodni ='N';
unsigned int flag = 0,unos_flag=0;

void initUART1(void)
{
U1BRG=0x0207;//ovim odredjujemo baudrate 1200
				
U1MODEbits.ALTIO=0;//biramo koje pinove koristimo za komunikaciju osnovne ili alternativne

IEC0bits.U1RXIE=1;//omogucavamo rx1 interupt

U1STA&=0xfffc;

U1MODEbits.UARTEN=1;//ukljucujemo ovaj modul

U1STAbits.UTXEN=1;//ukljucujemo predaju
}

void __attribute__((__interrupt__)) _U1RXInterrupt(void) 
{
    IFS0bits.U1RXIF = 0;
    prethodni=tempRX;
    tempRX=U1RXREG;
    
    
    
    if(flag){
        uneseno=tempRX;
        unos_flag=1;
        flag=0;
    }
    else if(prethodni=='0' && tempRX=='0')
    {
        kombo++;
        if(kombo > 1)
        {
            flag=1;
            kombo=0;
            prethodni='m';
        }
    } 
    
    //LATDbits.LATD0 = 1;
} 

void WriteUART1(unsigned int data)
{
	  while(!U1STAbits.TRMT);

    if(U1MODEbits.PDSEL == 3)
        U1TXREG = data;
    else
        U1TXREG = data & 0xFF;
}
void UART1_string(register const char *str)
{
    while((*str) != 0)
    {
        WriteUART1(*str);
        //if(str == 13) WriteUART1(10);
        //if(*str == 10) WriteUART1(13);
        str++;
    }
    WriteUART1(13); //prelazak u novi red nakon poslatog stringa
}

void __attribute__((__interrupt__)) _T2Interrupt(void)
{

   	TMR2 =0;
    IFS0bits.T2IF = 0;
}

void PWM_CFG(void)
{
    PR2=2499;
    OC1RS= 20;
    OC1R=1000;
    OC1CONbits.OCM = 0b110;
    T2CONbits.TCKPS = 0b11;
    T2CONbits.TON=1;
    OC1RS = PWM_NORMAL;
    
    OC2RS= 20;
    OC2R=1000;
    OC2CONbits.OCM = 0b110;
    OC2RS = PWM_NORMAL;
}

void check_direction(char smer){
    
}

int main(int argc, char** argv) {
    initUART1();
    PWM_CFG();
    
    //TRISDbits.TRISD0=0; //EN_1/PWM
    //TRISDbits.TRISD1=0; //EN_2

    ADPCFGbits.PCFG11=1;
    TRISBbits.TRISB11=0; //in1
    ADPCFGbits.PCFG12=1;
    TRISBbits.TRISB12=0; //in2
    TRISFbits.TRISF1=0; //in3
    TRISFbits.TRISF0=0; //in4
    
    ADPCFGbits.PCFG0=1;
    TRISBbits.TRISB11=0; //dioda
    ADPCFGbits.PCFG1=1;
    TRISBbits.TRISB11=0; //dioda
/*  
    LATBbits.LATB11=1;//in1
    LATBbits.LATB12=0;//in2
    LATFbits.LATF1=1;//in3
    LATFbits.LATF0=0;//in4
    LATDbits.LATD1=0;//en2
    LATDbits.LATD0=0;//en1
  */     
 
    
   unsigned int r=PWM_NORMAL;
    while(3){
        
        //UART1_string("OK");
        
            if(unos_flag==1){
                unos_flag=0;
                switch(uneseno){
                    case 'N': //levo
                        r=PWM_NORMAL;
                       LATBbits.LATB11=0;//in1
                       LATBbits.LATB12=1;//in2  
                        
                    
                        EN1 = PWM_NORMAL;
                        EN2 = 0;\

                        break;
                    case 'B': //desno
                        r=PWM_NORMAL;
                        LATBbits.LATB11=1; //in1
                        LATBbits.LATB12=0;//in2
                        
                        EN1 = PWM_NORMAL;
                        EN2 = 0;
                        break;
                    case 'D'://napred-desno
                        r=PWM_NORMAL;
                        LATBbits.LATB11=0;//in1
                        LATBbits.LATB12=1;//in2
                        LATFbits.LATF1=1; //in3
                        LATFbits.LATF0=0; //in4
                        EN1 = PWM_NORMAL;
                        EN2 = PWM_MAX;
                        break;
                    case 'L'://napred-levo
                        r=PWM_NORMAL;
                        LATBbits.LATB11=0;//in1
                        LATBbits.LATB12=1;//in2
                        LATFbits.LATF1=0;//in3
                        LATFbits.LATF0=1;//in4
                        EN1 = PWM_NORMAL;
                        EN2 = PWM_MAX;
                        break;
                    case 'C': //nazad-desno
                        r=PWM_NORMAL;
                        LATBbits.LATB11=1;//in1
                        LATBbits.LATB12=0;//in2
                        LATFbits.LATF1=1;//in3
                        LATFbits.LATF0=0;//in4
                        EN1 = PWM_NORMAL;
                        EN2 = PWM_MAX;
                        break;
                    case 'K': //nazad-levo
                        r=PWM_NORMAL;
                        LATBbits.LATB11=1;//in1
                        LATBbits.LATB12=0;//in2
                        LATFbits.LATF1=0;//in3
                        LATFbits.LATF0=1;//in4
                        EN1 = PWM_NORMAL;
                        EN2 = PWM_MAX;
                        break;
                    case 'Q': //napred
                       LATFbits.LATF1=0;//in3
                        LATFbits.LATF0=1;//in4
                        
                        EN1 = PWM_NORMAL;
                        EN2 = PWM_MAX;
                        break;
                    case 'W': //nazad
                        LATFbits.LATF1=1;//in3
                        LATFbits.LATF0=0;//in4
                        
                        EN1 = PWM_NORMAL;
                        EN2 = PWM_MAX;
                        break;
                    case 'T': //turbo
                        r = PWM_MAX;
                       // LATBbits.LATB11=0;//in1
                       // LATBbits.LATB12=1;//in2
                        LATFbits.LATF1=0;//in3
                        LATFbits.LATF0=1;//in4
                        EN1 = 0;
                        EN2 = PWM_MAX;
                        break;
                        
                    case 'X':
                        LATBbits.LATB0=1;
                        LATBbits.LATB1=1;
                        
                    case 'S':
                        LATBbits.LATB11=0;//in1
                        LATBbits.LATB12=0;//in2
                        LATFbits.LATF1=0;//in3
                        LATFbits.LATF0=0;//in4
                        EN1 = 0;
                        EN2 = 0;
                        break;
                    
                }
                
            }
            else
                if(tempRX !='W' && tempRX !='K' && tempRX !='Q' && tempRX !='L' && tempRX !='D' && tempRX !='C' && tempRX!='T' && tempRX!='B' && tempRX!='N' && tempRX!='X')
                {
                    //LATBbits.LATB11=0;//in1
                    //LATBbits.LATB12=0;//in2
                    LATBbits.LATB11=0;//in1
                    LATBbits.LATB12=0;//in2
                    LATFbits.LATF1=0;//in3
                    LATFbits.LATF0=0;//in4
                    //EN1 = 0;
                    //EN2 = 0;
                }
    }    
    
    return (EXIT_SUCCESS);
}

