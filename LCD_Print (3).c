
/*
 * File:   Project_EEE435.c
 * Author: Abdullah Al Rafi
 * ID: 2020-1-80-068
 * EEE435
 *
 * Created on August 15, 2023, 1:48 AM
 * 
 * 
 * 
 */

// PIC16F690 Configuration Bit Settings

// 'C' source line config statements

// CONFIG
#pragma config FOSC = INTRCIO 
#pragma config WDTE = OFF       
#pragma config PWRTE = OFF      
#pragma config MCLRE = OFF      
#pragma config CP = OFF         
#pragma config CPD = OFF        
#pragma config BOREN = OFF      
#pragma config IESO = OFF       
#pragma config FCMEN = OFF   

//#define _XTAL_FREQ 8000000   // OScillator Frequency 8MHz

#include <xc.h>
#define RS PORTBbits.RB4
#define RW PORTBbits.RB5
#define EN PORTAbits.RA0
#define TR PORTAbits.RA5
#define LCD_PORT PORTC
#define GODONE ADCON0bits.GO_DONE


void lcd_cmd(unsigned char cmd)
{
    LCD_PORT = cmd;
    RS = 0;
    RW = 0;
    EN = 1;
    _delay(10000);
    EN = 0;
    _delay(10000);
}

void lcd_clear()
{
    LCD_PORT = 0x01;
    RS = 0;
    RW = 0;
    EN = 1;
    _delay(1000);
    EN = 0;
    _delay(1000);
}

void lcd_init(void)
{
    lcd_clear();
    lcd_cmd(0x38);
    lcd_cmd(0x0E);
    lcd_cmd(0x06);
}

void lcd_shift_left(void)
{
    lcd_cmd(0x07);
    lcd_cmd(0x18);
}

void lcd_shift_right(void)
{
    lcd_cmd(0x07);
    lcd_cmd(0x1C);
}

void lcd_stop_disp_shift(void)
{
    lcd_cmd(0x06);
    lcd_cmd(0x14);
}

void lcd_data(char dat)
{
    LCD_PORT = dat;
    RS = 1;
    RW = 0;
    EN = 1;
    _delay(10000);
    EN = 0;
    _delay(10000);
}

void lcd_print_str(unsigned char *str)
{
    int cnt = 0;
    while(*str)
    {
        if (cnt == 15)
        {
            //lcd_shift_right();
                lcd_cmd(0x07);
                lcd_cmd(0x1C);
        }
       
        lcd_data(*str++);
        cnt++;
    }
   lcd_stop_disp_shift();
} 
void print_adc_value(void)
{
    GODONE = 1;
    while (GODONE == 1);
    {
        int adc_res = (ADRESH<<8) + ADRESL; // Left Justified
        
        float adc_res_volt = adc_res * (5.0-0)/1023;

        int a = adc_res_volt * 100; // Converting float into integer
        int d1 = (a/100)%10;
        int d2 = (a/10) % 10; // Second to the last digit
        int d3 = (a/1) % 10; // Last digit

       
        lcd_cmd(0x01);
        lcd_cmd(0x02);
        lcd_cmd(0x06);
       //lcd_cmd(0xC0);
        
        lcd_data(d1 + '0');
        lcd_data('.'); //decimal point
        lcd_data(d2 + '0');
        lcd_data(d3 + '0');
        
           //delay(400000);
    }
    }
 
//Global Variables
float tare,weight,weight_m; //Tare value, Weight measured, real weight excluding Tare
float Factor = 1.0; // Weight vs voltage Factor(1V for 1Kg)
int SFlag,TFlag; //Sign flag and Tare Flag


void __interrupt() external_isr(void) 
{
    if (INTF) 
    {
        TR = 1;
        TFlag = 1;
        INTCONbits.INTF = 0;
    }
}



void main(void)
{
    TRISAbits.TRISA1 = 1;
    ANSELbits.ANS1 = 1;
    TRISAbits.TRISA0 = 0;
    TRISAbits.TRISA5 = 0;
    TRISC = 0x00;
    TRISB = 0x00;
   
    ANSEL = 0x02;
    ANSELH = 0x00;
    ADCON0 = 0x85;
    ADCON1 = 0x00;
   
    // Configure the RA2/INT pin as an input
    TRISAbits.TRISA2 = 1;
    
    OPTION_REG &= 0b10111111; // INTEDG = 0 (falling edge)
    OPTION_REGbits.INTEDG = 1;
    INTCONbits.INTF = 0; // Clear the interrupt flag
    INTCONbits.INTE = 1; // Enable the external interrupt

    // Enable global and peripheral interrupts
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    while(1){
    lcd_init();
    _delay(10000);
    GODONE = 1;
    while(GODONE == 1);
    {
        int adc_res = (ADRESH<<8) + ADRESL; // Left Justified
        weight = Factor * adc_res * (5.0-0)/1023;
        
        if(TFlag == 1)
        {   //Taring weight
            tare = weight;
            weight_m = tare;
            lcd_print_str("Tare: ");
            TR = 0;
            TFlag = 0;
        }
        else
        {
            //Measuring Weight
            if(tare<=weight)
            {
            weight_m = weight - tare;
            SFlag = 0;
            }
            else
            {
            weight_m = tare - weight;
            SFlag = 1;
            }
         
         lcd_print_str("Weight:");
        }
        _delay(5000);
        
        int a = weight_m * 100; // Converting float into integer
        int d1 = (a/100)%10;
        int d2 = (a/10) % 10; // Second to the last digit
        int d3 = (a/1) % 10; // Last digit

        if(SFlag == 1){
            lcd_print_str("-");
        }
        lcd_data(d1 + '0');
        lcd_data('.'); //decimal point
        lcd_data(d2 + '0');
        lcd_data(d3 + '0');
        lcd_print_str("Kg");
        
        _delay(5000);
    }
    }      
 return;
}
/*
// For C90 Standards
void interrupt external() 
{
    if (INTF) 
    {
        TR = ~TR;
        Flag = 1;
        INTCONbits.INTF = 0;
    }
}*/