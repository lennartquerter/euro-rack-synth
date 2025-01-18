## Midi-2-cv


## Hacks and special considerations

### system_stm32f4xx.c

`SCB->VTOR = FLASH_BASE;  // <---------------- WORKAROUND!`
is needed to enable the systick handler, for some reason this does not work without.

[Forum link](https://community.st.com/t5/stm32cubemx-mcus/systick-handler-not-called-stm32g0b1/td-p/204749/page/2)

### 20.000 HZ oscillator
Ideally the STM runs off the 20MHZ crystal, but for some reason on the MIDAS board it can not be
found, there is a timeout that happens. This does not happen on the F411 protoboard I built. 

One possible explanation would be the tracks are too long?