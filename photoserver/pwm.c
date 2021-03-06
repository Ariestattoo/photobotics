/*
 * pwm.cpp
 *
 *  Created on: 2012-05-28
 *      Author: steve
 *      Reference: AM335x ARM® CortexTM-A8 Microprocessors (MPUs) Technical Reference Manual
 *
 */
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

//Module Registers for PWM Initialisation
#define CLCK_MOD_PER_REG_START	0x44E00000	// Address of the Clock Module Peripheral Registers pg209
#define CLCK_MOD_PER_REG_LENGTH	1024		// Length of the CMPR
#define CM_PER_EPWMSS0_CLKCTRL_OFFSET 0xd4
#define CM_PER_EPWMSS1_CLKCTRL_OFFSET 0xcc
#define CM_PER_EPWMSS2_CLKCTRL_OFFSET 0xd8
#define PWM_CLOCK_ENABLE 0x2
#define CONT_MOD_REG_START 		0x44e10000  // Address of the Control Module Registers pg210
#define CONT_MOD_REG_LENGTH 	131072 		// Length of the CMR
#define CONF_GPMC_A2_OFFSET		0x848		// Port 9, Pin 14 MUX
//Initialise the PWM hardware. Takes period frequency and duty cycle
int initialisePWM(uint16_t periodFreq, uint8_t dutyPercent) {

	volatile uint32_t *MemMapAddressContMod; //mmap pointer for the Control Module Registers
	int32_t MemoryFileDescriptor;
	int32_t pwmFileDescriptor;
	char buffer[8] = "1500706";

	//Create a file handle for the memory
	MemoryFileDescriptor = open("/dev/mem", O_RDWR); //Establish connection between file and file descriptor as read/write
	//Check that the file opened properly
	if (MemoryFileDescriptor == -1) {
		perror("Failed to Open /dev/mem file descriptor\n");
		return 1;
	}
	fprintf(stderr, "open file descriptor\n");

	//Open Clock Module Peripheral Registers mmap from the memory file descriptor
	//Open the map in read/write mode, shared=change the actual object, using file descriptor from above and offsetting to the register.
	MemMapAddressContMod = (volatile uint32_t *) mmap(NULL,
			CLCK_MOD_PER_REG_LENGTH, PROT_WRITE, MAP_SHARED,
			MemoryFileDescriptor, CLCK_MOD_PER_REG_START);
	if (MemMapAddressContMod == (volatile uint32_t *) MAP_FAILED) {
		perror("mmap failed for Control Module Registers");
		close(MemoryFileDescriptor);
		return 1;
	}
	fprintf(stderr, "mmap looks good\n");
	//Set PWM1 clock to enable
	MemMapAddressContMod[CM_PER_EPWMSS1_CLKCTRL_OFFSET / sizeof(uint32_t)] =
			PWM_CLOCK_ENABLE;
	fprintf(stderr, "mmap of pwmr %d\n",
			MemMapAddressContMod[CM_PER_EPWMSS1_CLKCTRL_OFFSET
					/ sizeof(uint32_t)]);
	//Open Control Module mmap from the memory file descriptor
	//Open the map in read/write mode, shared=change the actual object, using file descripter from above and offseting to the register.
	MemMapAddressContMod = (volatile uint32_t *) mmap(NULL, CONT_MOD_REG_LENGTH,
			PROT_WRITE | PROT_READ, MAP_SHARED, MemoryFileDescriptor,
			CONT_MOD_REG_START);
	if (MemMapAddressContMod == (volatile uint32_t *) MAP_FAILED) {
		perror("mmap failed for Control Module Registers");
		close(MemoryFileDescriptor);
		return 1;
	}
	//Set Port 9 Pin 14 to ehrpwm1A_mux1
	MemMapAddressContMod[CONF_GPMC_A2_OFFSET / sizeof(uint32_t)] = 0x6;
	fprintf(stderr, "mmap of mux %d\n",
			MemMapAddressContMod[CONF_GPMC_A2_OFFSET / sizeof(uint32_t)]);

	//Close the Control Module mmap
	munmap((void *) MemMapAddressContMod, CONT_MOD_REG_LENGTH);

	//Close the Memory File Descriptor
	close(MemoryFileDescriptor);

	pwmFileDescriptor = open("/sys/kernel/debug/omap_mux/gpmc_a2", O_RDWR);
	write(pwmFileDescriptor, &buffer[6], 1);
	close(pwmFileDescriptor);
	pwmFileDescriptor = open("/sys/class/pwm/ehrpwm.1:0/request", O_RDWR);
	write(pwmFileDescriptor, &buffer[0], 1);
	close(pwmFileDescriptor);
	pwmFileDescriptor = open("/sys/class/pwm/ehrpwm.1:0/run", O_RDWR);
	write(pwmFileDescriptor, &buffer[0], 1);
	close(pwmFileDescriptor);
	pwmFileDescriptor = open("/sys/class/pwm/ehrpwm.1:0/period_freq", O_RDWR);
	write(pwmFileDescriptor, &buffer[1], 2);
	close(pwmFileDescriptor);
	pwmFileDescriptor = open("/sys/class/pwm/ehrpwm.1:0/duty_percent", O_RDWR);
	write(pwmFileDescriptor, &buffer[4], 1);
	close(pwmFileDescriptor);

	pwmFileDescriptor = open("/sys/kernel/debug/omap_mux/gpmc_a3", O_RDWR);
	write(pwmFileDescriptor, &buffer[6], 1);
	close(pwmFileDescriptor);
	pwmFileDescriptor = open("/sys/class/pwm/ehrpwm.1:1/request", O_RDWR);
	write(pwmFileDescriptor, &buffer[0], 1);
	close(pwmFileDescriptor);
	pwmFileDescriptor = open("/sys/class/pwm/ehrpwm.1:1/run", O_RDWR);
	write(pwmFileDescriptor, &buffer[0], 1);
	close(pwmFileDescriptor);
	pwmFileDescriptor = open("/sys/class/pwm/ehrpwm.1:1/period_freq", O_RDWR);
	write(pwmFileDescriptor, &buffer[1], 2);
	close(pwmFileDescriptor);
	pwmFileDescriptor = open("/sys/class/pwm/ehrpwm.1:1/duty_percent", O_RDWR);
	write(pwmFileDescriptor, &buffer[4], 1);
	close(pwmFileDescriptor);

	return 0;
}


