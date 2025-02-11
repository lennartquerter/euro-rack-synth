## Midi-2-cv


## Hacks and special considerations

### system_stm32f4xx.c

`SCB->VTOR = FLASH_BASE;  // <---------------- WORKAROUND!`
is needed to enable the systick handler, for some reason this does not work without.

[Forum link](https://community.st.com/t5/stm32cubemx-mcus/systick-handler-not-called-stm32g0b1/td-p/204749/page/2)


### REV 2 learnings:
- I need external pull up resistors for each I2C connection
- Ensure that the transistors for LED's are set up correct before printing.