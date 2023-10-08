/*  Arduino UNO R4 code for switching to external resonator 12.0MHz XTAL
 *
 *  Susan Parker - 8th October 2023.
 *    Tested on EK-RA4M1, should work on both Minima and WiFi
 *    USB Serial module remains on HOCO clock source... 
 *      so non-standard XTAL can be used in the range 4 MHz to 12.5 MHz
 *      e.g. 12.288 MHZ - Adjust PLL settings to match.
 *
 * This code is "AS IS" without warranty or liability. 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

// ARM-developer - Accessing memory-mapped peripherals
// https://developer.arm.com/documentation/102618/0100

// ==== System & Clock Generation ====
#define SYSTEM 0x40010000 // ICU Base - See 13.2.6 page 233

// Register Write Protection - See section 12
// PRC0 Registers related to the clock generation circuit:
//   SCKDIVCR, SCKSCR, PLLCR, PLLCCR2, MEMWAIT, MOSCCR, HOCOCR, MOCOCR, CKOCR, TRCKCR,
//   OSTDCR, OSTDSR, SLCDSCKCR, EBCKOCR, MOCOUTCR, HOCOUTCR, MOSCWTCR, MOMCR, SOSCCR,
//   SOMCR, LOCOCR, LOCOUTCR, HOCOWTCR, USBCKCR
//
//   *SYSTEM_PRCR = 0xA501;     // Enable writing to the clock registers
//   *SYSTEM_PRCR = 0xA500;     // Disable writing to the clock registers

#define SYSTEM_PRCR  ((volatile unsigned short *)(SYSTEM + 0xE3FE))    // Protect Register
#define PRCR_PRC0           0   // Enables or disables writing to clock generation registers
#define PRCR_PRC1           1   // En/Dis writing to the low power modes and battery backup function registers
#define PRCR_PRC3           3   // Enables or disables writing to the registers related to the LVD
#define PRCR_PRKEY_7_0      8   // Control write access to the PRCR register.
#define PRCR_PRKEY       0xA5   // PRC Key Code - write to the upper 8 bits

#define SYSTEM_SCKDIVCR  ((volatile unsigned int *)(SYSTEM + 0xE020))  // System Clock Division Control Register
                                // SYSTEM_SCKDIVCR = 100010100 
#define SCKDIVCR_PCKD_2_0   0   // Peripheral Module Clock D           = 4; 1/16
#define SCKDIVCR_PCKC_2_0   4   // Peripheral Module Clock C           = 1; 1/2
#define SCKDIVCR_PCKB_2_0   8   // Peripheral Module Clock B           = 1; 1/2
#define SCKDIVCR_PCKA_2_0  12   // Peripheral Module Clock A           = 0
#define SCKDIVCR_ICK_2_0   24   // System Clock (ICLK) Select          = 0
#define SCKDIVCR_FCK_2_0   28   // Flash Interface Clock (FCLK) Select = 0
#define SYSTEM_SCKSCR  ((volatile unsigned char *)(SYSTEM + 0xE026))  // System Clock Source Control Register
#define SCKSCR_CKSEL_2_0    0   // Clock Source Select - See section 8.2.2
#define SYSTEM_PLLCR   ((volatile unsigned char *)(SYSTEM + 0xE02A))  // PLL Control Register
#define PLLCR_PLLSTP        0   // PLL Stop Control; 0: PLL is operating, 1: PLL is stopped
#define SYSTEM_PLLCCR2 ((volatile unsigned char *)(SYSTEM + 0xE02B))  // PLL Clock Control Register 2
#define PLLCCR2_PLLMUL_4_0  0   // PLL Frequency Multiplication Factor Select
#define PLLCCR2_PLODIV_1_0  6   // PLL Output Frequency Division Ratio Select
#define SYSTEM_MEMWAIT ((volatile unsigned char *)(SYSTEM + 0xE031))  // Memory Wait Cycle Control Register
#define MEMWAIT_MEMWAIT     0   // Memory Wait Cycle Select; 0: No wait, 1: Wait
#define SYSTEM_MOSCCR   ((volatile unsigned char *)(SYSTEM + 0xE032))  // Main Clock Oscillator Control Register
#define MOSCCR_MOSTP        0   // Main Clock Oscillator Stop; 0: Main clock oscillator is operating, 1: MCO is stopped
#define SYSTEM_HOCOCR   ((volatile unsigned char *)(SYSTEM + 0xE036))  // High-Speed On-Chip Oscillator Control Register
#define HOCOCR_HCSTP        0   // HOCO Stop; 0: HOCO is operating, 1: HOCO is stopped
#define SYSTEM_MOCOCR   ((volatile unsigned char *)(SYSTEM + 0xE038))  // Middle-Speed On-Chip Oscillator Control Register
#define MOCOCR_MCSTP        0   // MOCO Stop; 0: MOCO is operating, 1: MOCO is stopped
#define SYSTEM_OSCSF    ((volatile unsigned char *)(SYSTEM + 0xE03C))  // Oscillation Stabilization Flag Register
#define OSCSF_HOCOSF        0   // HOCO Clock Oscillation Stabilization Flag; 0: The HOCO clock is stopped or not stable, 1: The clock is stable
#define OSCSF_MOSCSF        3   // Main Clock Oscillation Stabilization Flag; 0: The Main clock is stopped or not stable, 1: The clock is stable
#define OSCSF_PLLSF         5   // PLL  Clock Oscillation Stabilization Flag; 0: The PLL  clock is stopped or not stable, 1: The clock is stable
#define SYSTEM_CKOCR    ((volatile unsigned char *)(SYSTEM + 0xE03E))  // Clock Out Control Register
#define CKOCR_CKOSEL_2_0    0   // Clock Out Source Select; 000: HOCO, 001: MOCO, 010: LOCO, 011: MOSC, 100: SOSC
#define CKOCR_CKODIV_2_0    4   // Clock Out Input Frequency Division Select; 000: ×1, 001: /2, 010: /4, ... , 111: /128
#define CKOCR_CKOEN         7   // Clock Out Enable; 0: Disable clock out, 1: Enable clock out
#define SYSTEM_TRCKCR   ((volatile unsigned char *)(SYSTEM + 0xE03F))  // Trace Clock Control Register
#define TRCKCR_TRCK_3_0     0   // Trace Clock Operation Frequency Select; 0000: /1, 0001: /2, 0010: /4 ( /2 = value after reset )
#define TRCKCR_TRCKEN       7   // Trace Clock Operating Enable; 0: Disable clock, 1: Enable clock

#define SYSTEM_OSTDCR   ((volatile unsigned char *)(SYSTEM + 0xE040))  // Oscillation Stop Detection Control Register
#define OSTDCR_OSTDIE       0   // Oscillation Stop Detection Interrupt Enable; 0: Disable oscillation stop detection interrupt, 1: Enable OSDI
#define OSTDCR_OSTDE        7   // Oscillation Stop Detection Function Enable; 0: Disable the oscillation stop detection function, 1: Enable OSDF
#define SYSTEM_OSTDSR   ((volatile unsigned char *)(SYSTEM + 0xE041))  // Oscillation Stop Detection Control Register
#define OSTDSR_OSTDF        0   // Oscillation Stop Detection Flag; 0: Main clock oscillation stop not detected, 1: Main clock oscillation stop detected

#define SYSTEM_SLCDSCKCR    ((volatile unsigned char *)(SYSTEM + 0xE050))  // Segment LCD Source Clock Control Register
#define SLCDSCKCR_LCDSCKSEL_2_0  0  // LCD Source Clock Select; 000: LOCO, 001: SOSC, 010: MOSC, 100: HOCO
#define SLCDSCKCR_LCDSCKEN       7  // LCD Source Clock Out Enable; 0: LCD source clock out disabled, 1: LCD source clock out enabled

#define SYSTEM_MOCOUTCR   ((volatile unsigned char *)(SYSTEM + 0xE061))  // MOCO User Trimming Control Register
#define MOCOUTCR_MOCOUTRM_7_0   0  // MOCO User Trimming - See: 8.2.21
#define SYSTEM_HOCOUTCR   ((volatile unsigned char *)(SYSTEM + 0xE062))  // HOCO User Trimming Control Register
#define HOCOUTCR_HOCOUTRM_7_0   0  // HOCO User Trimming - See: 8.2.21

#define SYSTEM_MOSCWTCR ((volatile unsigned char *)(SYSTEM + 0xE0A2))  // Main Clock Oscillator Wait Control Register
#define MOSCWTCR_MSTS_3_0   0   // Main Clock Oscillator Wait Time Setting
#define SYSTEM_HOCOWTCR ((volatile unsigned char *)(SYSTEM + 0xE0A5))  // High-Speed On-Chip Oscillator Wait Control Register
#define HOCOWTCR_MSTS_2_0   0   // HOCO Wait Time Setting

#define SYSTEM_USBCKCR  ((volatile unsigned char *)(SYSTEM + 0xE0D0))  // USB Clock Control Register
#define USBCKCR_USBCLKSEL   0   // USB Clock Source Select; 0: PLL (value after reset), 1: HOCO

#define SYSTEM_MOMCR    ((volatile unsigned char *)(SYSTEM + 0xE413))  // Main Clock Oscillator Mode Oscillation Control Register
#define MOMCR_MODRV1        3   // Main Clock Oscillator Drive Capability 1 Switching; 0: 10 MHz to 20 MHz, 1: 1 MHz to 10 MHz
#define MOMCR_MOSEL         6   // Main Clock Oscillator Switching; 0: Resonator, 1: External clock input

#define SYSTEM_SOSCCR   ((volatile unsigned char *)(SYSTEM + 0xE480))  // Sub-Clock Oscillator Control Register
#define SOSCCR_SOSTP        0   // Sub-Clock Oscillator Stop; 0: Operate the sub-clock oscillator, 1: Stop the sub-clock osc
#define SYSTEM_SOMCR    ((volatile unsigned char *)(SYSTEM + 0xE481))  // Sub-Clock Oscillator Mode Control Register
#define SOMCR_SODRV_1_0     0   // Sub-Clock Oscillator Drive Capability Switching; 00: Normal, 01: Low-power 1, 10: Low-power 2, 11: Low-power 3

#define SYSTEM_LOCOCR   ((volatile unsigned char *)(SYSTEM + 0xE490))  // Low-Speed On-Chip Oscillator Control Register
#define LOCOCR_LCSTP        0   // LOCO Stop; 0: Operate the LOCO clock, 1: Stop the LOCO clock
#define SYSTEM_LOCOUTCR ((volatile unsigned char *)(SYSTEM + 0xE492))  // LOCO User Trimming Control Register
#define LOCOUTCR_LOCOUTRM_7_0   0  // LOCO User Trimming - See: 8.2.20

#define SYSTEM_RSTSR0   ((volatile unsigned char *)(SYSTEM + 0xE410))  // Reset Status Register 0
#define RSTSR0_PORF         0    // Power-On Reset Detect Flag
#define RSTSR0_LVD0RF       1    // Voltage Monitor 0 Reset Detect Flag
#define RSTSR0_LVD1RF       2    // Voltage Monitor 1 Reset Detect Flag
#define RSTSR0_LVD2RF       3    // Voltage Monitor 2 Reset Detect Flag
#define SYSTEM_RSTSR1   ((volatile unsigned char *)(SYSTEM + 0xE0C0))  // Reset Status Register 1
#define RSTSR1_IWDTRF       0    // Independent Watchdog Timer Reset Detect Flag
#define RSTSR1_WDTRF        1    // Watchdog Timer Reset Detect Flag
#define RSTSR1_SWRF         2    // Software Reset Detect Flag
#define SYSTEM_RSTSR2   ((volatile unsigned char *)(SYSTEM + 0xE411))  // Reset Status Register 2
#define RSTSR2_CWSF         0    // Cold/Warm Start Determination Flag - 0: Cold start, 1: Warm start

// === Local Defines

#define CLOCK_PLL      // Use when external 12.0MHz crystal fitted


void setup()
  {
  Serial.begin(115200);      // The interrupts for the USB serial are already in place before setup() starts
  while (!Serial){};         // 

#ifdef CLOCK_PLL
  sys_clock_pll_setup();
#else
  Serial.println("Default HOCO Clock"); 
#endif

  }

void loop()
  {
  delay(100);
  }

void sys_clock_pll_setup(void)
  {
  Serial.println("Setup MOSC - XTAL & PLL"); 
  *SYSTEM_PRCR     = 0xA501;          // Enable writing to the clock registers
  *SYSTEM_MOSCCR   = 0x01;            // Make sure XTAL is stopped
  *SYSTEM_MOMCR    = 0x00;            // MODRV1 = 0 (10 MHz to 20 MHz); MOSEL = 0 (Resonator)
  *SYSTEM_MOSCWTCR = 0x07;            // Set stability timeout period 
  *SYSTEM_MOSCCR   = 0x00;            // Enable XTAL
	asm volatile("dsb");                // Data bus Synchronization instruction
  char enable_ok = *SYSTEM_MOSCCR;    // Check bit 
  delay(100);                         // wait for XTAL to stabilise  
  *SYSTEM_PLLCR    = 0x01;            // Disable PLL
  *SYSTEM_PLLCCR2  = 0x07;            // Setup PLLCCR2_PLLMUL_4_0 PLL PLL Frequency Multiplication to 8x
  *SYSTEM_PLLCCR2 |= 0x40;            // Setup PLLCCR2_PLODIV_1_0 PLL Output Frequency Division to /2
  *SYSTEM_PLLCR    = 0x00;            // Enable PLL
  delayMicroseconds(1000);            // wait for PLL to stabilise
  *SYSTEM_SCKSCR   = 0x05;            // Select PLL as the system clock 
  *SYSTEM_PRCR     = 0xA500;          // Disable writing to the clock registers
  }
