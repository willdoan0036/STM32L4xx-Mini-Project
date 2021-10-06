/*====================================================*/
/* Tuan Anh Doan*/
/* ELEC 3040 - Lab 3 */
/* Decade counter controlled by two switches and outputted to LEDs */
/*====================================================*/
#include "stm3214xx.h" /* Microcontroller information */

// Define Global Variables
unsigned int static countA;
unsigned int static countB;
unsigned int run;
unsigned int LED3;
unsigned int LED4;
unsigned char up;
/*
Runs two for loops, one nested in the other, to create a delay of ~0.5 sec
*/
void delay() {
	int n;
    for (int i=0; i < 20; i++) {
        for (int j=0; j < 25000; j++) {
						n = j;//__nop();
        }
    }
}

/* */
void count(int up) {
    if (countB < 9) {
        countB++;
    }
    else {  // countB is at 9 and needs to roll over
        countB = 0;
    }
    if (up == 1) {    // A counts up
        if (countA < 9) {
            countA++;
        }
        else {  // countA is at 9 and needs to roll over
            countA = 0;
        }
    }
    else {  // up == 1, A counts down
        if (countA > 0) {
            countA--;
        }
        else {  // countA is at 0 and needs to roll over
            countA = 9;
        }
    }
}

/*
Function to setup the GIO pins for the program to run
*/
void pinSetup() {
		// Enable GPIOA clock (bit 0)
    RCC->AHB2ENR |= 0x01;
    // Sets PA1-2=00 for input and PA5-12=01 for output
		GPIOA->MODER &= ~(0x03FFFC3C);		//Clearing PA1-2 and PA5-12
    GPIOA->MODER |= 0x01555400;				//Setting PA1-2 for input and PA5-12 for output
	  // Configure PB3-PB4 as output LED toggled by PA1-PA2
		RCC->AHB2ENR |= 0x02; // Enable GPIOB clock (bit 1)
		GPIOB->MODER &= ~(0x000003C0); // Clear PB3-PB4 mode bits
		GPIOB->MODER |=   0x00000140; // General purpose output mode
}
void InterruptSetup() {
	
	SYSCFG->EXTICR[0] &= 0xFF0F; // Configure EXTI1 to be triggered by PA1
	
	SYSCFG->EXTICR[0] &= 0xF0FF; // Clear EXTI2
	SYSCFG->EXTICR[0] |= 0x0200; // Configure EXTI2 to be triggered by PA2
	
	EXTI->RTSR1 |= 0x0006; // Set EXTI1 and EXTI2 to be rising edge triggered
	EXTI->IMR1 |= 0x0006; // Set interrupt masks for EXTI1 and EXTI2
	EXTI->PR1 |= 0x0006; // Set pending register for EXTI1 and EXTI2
	
	NVIC_EnableIRQ(EXTI1_IRQn); // Enable EXTI1
	NVIC_EnableIRQ(EXTI2_IRQn); // Enable EXTI2
	
	NVIC_ClearPendingIRQ(EXTI1_IRQn); // Clear pending register for EXTI1
	NVIC_ClearPendingIRQ(EXTI2_IRQn); // Clear pending register for EXTI2
}
/*----------------------------------------------------------*/
/* EXTI1_IRQHandler function - performs operations when EXTI1 is triggered */
/*----------------------------------------------------------*/
void EXTI1_IRQHandler() {
	EXTI->PR1 |= 0x0002; //Set pending register for EXTI1
	
	run = ~run; // Set Counter A & Counter B to on/off
	
	if (LED3 == 1) {
		GPIOB->BSRR |= 0x0008 << 16; // Turn LED3 off
		LED3 = 0;
	}
	else {
		GPIOB->BSRR |= 0x0008; // Turn LED3 on
		LED3 = 1;
	}

	NVIC_ClearPendingIRQ(EXTI1_IRQn); //Reset pending register for EXTI1
}

/*----------------------------------------------------------*/
/* EXTI2_IRQHandler function - performs operations when EXTI2 is triggered */
/*----------------------------------------------------------*/
void EXTI2_IRQHandler() {
	EXTI->PR2 |= 0x0004; //Set pending register for EXTI2
	
	up = ~up; //Set Counter A to change direction 
	
	if (LED4 == 1) {
		GPIOA->BSRR |= 0x0010 << 16; //Turn LED4 off
		LED4 = 0;
	}
	else {
		GPIOA->BSRR |= 0x0010; // Turn LED4 on
		LED4 = 1;
	}

	NVIC_ClearPendingIRQ(EXTI2_IRQn); //Reset pending register for EXTI2
}
/*
Main function driving the program
*/
int main(void) {
    countA = 0;
    countB = 0;
    unsigned int run;
		unsigned char up;
    pinSetup();
		InterruptSetup(); // Configure interrupts
	
		GPIOB->BSRR |= 0x0008; // Set LED3 to initially be on
		LED3 = 1;
		GPIOB->BSRR |= 0x0010; // Set LED4 to initially be on
		LED4 = 1;
	
		__enable_irq(); //Enable interrupts to occur
    //Infinite Loop
    while(1) {
        delay();    // ~0.5 sec delay
        //get switch values
        up = (GPIOA->IDR & PWR_PUCRA_PA2) >> 2;   //Sets the direction to the state of SW2
        run = (GPIOA->IDR & PWR_PUCRA_PA1) >> 1;    //Sets the state of run to on
        // Increments counter if switch is on
        if (run == 1) {
            count(up);
            // Writes countA and countB to the output pins
            GPIOA->ODR = (GPIOA->ODR & ~(0x1FE0)) | (((countB << 4) + countA) << 5);
        }
    }
}