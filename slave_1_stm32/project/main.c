#include <stm32f4xx.h>



//-------------------------//definitions 
#define mask(x) (1U << x)
//-------------------------||


//-------------------------//constants 
const int BRR_value = 0x683 ; // baudrate register value
//-------------------------||


//-------------------------//function prototypes 
void init_usart2 (void);
void write_usart2(uint32_t ch);
void usart2_write_string(char* str);
void GPIOB_init(void);
void delayMS(int n);
//-------------------------||


int main(void){
	init_usart2();
	GPIOB_init();
	GPIOB->ODR = 0 ; 
	GPIOB->ODR = 1 ; 
	//usart2_write_string("hello bitches");
	GPIOA->MODER |= 0x00040000;
	GPIOA->ODR |= 0x00000200 ;
	while(1){ // main event loop 
		GPIOB->ODR ^= 1 ; 
		write_usart2('i');
		delayMS(10);
	}
}

void GPIOB_init(void){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 
	GPIOB->MODER &= ~0x0000000FU ; 
	GPIOB->MODER |= 0x000000001 ;
}

void init_usart2 (void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; 	/* enable GPIOA clock */
	RCC->APB1ENR |= 0x20000; 							/* enable USART2 clock */
	
	/* Configure PA2 for USART2_TX */
	GPIOA->OSPEEDR |= 0x40;
	GPIOA->OSPEEDR |= 0x80;
	GPIOA->AFR[0] &= ~0x0FF00U;
	GPIOA->AFR[0] |= 0x07700; /* alt7 for USART2 */
	GPIOA->MODER  &= ~0x00F0U;
	GPIOA->MODER  |= 0x00A0; /* enable alternate function for PA2,PA3*/
	
	
	USART2->BRR = BRR_value  ;/* 9600 baud @ 16 MHz */
	
	
	USART2->CR1 = USART_CR1_TE; /* enable Tx, 8-bit data */
	//USART2->CR1 |= USART_CR1_TCIE;
	//USART2->CR2 = 0x0000; /* 1 stop bit */
	//USART2->CR3 = 0x0000; /* no flow control */
	USART2->CR1 |= USART_CR1_UE; /* enable USART2 */

}


/* Write a character to USART2 */
void write_usart2(uint32_t ch){
  USART2->DR = ch ; 
	while(!(USART2->SR & USART_SR_TC));
	//GPIOC->ODR |=  1 ;
}


void usart2_write_string(char* str){
	while(*str){ // iterate until '\0' is seen 
		write_usart2(*str);
		str++ ;
	}
	//GPIOC->ODR ^= 1 ;
}


void delayMS(int n){ // inaccurate delay in milliseconds 
	for(volatile int i = 0 ; i < 25000 * n ; i ++ ){
	}
}