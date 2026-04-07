#include "stm32f10x.h"
#include <stdio.h>

// -------- LCD --------
#define RS (1<<0)
#define EN (1<<1)

void delay_ms(int ms){
    int i,j;
    for(i=0;i<ms;i++)
        for(j=0;j<8000;j++);
}

void lcd_pulse(){
    GPIOB->ODR |= EN;
    delay_ms(1);
    GPIOB->ODR &= ~EN;
}

void lcd_cmd(unsigned char cmd){
    GPIOB->ODR &= ~RS;

    GPIOB->ODR = (GPIOB->ODR & 0xFFC3)|((cmd>>4)<<2);
    lcd_pulse();

    GPIOB->ODR = (GPIOB->ODR & 0xFFC3)|((cmd&0x0F)<<2);
    lcd_pulse();
}

void lcd_data(unsigned char data){
    GPIOB->ODR |= RS;

    GPIOB->ODR = (GPIOB->ODR & 0xFFC3)|((data>>4)<<2);
    lcd_pulse();

    GPIOB->ODR = (GPIOB->ODR & 0xFFC3)|((data&0x0F)<<2);
    lcd_pulse();
}

void lcd_print(char *s){
    while(*s) lcd_data(*s++);
}

void lcd_init(){
    delay_ms(20);
    lcd_cmd(0x02);
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);
}

// -------- UART --------
void UART_Init(){
    RCC->APB2ENR |= (1<<2) | (1<<14);

    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |=  (0x4 << 8); // RX

    USART1->BRR = 0x1D4C;
    USART1->CR1 = (1<<13) | (1<<2);
}

char UART_ReadChar(){
    while(!(USART1->SR & (1<<5)));
    return USART1->DR;
}

// -------- MAIN --------
int main(){

    char c;
    char buffer[30];
    int i = 0;
    int door = 0;
    int fuel = 0;
    char msg[16];     // ? moved here
    char fmsg[16];    // ? moved here

    RCC->APB2ENR |= (1<<3);

    GPIOB->CRL &= ~(0xFFFFFF);
    GPIOB->CRL |=  (0x333333);

    UART_Init();
    lcd_init();

    while(1){

        c = UART_ReadChar();

        if(c == '\n'){
            buffer[i] = '\0';
            i = 0;

            sscanf(buffer,"D%d F%d",&door,&fuel);

            lcd_cmd(0x01);

            if(door == 0){
                lcd_print("All Closed");
            } else {
                sprintf(msg,"Door Open:%d",door);
                lcd_print(msg);
            }

            lcd_cmd(0xC0);
            sprintf(fmsg,"Fuel:%d%%",fuel);
            lcd_print(fmsg);
        }
        else{
            buffer[i++] = c;
        }
    }
}