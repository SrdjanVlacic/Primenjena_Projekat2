#include <stdio.h>
#include <stdlib.h>
#include<p30fxxxx.h>

_FOSC(CSW_FSCM_OFF & XT_PLL4);
_FWDT(WDT_OFF);

#define TMR2_period 10000

unsigned char tempRX;

void Init_T2(void)
{
	TMR2 = 0;
	PR2 = TMR2_period;
	
	T2CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
	//IPC1bits.T2IP = 3 // T2 interrupt pririty (0-7)
	//SRbits.IPL = 3; // CPU interrupt priority is 3(11)
	IFS0bits.T2IF = 0; // clear interrupt flag
	IEC0bits.T2IE = 1; // enable interrupt

	T2CONbits.TON = 1; // T2 on 
}

void __attribute__ ((__interrupt__)) _T2Interrupt(void) // svakih 1ms
{
	TMR2 =0;
	IFS0bits.T2IF = 0;       
}

void initUART1(void)
{
    U1BRG=0x0208;//ovim odredjujemo baudrate 1200

    U1MODEbits.ALTIO=0;//biramo koje pinove koristimo za komunikaciju osnovne ili alternativne

    IEC0bits.U1RXIE=1;//omogucavamo rx1 interupt

    U1STA&=0xfffc;

    U1MODEbits.UARTEN=1;//ukljucujemo ovaj modul

    U1STAbits.UTXEN=1;//ukljucujemo predaju
}

void __attribute__((__interrupt__)) _U1RXInterrupt(void) 
{
    IFS0bits.U1RXIF = 0;
    tempRX=U1RXREG;
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
        str++;
    }
}

void pin_config(){
    //RD2,RD9,RD1,RD0,RB11,RB10
    ADPCFGbits.PCFG10=1;//digitalni
    ADPCFGbits.PCFG11=1;//digitalni
	TRISBbits.TRISB10=1;//ulaz(napred)
    TRISBbits.TRISB11=1;//ulaz(nazad)
	TRISDbits.TRISD0=1;//ulaz(levo)
	TRISDbits.TRISD1=1;//ulaz(desno)
    TRISDbits.TRISD2=1;//ulaz(dioda)
    TRISDbits.TRISD9=1;//ulaz(turbo)
  
}

int main(int argc, char** argv){
    pin_config();
    initUART1();
    Init_T2();
    
     
    
    while(1){
    if(PORTBbits.RB10){//napred
        LATDbits.LATD3=1; //upali diodu
        
        if(PORTDbits.RD0){//napred-levo
            UART1_string("000L");
        }
        else if(PORTDbits.RD1){//napred-desno
            UART1_string("000D");
        }
        else{
            UART1_string("000N");
        }
    }
        
    else if(PORTBbits.RB11){//nazad
        LATDbits.LATD3=1; //upali diodu
        
        if(PORTDbits.RD0){//nazad-levo
            UART1_string("000K");
        }
        else if(PORTDbits.RD1){//nazad-desno
            UART1_string("000C");
        }
        else{
            UART1_string("000B");
        }
    }
    
    else if(PORTDbits.RD0){//levo
        LATDbits.LATD3=1; //upali diodu
        
        UART1_string("000Q");
    }
    else if(PORTDbits.RD1){//desno
        LATDbits.LATD3=1; //upali diodu
        
        UART1_string("000W");
    }
    
    else if(PORTDbits.RD9){//turbo
        LATDbits.LATD3=1; //upali diodu
         
        UART1_string("000T");
        }
    
    else if(PORTDbits.RD2){
        LATDbits.LATD3=1;
        UART1_string("000X");
    }
    

    LATDbits.LATD3=0;
    }
    return (EXIT_SUCCESS);
}