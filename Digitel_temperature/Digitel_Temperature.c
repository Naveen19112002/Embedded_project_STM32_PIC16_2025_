#include "stm32f10x.h"

long temp = 0;

/* ---------------- Delay ---------------- */
void delay(volatile unsigned int t)
{
    while(t--);
}

/* ---------------- Enable Pulse ---------------- */
void enable()
{
    GPIOC->BSRR = (1<<14);
    delay(3000);
    GPIOC->BRR  = (1<<14);
    delay(3000);
}

/* ---------------- Send 4-bit Data ---------------- */
void lcd4bit(unsigned char data)
{
    GPIOB->BRR = 0x0F;
    GPIOB->BSRR = (data & 0x0F);
    enable();
}

/* ---------------- LCD Send ---------------- */
void lcd(char rs, char data)
{
    if(rs)
        GPIOC->BSRR = (1<<15);
    else
        GPIOC->BRR  = (1<<15);

    lcd4bit(data >> 4);
    lcd4bit(data);
}

/* ---------------- LCD String ---------------- */
void string(char *data)
{
    while(*data)
        lcd(1,*data++);
}

/* ---------------- ADC Read ---------------- */
int adc_read()
{
    ADC1->CR2 |= (1<<22);
    while(!(ADC1->SR & (1<<1)));
    return ADC1->DR;
}

/* ---------------- MAIN ---------------- */
int main()
{
    int voltage;
    int a,b,c;

    /* Enable Clocks */
    RCC->APB2ENR |= (1<<2);   // GPIOA
    RCC->APB2ENR |= (1<<3);   // GPIOB
    RCC->APB2ENR |= (1<<4);   // GPIOC
    RCC->APB2ENR |= (1<<9);   // ADC1

    /* PA0 -> Analog */
    GPIOA->CRL &= ~(0xF<<0);

    /* PB0-PB3 -> LCD */
    GPIOB->CRL &= ~(0xFFFF);
    GPIOB->CRL |=  0x2222;

    /* PC14, PC15 -> LCD Control */
    GPIOC->CRH &= ~(0xFFUL<<24);
    GPIOC->CRH |=  (0x22<<24);

    /* -------- ADC CONFIG IMPORTANT -------- */

    ADC1->SMPR2 |= (7<<0);     // Sample time for channel 0
    ADC1->SQR3 = 0;            // Channel 0 selected

    ADC1->CR2 |= (1<<0);       // ADON
    delay(10000);
    ADC1->CR2 |= (1<<3);       // Calibration
    while(ADC1->CR2 & (1<<3));

    /* -------- LCD INIT -------- */

    delay(20000);
    lcd4bit(0x03);
    delay(5000);
    lcd4bit(0x03);
    delay(5000);
    lcd4bit(0x03);
    lcd4bit(0x02);

    lcd(0,0x28);
    lcd(0,0x0C);
    lcd(0,0x06);
    lcd(0,0x01);

    while(1)
    {
        ADC1->CR2 |= (1<<22);
        while(!(ADC1->SR & (1<<1)));

        voltage = ADC1->DR;

        voltage = (voltage * 330) / 4095;

        a = voltage/100;
        b = (voltage/10)%10;
        c = voltage%10;

        lcd(0,0x80);
        lcd(1,a+48);
        lcd(1,b+48);
        lcd(1,'.');
        lcd(1,c+48);
        lcd(1,'V');

        delay(500000);
    }
}