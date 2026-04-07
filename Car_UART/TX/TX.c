#include "stm32f10x.h"
#include <stdio.h>

// -------- DELAY --------
void delay_ms(int ms){
    int i,j;
    for(i=0;i<ms;i++)
        for(j=0;j<8000;j++);
}

// -------- UART INIT --------
void UART_Init(){
    RCC->APB2ENR |= (1<<2) | (1<<14);

    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |=  (0xB << 4); // TX

    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |=  (0x4 << 8); // RX

    USART1->BRR = 0x1D4C;
    USART1->CR1 = (1<<13) | (1<<3) | (1<<2);
}

void UART_SendChar(char c){
    while(!(USART1->SR & (1<<7)));
    USART1->DR = c;
}

void UART_SendString(char *s){
    while(*s) UART_SendChar(*s++);
}

// -------- ADC --------
void ADC_Init(){
    int i;
    RCC->APB2ENR |= (1<<2) | (1<<9);

    GPIOA->CRL &= ~(0xF << 20);

    ADC1->SQR3 = 5;
    ADC1->CR2 |= 1;

    for(i=0;i<1000;i++);

    ADC1->CR2 |= (1<<2);
    while(ADC1->CR2 & (1<<2));
}

uint16_t ADC_Read(){
    ADC1->CR2 |= (1<<22);
    while(!(ADC1->SR & (1<<1)));
    return ADC1->DR;
}

// -------- GPIO --------
void GPIO_Init(){
    RCC->APB2ENR |= (1<<2);

    GPIOA->CRL &= ~(0xFFFF << 4);
    GPIOA->CRL |=  (0x8888 << 4);
    GPIOA->ODR |= (0x1E);
}

// -------- MAIN --------
int main(){

    uint8_t d1,d2,d3,d4;
    uint16_t adc;
    int fuel;
    int door;          // ? moved here
    char buf[30];

    GPIO_Init();
    ADC_Init();
    UART_Init();

    while(1){

        d1 = (GPIOA->IDR & (1<<1)) ? 1:0;
        d2 = (GPIOA->IDR & (1<<2)) ? 1:0;
        d3 = (GPIOA->IDR & (1<<3)) ? 1:0;
        d4 = (GPIOA->IDR & (1<<4)) ? 1:0;

        adc = ADC_Read();
        fuel = (adc * 100)/4095;

        door = 0;   // ? assign here (not declare)

        if(!d1) door = 1;
        else if(!d2) door = 2;
        else if(!d3) door = 3;
        else if(!d4) door = 4;

        sprintf(buf,"D%d F%d\n",door,fuel);
        UART_SendString(buf);

        delay_ms(200);
    }
}