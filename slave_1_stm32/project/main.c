#include <stm32f4xx.h>



//-------------------------//definitions 
#define mask(x) (1U << x)
//-------------------------||


//-------------------------//constants 
const int BRR_value = 0x683 ; // baudrate register value
uint8_t synchronizer[3] = {0xFC,0xFC,0xFC};
//-------------------------||


//-------------------------//function prototypes 
void init_usart2 (void);
void write_usart2(uint8_t ch);
void usart2_write_string(char* str);
void GPIOA_init(void);
void GPIOB_init(void);
void delayMS(int n);
uint8_t read_usart2(void);
void listen_for_master(void);
void write_to_master(void);
//-------------------------||


int main(void){
	init_usart2();
	GPIOB_init();
	GPIOA_init();
	GPIOB->ODR = 0x3 ; 
	while(1){ // main event loop 
		listen_for_master();
	}
}


void GPIOA_init(){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 
	GPIOA->MODER &= ~0x000C0000U ; // set pin 9 to output for bus control 
	GPIOA->MODER |= 0x00040000U ;
	GPIOA->ODR &= ~mask(9);
}

void listen_for_master(){
	GPIOA->ODR &= ~mask(9);
	uint8_t d = read_usart2();
	int synch_count = 3 ; 
	while(synch_count>0){
		if(d==synchronizer[0]){
			synch_count-- ; 
		}
		d = read_usart2();
	}
	if(d == 0x1){
		d = read_usart2();
		if( d == 0x80){ // read 
			if(read_usart2() == 0x00){
				//GPIOA->ODR |= mask(9);
				write_to_master();
			}
		}
		else if(d == 0x0){ // write 
			d = read_usart2() ; 
			GPIOB->ODR = d ;
			//GPIOB->ODR = 0 ;
			return ; 
		}
	}
	else{
		return ; 
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

	USART2->CR1 |= USART_CR1_UE;
	// set baud rate as 9600 for 16Mhz pclk value (defined above)
	USART2->BRR = BRR_value;

	// enable receive for rx
	USART2->CR1 = USART_CR1_RE | USART_CR1_TE ; 
	// usart enable
	USART2->CR1 |= USART_CR1_UE;

}



void write_to_master(){
	GPIOA->ODR |= mask(9);
	for(int i = 0 ; i < 3 ; i++){
		write_usart2(synchronizer[i]);
	}
	write_usart2(10);
	write_usart2(12);//write 
	write_usart2(GPIOB->ODR & 0x0F );
	GPIOA->ODR &= ~mask(9);
}
/* Write a character to USART2 */
void write_usart2(uint8_t ch){
	USART2->DR = ch ;
	while(!(USART2->SR & USART_SR_TC));
}


void usart2_write_string(char* str){
	while(*str){ // iterate until '\0' is seen 
		write_usart2(*str);
		str++ ;
	}
	//GPIOC->ODR ^= 1 ;
}

uint8_t read_usart2(void)
{
	GPIOA->ODR &= ~mask(9);
	while (!(USART2->SR & USART_SR_RXNE))
	{
	} // wait until char arrives 
	return USART2->DR & 0xFF ;
}



void delayMS(int n){ // inaccurate delay in milliseconds 
	for(volatile int i = 0 ; i < 25000 * n ; i ++ ){
	}
}
