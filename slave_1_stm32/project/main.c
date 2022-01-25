#include <stm32f4xx.h>



//-------------------------//definitions 
#define mask(x) (1U << x)
//-------------------------||


//-------------------------//constants 
const int BRR_value = 0x683 ; // baudrate register value
uint8_t synchronizer[3] = {0xFC,0xFC,0xFC};
//-------------------------||

//-------------------------//golbal variables 
volatile int selected = 0 ; 
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

//-------------------------//interrupt functions 
void EXTI0_IRQHandler(void){ // ISR to handle button click for PB0 
	EXTI->PR |= mask(0);
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	uint32_t m = __get_PRIMASK() ; // save primask
	__disable_irq();// disable preemption
	//
	if(selected){
		return ; 
	}
	if(GPIOB->IDR & 0x1){
		GPIOB->ODR ^= 0x10 ; 
	}
	//
	__set_PRIMASK(m);
}
void EXTI1_IRQHandler(void){ // ISR to handle button click for PB0 
	EXTI->PR |= mask(0);
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	uint32_t m = __get_PRIMASK() ; // save primask
	__disable_irq();// disable preemption
	//
	if(selected){
		return ; 
	}
	if(GPIOB->IDR & 0x2){
		GPIOB->ODR ^= 0x20 ;
	}
	//
	__set_PRIMASK(m);
}
void EXTI2_IRQHandler(void){ // ISR to handle button click for PB0 
	EXTI->PR |= mask(0);
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	uint32_t m = __get_PRIMASK() ; // save primask
	__disable_irq();// disable preemption
	//
	if(selected){
		return ; 
	}
	if(GPIOB->IDR & 0x4){
		GPIOB->ODR ^= 0x40 ;
	}
	//
	__set_PRIMASK(m);
}
void EXTI3_IRQHandler(void){ // ISR to handle button click for PB0 
	EXTI->PR |= mask(0);
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	write_to_master();
	uint32_t m = __get_PRIMASK() ; // save primask
	__disable_irq();// disable preemption
	//
	if(selected){
		return ; 
	}
	if(GPIOB->IDR & 0x8){
		GPIOB->ODR ^= 0x80 ;
	}
	//
	__set_PRIMASK(m);
}
//-------------------------||

int main(void){
	init_usart2();
	GPIOB_init();
	GPIOA_init();
	GPIOB->ODR = 0x30 ; 
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
		selected = 1 ; 
		d = read_usart2();
		if( d == 0x80){ // read 
			if(read_usart2() == 0x00){
				//GPIOA->ODR |= mask(9);
				write_to_master();
			}
		}
		else if(d == 0x0){ // write 
			d = read_usart2() ; 
			if(d == 0xFF){
				selected = 0 ; 
				return ; 
			}
			GPIOB->ODR = (d << 4 );
			//GPIOB->ODR = 0 ;
			return ; 
		}
		else {
			read_usart2();
		}
	}
	else{
		read_usart2();
		read_usart2();
		return ; 
	}
}
void GPIOB_init(void){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 
	GPIOB->MODER &= ~0x0000FFFFU ; 
	GPIOB->MODER |= 0x000005500U ; // set led ports to output mode 
	GPIOB->ODR &= 0x0F	;
	GPIOB->PUPDR &= ~0x000000FFU;
	GPIOB->PUPDR |= ~0x00000055U; // pull down pb0 to pb3 
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB ; // pin 0 
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PB ;	
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PB ; 
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PB ; 
	EXTI->IMR |= mask(0)|mask(1) | mask(2) | mask(3); 	
	EXTI->RTSR |= mask(0)|mask(1) | mask(2) | mask(3); 	
	__enable_irq();
	NVIC_SetPriority(EXTI0_IRQn,0);
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(EXTI0_IRQn);
	
	NVIC_SetPriority(EXTI1_IRQn,0);
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
	NVIC_EnableIRQ(EXTI1_IRQn);
	
	NVIC_SetPriority(EXTI2_IRQn,0);
	NVIC_ClearPendingIRQ(EXTI2_IRQn);
	NVIC_EnableIRQ(EXTI2_IRQn);
	
	NVIC_SetPriority(EXTI3_IRQn,0);
	NVIC_ClearPendingIRQ(EXTI3_IRQn);
	NVIC_EnableIRQ(EXTI3_IRQn);
	
	
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
	write_usart2((GPIOB->ODR & 0xF0) >> 4);
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
