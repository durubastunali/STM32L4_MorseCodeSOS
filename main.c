#include "stm32l476xx.h"

// PA.5  <--> Green LED
// PC.13 <--> Blue user button
#define LED_PIN    5
#define BUTTON_PIN 13

//Delay
volatile int32_t timeDelay;

// User HSI (high-speed internal) as the processor clock
void enable_HSI(){
	// Enable High Speed Internal Clock (HSI = 16 MHz)
  RCC->CR |= ((uint32_t)RCC_CR_HSION);
	
	
  // wait until HSI is ready
  while ( (RCC->CR & (uint32_t) RCC_CR_HSIRDY) == 0 ) {;}
	
  // Select HSI as system clock source 
  RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
  RCC->CFGR |= (uint32_t)RCC_CFGR_SW_HSI;  //01: HSI16 oscillator used as system clock

  // Wait till HSI is used as system clock source 
  while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) == 0 ) {;}
}

void configure_LED_pin(){
  // Enable the clock to GPIO Port A	
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; 
		
	// GPIO Mode: Input(00), Output(01), AlterFunc(10), Analog(11, reset)
	GPIOA->MODER &= ~(3UL<<(2*LED_PIN));  
	GPIOA->MODER |=   1UL<<(2*LED_PIN);      // Output(01)
	
	// GPIO Speed: Low speed (00), Medium speed (01), Fast speed (10), High speed (11)
	GPIOA->OSPEEDR &= ~(3<<(2*LED_PIN));
	GPIOA->OSPEEDR |=   2<<(2*LED_PIN);  // Fast speed
	
	// GPIO Output Type: Output push-pull (0, reset), Output open drain (1) 
	GPIOA->OTYPER &= ~(1<<LED_PIN);      // Push-pull
	
	// GPIO Push-Pull: No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
	GPIOA->PUPDR  &= ~(3<<(2*LED_PIN));  // No pull-up, no pull-down
}

void configure_BUTTON_pin() {
	// Enable the clock to GPIO Port C
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN; 
	
	// GPIO Mode: Input(00), Output(01), AlterFunc(10), Analog(11, reset)
	GPIOC->MODER &= ~(3UL<<(2*BUTTON_PIN));  //Input(00)
	
	// GPIO Speed: Low speed (00), Medium speed (01), Fast speed (10), High speed (11)
	GPIOC->OSPEEDR &= ~(3<<(2*BUTTON_PIN));
	GPIOC->OSPEEDR |=   2<<(2*BUTTON_PIN);  // Fast speed
	
	// GPIO Push-Pull: No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
	GPIOC->PUPDR  &= ~(3<<(2*BUTTON_PIN));  // No pull-up, no pull-down
}


void turn_on_LED(){
	GPIOA->ODR |= 1 << LED_PIN;
}

void turn_off_LED(){
	GPIOA->ODR &= ~(1 << LED_PIN);
}

void toggle_LED(){
	GPIOA->ODR ^= (1 << LED_PIN);
}

void configure_SysTick() {
	SysTick->CTRL = 0; 
	
	uint32_t quarterSecond = (250 * 16000) - 1;
	SysTick->LOAD = quarterSecond;
	
	SysTick->VAL = 0; // Reset the SysTick counter value
	// Select processor clock: 1 = processor clock; 0 = external clock
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
	// Enables SysTick interrupt, 1 = Enable, 0 = Disable
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	// Enable SysTick
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler (void) { // SysTick interrupt service routine
	if (timeDelay > 0) // Prevent it from being negative
		timeDelay--; // TimeDelay is a global volatile variable
}

void delay (uint32_t nTime) {
	timeDelay = nTime; // TimeDelay must be declared as volatile
	while(timeDelay != 0); // Busy wait
}

void DASH(){ //turn on led for 1/2 seconds and turn off to make dash in morse code
	turn_on_LED();
	delay(2);
	turn_off_LED();
	delay(1);
}

void DOT(){ // turn on led for 1/4 seconds and turn off to make dot in morse code
	turn_on_LED();
	delay(1);
	turn_off_LED();
	delay(1);
}

int main(void){
	enable_HSI();
	configure_LED_pin();
	configure_BUTTON_pin();
	configure_SysTick();
  // Dead loop & program hangs here
	while(1){
		uint32_t input = (GPIOC->IDR & GPIO_IDR_IDR_13);
		if (input == 0){
			DOT(); DOT(); DOT(); //make 3 dot morse code to get S
			DASH(); DASH(); DASH(); // make 3 dash morse code to get O
			DOT(); DOT(); DOT();
			while((GPIOC->IDR & GPIO_IDR_IDR_13)==0);
		}
	}
}