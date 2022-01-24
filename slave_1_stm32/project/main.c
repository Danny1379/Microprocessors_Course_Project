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
unsigned int read_usart2(void);
//-------------------------||


int main(void){
	init_usart2();
	GPIOB_init();
	//usart2_write_string("hello bitches");
	while(1){ // main event loop 
		char d = (char) read_usart2();
		if(d == '1'){
			GPIOB->ODR ^= 7 ;
		}
	}
}

void GPIOB_init(void){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 
	GPIOB->MODER &= ~0x000000FFU ; 
	GPIOB->MODER |= 0x000000055U ; // set led ports to output mode 
	GPIOB->ODR &= 0 ; 
}

void init_usart2 (void) {
	// clock enable
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	GPIOA->OSPEEDR |= 0x40; // don't fix what's not broken
	GPIOA->OSPEEDR |= 0x80;
	// set mode to alt func
	GPIOA->MODER &= ~0x00F0U;
	GPIOA->MODER |= 0x00A0;
	// set af to usart rx : pin 3  and tx : pin 2(not used)
	GPIOA->AFR[0] &= ~0x0FF00U;
	GPIOA->AFR[0] |= 0x07700;

	// set baud rate as 9600 for 16Mhz pclk value (defined above)
	USART2->BRR = BRR_value;

	// enable receive for rx
	USART2->CR1 = USART_CR1_RE;

	// usart enable
	USART2->CR1 |= USART_CR1_UE;

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

unsigned int read_usart2(void)
{
	while (!(USART2->SR & USART_SR_RXNE))
	{
	} // wait until char arrives 
	return USART2->DR;
}
void delayMS(int n){ // inaccurate delay in milliseconds 
	for(volatile int i = 0 ; i < 25000 * n ; i ++ ){
	}
}