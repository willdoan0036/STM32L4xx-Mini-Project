#include "stm32l4xx.h"
/*
		2 counters, A and B
		Count Period is ~0.5 sec
		A counts from 0-9 and rolls over, switch can toggle the direction       writes to PA[8:5]
		B counts from 0-9 and rolls over, only counts up                        writes to PA[12:9]
		SW1 (PA1) is to start/stop both counters (1 = start, 0 = stop)
		SW2 (PA2) is to change direction of A (1 = down, 0 = up)
*/


// Define Global Variables
unsigned int static countA;
unsigned int static countB;

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

/*
Updates the countA and countB variables
Param:      int dirA    -   sets the direction for A to count (0 for up and 1 for down)
*/
void count(int dirA) {
    if (countB < 9) {
        countB++;
    }
    else {  // countB is at 9 and needs to roll over
        countB = 0;
    }
    if (dirA == 0) {    // A counts up
        if (countA < 9) {
            countA++;
        }
        else {  // countA is at 9 and needs to roll over
            countA = 0;
        }
    }
    else {  // dirA == 1, A counts down
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
}

/*
Main function driving the program
*/
int main(void) {
    countA = 0;
    countB = 0;
    unsigned int SW1, dirA;
    pinSetup();
    //Infinite Loop
    while(1) {
        delay();    // ~0.5 sec delay
        //get switch values
        dirA = (GPIOA->IDR & PWR_PUCRA_PA2) >> 2;   //Sets the direction to the state of SW2
        SW1 = (GPIOA->IDR & PWR_PUCRA_PA1) >> 1;    //Sets the state of SW1 to on
        // Increments counter if switch is on
        if (SW1 == 1) {
            count(dirA);
            // Writes countA and countB to the output pins
            GPIOA->ODR = (GPIOA->ODR & ~(0x1FE0)) | (((countB << 4) + countA) << 5);
        }
    }
}