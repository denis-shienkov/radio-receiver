
# Used pins

## PORT A

  * PA9  - UART TX (traces)

  * PA11 - USB DM
  * PA12 - USB DP

  * PA13 - SWDIO
  * PA14 - SWCLK

## PORT C

  * PC13 - LED (blinking)

# Used clocks from HSE - 8 MHz

## USB

  * USBCLK  - 48 MHz

## AHB

  * HCLK (AHB, core, mem, DMA) - 72 MHz
  * SYSTIMCLK (System timer) - 72/8 = 9 MHz
  * FCLK - 72 MHz

## APB1

  * PCLK1 (APB1 periph) - 36 MHz

## APB2

  * PCLK2 (APB2 periph) - 72 MHz
  * ADCCLK - 72/8 = 9 MHz
