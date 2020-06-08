 //----------------------------------
// Lab 2 - Timer Interrupts - Lab02.c
//----------------------------------
// Objective:
//   Build a small game that records user's reaction time.
//

//
//
// -- Imports ---------------
//
#include "init.h"

//
//

// -- Prototypes ------------
//
void blinkScreen();
void Init_GPIO();
void Init_Timer();

//
//
// -- Code Body -------------
//
volatile uint8_t timeUpdated = 0;
volatile uint8_t button1Pressed = 0;
volatile uint8_t buttonReleased = 0;
volatile uint32_t elapsed = 0;


int32_t randomNumber = 0;
uint32_t startTime = 0;
float averageScore = 0;
unsigned int iterations = 0;

int main() {
	Sys_Init();
	Init_Timer();
	Init_GPIO();

	while (1) {
		// Main loop code goes here
		if (button1Pressed == 1){
			printf("It worked \r\n");
			//printf("Time passed: %lu\r\n", elapsed);
			button1Pressed = 0;
		}



		/*
		printf("\033c\033[36m\033[2J");
		printf("Blink!\r\n");
		HAL_Delay(1000);
		blinkScreen(); // Alternatively: in some terminals, the BELL can be configured to produce
					   // 			a "visual bell" ... a blink.
		HAL_Delay(1000);*/
	}
}

//
//
// -- Utility Functions ------
//
void blinkScreen(){
	printf("\033[30;47m");
	// Clear and redraw display (flash it & sound the bell).
	printf("\a\033[s\033[2J\033[u");
	fflush(stdout);
	HAL_Delay(100);

	printf("\033[37;40m");
	// Clear and redraw display (flash it).
	printf("\033[s\033[2J\033[u");
	fflush(stdout);
}


//
//
// -- Init Functions ----------
//
void Init_Timer() {
	// Enable the TIM6 interrupt.
	// Looks like HAL hid this little gem, this register isn't mentioned in
	//   the STM32F7 ARM Reference Manual....
	NVIC->ISER[54 / 32] =(uint32_t) 1 << (54 % 32);

	// Enable TIM6 clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
	asm ( "nop" );
	asm ( "nop" );

	// Set pre-scaler to slow down ticlks
	TIM6->PSC = 799;

	// Set the Auto-reload Value for 10Hz overflow
	TIM6->ARR = 13499;

	// Generate update events to auto reload.
	TIM6->EGR |= TIM_EGR_UG;

	// Enable Update Interrupts.
	TIM6->DIER |= TIM_DIER_UIE;

	// Start the timer.
	TIM6->CR1 |= TIM_CR1_CEN;
}

void Init_GPIO() {
	// Enable GPIO clocks?
	// Looks like GPIO reg updates are synced to a base clock.
	//  for any changes to appear the clocks need to be running.
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOJEN;
	// Enable clock to SYSCONFIG module to enable writing of EXTICRn registers
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	// or __HAL_RCC_SYSCFG_CLK_ENABLE();
	// Delay after an RCC peripheral clock enabling
	asm ("nop");
	asm ("nop");

	// Set Pin 13/5 to output. (LED1 and LED2)
	GPIOJ->MODER |= 67109888U; //Bitmask for GPIO J Pin 5 and 13 sets to output mode
	// GPIO Interrupt
	// By default pin PA0 will trigger the interrupt, change EXTICR1 to route proper pin
	SYSCFG->EXTICR[0] |= 0x09; // EXTICR1-4 are confusingly an array [0-3].

	// Set Pin J0 as input (button) with pull-down.
	GPIOJ->MODER &= ~3U;
	GPIOJ->PUPDR |= 1U; //fix this

	// Set interrupt enable for EXTI0.
	NVIC->ISER[6 / 32] =(uint32_t) 1 << (6 % 32);

	// Unmask interrupt.
	EXTI->IMR = EXTI_IMR_MR0;

	// Register for rising edge.
	EXTI->RTSR = EXTI_RTSR_TR0;

	// And register for the falling edge.
	//EXTI->FTSR = EXTI_FTSR_TR0;
}

//
//
// -- ISRs (IRQs) -------------
//
void TIM6_DAC_IRQHandler() {
	// Clear Interrupt Bit
	TIM6->SR &= ~0x01;
	// Other code here:
	elapsed++;
	printf("the time passed is: %lu\r\n", elapsed);

}

// Non-HAL GPIO/EXTI Handler
void EXTI0_IRQHandler() {
	// Clear Interrupt Bit by setting it to 1.
	EXTI->PR |= EXTI_PR_PR0;
	button1Pressed = 1;

}

//HAL - GPIO/EXTI Handler
void xxx_IRQHandler() {
	//HAL_GPIO_EXTI_IRQHandler(???);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	// ISR code here.
}



// For the HAL timer interrupts, all of the associated Callbacks need to exist,
// otherwise during assembly, they will generate compiler errors as missing symbols
// Below are the ones that are not used.

// void HAL_TIMEx_BreakCallback(TIM_HandleTypeDef *htim){};
// void HAL_TIMEx_CommutationCallback(TIM_HandleTypeDef *htim){};
