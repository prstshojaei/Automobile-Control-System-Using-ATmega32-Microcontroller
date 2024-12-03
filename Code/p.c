#include <mega32a.h>
#include <delay.h>
#include <alcd.h>
#include <math.h>
#include <stdio.h>
#define ADC_VREF_TYPE ((0<<REFS1) | (0<<REFS0) | (0<<ADLAR))

int benz , ms , sec , minute , hour = 12 , oc1 =10 , oc2 = 10 ;
unsigned char c[20];
float temprature;
bit tormoz_dasti = 0 ; 

interrupt [EXT_INT0] void ext_int0_isr(void)
{
   if(tormoz_dasti ==0)
   {
    OCR1AL = 0 ;
    OCR1BL = 0 ;
    tormoz_dasti = 1 ; // tormoz dasti keshide shod
   }                                               
   else
   {
    tormoz_dasti = 0 ; 
   }
}


unsigned int read_adc(unsigned char adc_input)
{
ADMUX=adc_input | ADC_VREF_TYPE;
delay_us(10);
ADCSRA|=(1<<ADSC);
while ((ADCSRA & (1<<ADIF))==0);
ADCSRA|=(1<<ADIF);
return ADCW;
}

void benzin()
{
    benz = read_adc(1);
    benz /= 127 ; 
    benz++ ;
              
    if(benz <=2) PORTB.4 = 1 ;// LED YELLOW
    else PORTB.4 = 0 ;
    PORTC = pow(2,benz) - 1;
}
void temp()
{
    temprature = read_adc(0);
    temprature = (temprature * 5.0) / 1024.0;
    temprature *= 100 ;
    
    lcd_gotoxy(0,1);
    sprintf(c,"Temp = %5.2f'C",temprature);
    lcd_puts(c);
    if(temprature > 70 && temprature < 90)
    {
        PORTB.3 = 1; // RED LED
        PORTB.2 = 1; // BUZZER
    }               
    else if(temprature > 90)
    {
        PORTB.3 = 1 ; // RED LED
        PORTB.2 = 0 ; // BUZZER 
        OCR1AL = 0X00 ;
        OCR1BL = 0X00 ;
    }                         
    else
    {
        PORTB.3 = 0 ; // RED LED
        PORTB.2 = 1 ; // BUZZER
    }
}

void back()
{
    PORTD.0 = 0 ;
    PORTD.1 = 1 ;
}
void front()
{
    PORTD.0 = 1 ;
    PORTD.1 = 0 ;
}
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
TCNT0=0x83;
ms++ ;
if(ms > 1000) 
{
    ms = 0 ; 
    sec++  ;   
    if(sec>59)
    {
        sec = 0 ;
        minute++ ;
        if(minute > 59)
        {
            minute = 0 ; 
            hour++ ;
            if(hour >23)
            {
                hour = 0 ;
            }
        }
    }                     
    lcd_gotoxy(0,0);
    sprintf(c,"time : %02d:%02d:%02d",hour , minute , sec );
    lcd_puts(c);
                 
          benzin();
          temp();    
          
    if(PINA.5 == 1)
    {
     lcd_gotoxy(0,3);
    lcd_puts("TURNING LEFT ");
    }     
    else if(PINA.3 == 1)
    {
     lcd_gotoxy(0,3);
    lcd_puts("TURNING RIGHT");
    }        
    else if(PINA.4 == 1)
    {             
        if(PINA.2 == 1)
        {
         lcd_gotoxy(0,3);
        lcd_puts("  GO FRONT   ");
        }                  
        else
        {
        
         lcd_gotoxy(0,3);
        lcd_puts("  GO BACK    ");
        }
    }        

              
}
}



void main(void)
{
DDRD = 0XFF ;
DDRC = 0XFF ;
DDRB = 0XFC ;
DDRD.2 = 0;// INPUT
PORTD.2 =1 ;//PULL UP 


// External Interrupt(s) initialization
// INT0: On
// INT0 Mode: Any change
// INT1: Off
// INT2: Off
GICR|=(0<<INT1) | (1<<INT0) | (0<<INT2);
MCUCR=(0<<ISC11) | (0<<ISC10) | (0<<ISC01) | (1<<ISC00);
MCUCSR=(0<<ISC2);
GIFR=(0<<INTF1) | (1<<INTF0) | (0<<INTF2);


// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 125.000 kHz
// Mode: Normal top=0xFF
// OC0 output: Disconnected
// Timer Period: 1 ms
TCCR0=(0<<WGM00) | (0<<COM01) | (0<<COM00) | (0<<WGM01) | (0<<CS02) | (1<<CS01) | (1<<CS00);
TCNT0=0x83;
OCR0=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 125.000 kHz
// Mode: Ph. correct PWM top=0x00FF
// OC1A output: Non-Inverted PWM
// OC1B output: Non-Inverted PWM
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer Period: 4.08 ms
// Output Pulse(s):
// OC1A Period: 4.08 ms Width: 0.416 ms
// OC1B Period: 4.08 ms Width: 0.416 ms
// Timer1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=(1<<COM1A1) | (0<<COM1A0) | (1<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (1<<WGM10);
TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (1<<CS11) | (1<<CS10);
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x1A;
OCR1BH=0x00;
OCR1BL=0x1A;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=(0<<OCIE2) | (0<<TOIE2) | (0<<TICIE1) | (0<<OCIE1A) | (0<<OCIE1B) | (0<<TOIE1) | (0<<OCIE0) | (1<<TOIE0);

// ADC initialization
// ADC Clock frequency: 1000.000 kHz
// ADC Voltage Reference: AREF pin
// ADC Auto Trigger Source: ADC Stopped
ADMUX=ADC_VREF_TYPE;
ADCSRA=(1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (0<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
SFIOR=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);

lcd_init(20);

#asm("sei")

while (1)
      {
           
            if(PINA.3 == 1)   // TURN RIGHT
            {    
                oc1 = OCR1AL ;
                oc2 = OCR1BL ;   

                while(PINA.3 == 1)
                {
                
                OCR1AL = 0x00 ;
                OCR1BL = oc2 ;  
                }
            }              
            else if(PINA.4 == 1) // GO STRAIGHT
            {       
                OCR1AL = oc1 ;
                OCR1BL = oc2 ;
                   while(PINA.4 == 1)
                   { 
                        if(PINA.2 == 0)
                        {                 
                            back();
                        }           
                        else
                        {          

                           front();  
                        } 
                        if(PINB.0 == 0) //  acc
                        {   
                            if(tormoz_dasti == 0) // tormoz daste qeyre faal 
                            {
                                OCR1AL++ ;
                                OCR1BL++ ;
                                delay_ms(10);
                            }
                        }            
                        if(PINB.1 == 0) // break
                        {  
                            if(tormoz_dasti == 0)
                            {
                                OCR1AL-- ;
                                OCR1BL-- ;
                                delay_ms(10);
                            }
                        }
                    }
            }              
            else if(PINA.5 == 1) // TURN LEFT
            {                      
                            
                                        
            
                oc1 = OCR1AL ;
                oc2 = OCR1BL ;
                while(PINA.5 == 1)
                {
                
                OCR1AL=oc1;
                OCR1BL=0x00;
                }
            }
            
      }
}
