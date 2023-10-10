# Arduino_UNO_R4_RA4M1_HOCO_to_XTAL_switch
Startup code for the Arduino UNO R4 boards (RA4M1 processor) to change from the internal HOCO clock to an external XTAL for the main processor functions.
The USB interface remains on the HOCO clock.

Whilst the HOCO clock is good enough for most general purpose applications there may be times when one is looking longingly at the unpopulated external resonator/XTAL PCB footprint...

This code enables switching to an external XTAL in the range of 4.0 to 12.5 MHz.

For reasons, this is the clock variation for the HOCO clock when using the inbuilt Clock Frequency Accuracy Measurement Circuit (CAC) - see section 9 of the processor datasheet - measuring the Peripheral clock PCLKB (which is at 1/2 the main 48 MHz processor clock).

Values are in kHz, the Orange and Green lines are the maximum and minimum values recorded (with an XTAL fitted the number is alternating between 24,000 and 23,999 just about 50/50).

![Susan-RA4M1-HOCO-PCLKB-measurment-with-min-max-bounds-1](https://github.com/TriodeGirl/Arduino_UNO_R4_RA4M1_HOCO_to_XTAL_switch/assets/139503623/e6c866ad-faa9-4a7c-9213-aabb6820d746)

Comparison between using an XTAL and the HOCO clock sources... looking at 1.000uS after the trigger (so the 12th edge), the comparison is quite notable.

This is the direct 12MHz XTAL clock on the CLOCKOUT pin.
![Susan-RA4M1-XTAL-12MHz-compare-measurment-at-1uS-1](https://github.com/TriodeGirl/Arduino_UNO_R4_RA4M1_HOCO_to_XTAL_switch/assets/139503623/fdb30142-05de-4c04-85eb-3b115e8c064a)

This is the HOCO clock, with a divide by 4, to give 12MHz (nominal) on the CLOCKOUT pin.
![Susan-RA4M1-HOCO-12MHz-compare-measurment-at-1uS-1](https://github.com/TriodeGirl/Arduino_UNO_R4_RA4M1_HOCO_to_XTAL_switch/assets/139503623/7115fde9-acac-4a4e-9164-cd4625232134)
