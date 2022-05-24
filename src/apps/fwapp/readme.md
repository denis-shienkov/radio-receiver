
# Used pins for stm32f401xc

## PORT A

//  * PA0 - ADC1 ch0 (LIN audio)
//  * PA1 - ADC1 ch1 (RIN audio)
  * PA2 - UART2 TX (traces)
?  * PA3 - RSTB

  * PA11 - USB DM
  * PA12 - USB DP

  * PA13 - SWDIO
  * PA14 - SWCLK

## PORT B

?  * PB10 - I2C SCL
?  * PA11 - I2C SDA

## PORT C

  * PC13 - LED (blinking)
?  * PC14 - SEN
?  * PC15 - GPIO2

# Used clocks from HSE - 8 MHz

## USB

  * USBCLK  - 48 MHz

## AHB

  * HCLK (AHB, core, mem, DMA) - 84 MHz
  * SYSTIMCLK (System timer) - 84/8 = 10.5 MHz
  * FCLK - 72 MHz

## APB1

  * PCLK1 (APB1 periph) - 42 MHz

## APB2

  * PCLK2 (APB2 periph) - 84 MHz
?  * ADCCLK - 72/8 = 9 MHz
