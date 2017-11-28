# CC3220SF LAUNCHXL Settings and Resources

The CC3220SF LAUNCHXL board contains a CC3220SF device.

![](./images/CC3220SF_LAUNCHXL.jpg "CC3220SF LAUNCHXL")

## Jumper Settings

* Set __RX__ and __TX__ to provide UART communications via the onboard USB debugger.

## Board Resources Used in Driver Examples

The following table lists the Board names of the resources used by
the driver examples, and their corresponding pins.  Note that there may be
other resources configured in the board files that are not used by any
driver example.  Only the example resources are listed in the table.

  |Board Resource|Pins|
  |--------------|:---|
  |`Board_ADC0`|P59  (Connect to analog signal)|
  |`Board_ADC1`|P60  (Connect to analog signal)|
  |`Board_CAPTURE0`|P04 |
  |`Board_CAPTURE1`|P05 |
  |`Board_GPIO_LED0`|P64  (__LED EN__ must be jumpered)|
  |`Board_GPIO_LED1`|P64  (__LED EN__ must be jumpered)|
  |`Board_GPIO_LED2`|P64  (__LED EN__ must be jumpered)|
  |`Board_GPIO_BUTTON0`|P15  (`SW2`)|
  |`Board_GPIO_BUTTON1`|P04  (`SW3`)|
  |`Board_I2C0`|P01 - `SCL`, P02 - `SDA`|
  |`Board_I2C_TMP`|P01 - `SCL`, P02 - `SDA`  (__SCL__ & __SDA__ must be jumpered)|
  |`Board_I2S0`|P50 - `XR1`, P53 - `CLK`, P62 - `CLKX`, P63 - `FSX`, P64 - `XR0`|
  |`Board_PWM0`|P01  (__LED EN__ must be jumpered)|
  |`Board_PWM1`|P02  (__LED EN__ must be jumpered)|
  |`Board_SDSPI0`|P05 - `CLK`, P06 - `MISO`, P07 - `MOSI`, P62 - `CS`|
  |`Board_SD0`|P06 - `DATA`, P07 - `CLK`, P08 - `CMD`|
  |`Board_SDFatFS0`|P06 - `DATA`, P07 - `CLK`, P08 - `CMD`|
  |`Board_SPI0`|P05 - `CLK`, P06 - `MISO`, P07 - `MOSI`, P08 - `CS`|
  |`Board_UART0`|P57 - `RX`, P55 - `TX`  (UART provided through emulation, __RX__ and __TX__ must be jumpered)|
  |`Board_UART1`|P08 - `RX`, P07 - `TX`|

## BoosterPacks

The following examples require BoosterPacks.

  |Example|BoosterPack|
  |-------|:------------|
  |fatsd|[microSD Card BoosterPack](http://boardzoo.com/index.php/boosterpacks/microsd-boosterpack.html#.WBjQnXr9xv4) or [SD Card BoosterPack](http://store.43oh.com/index.php?route=product/product&path=64&product_id=66)|
  |fatsdraw|[microSD Card BoosterPack](http://boardzoo.com/index.php/boosterpacks/microsd-boosterpack.html#.WBjR0nr9xv4) or [SD Card BoosterPack](http://store.43oh.com/index.php?route=product/product&path=64&product_id=66)|
  |i2secho|[Audio BoosterPack](http://www.ti.com/tool/cc3200audboost)|
  |sdfatfs|[SD Card BoosterPack](http://store.43oh.com/index.php?route=product/product&path=64&product_id=66)|
  |sdfatfsraw|[SD Card BoosterPack](http://store.43oh.com/index.php?route=product/product&path=64&product_id=66)|
  |portable|[BOOSTXL-SENSORS Sensors BoosterPack](http://www.ti.com/tool/boostxl-sensors) (optional)|

>For the __sdfatfs__ and __sdfatfsraw__ examples, modifications must be made
to the SD Card BoosterPack. See the section below for details.

### SD Card BoosterPack Modifications
![](./images/SD_Card.jpg "CC3220S LAUNCHXL")

Scratch out the following traces:
* 1 - `SJ3`
* 2 - `SJ1`
* 3 - `SJ4`
* 4 - `SJ2`

Solder shorts for the following:
* 5 - `J2.4`
* 6 - `J2.5`
* 7 - `J2.8`

## ADC Examples Pin Connections

The ADC examples sample input from the A0 and A1 pins.  These pins should
be connected to the analog input. For quick testing on the __CC3220SF LAUNCHXL__, the A0 pin can be
connected to `3V3` and A1 to `GND`. The __CC3220SF LAUNCHXL__ has a protective circuit to allow an input of 3.3VDC.
The `5V` should NOT be used for this may damage the ADC inputs. For testing, the following jumpers must be present:
`J5` = `OPAMP EN`, `J10` = `GND`, `J13` = `BRD`.

  |ADC Pin (Function)||Analog Input|
  |------------------|-|:-----------|
  |P59 (A0)|-->|3V3|
  |P60 (A1)|-->|GND|

>__Note:__ There are duplicate pins P59 and P60 on the board. Choose the ones connected to the Booster Pack headers (the pins not connected to the Booster Pack headers have an asterisk next to them).

>__Important:__ The inputs to the ADC are required to be well within 1.4V to avoid clipping, which will cause distortion.
The ADC inputs can be damaged if an input voltage higher than 1.8V is applied to these pin. The 1.8V hard limit must be considered from both the SW and HW points of view.
Always add a resistor divider/buffer to bring down the measurement signal voltage to within the ADC limits. See the [CC32xx ADC Appnote](http://processors.wiki.ti.com/index.php/CC32xx_ADC_Appnote)
for more details.

## Peripherals Used

The following list shows which CC3220SF peripherals are used by
driver and kernel applications. Driver examples control which peripherals (and which ports) are used.

* __TI-RTOS Kernel (SYS/BIOS).__ Uses the first general-purpose timer available and that timer's associated interrupts. Generally, this will be Timer\_A0. The TI-RTOS Kernel manages the interrupt controller statically without an interrupt dispatcher.
* __Drivers.__
    * __I<sup>2</sup>C:__ The I<sup>2</sup>C driver is configured to support various BoosterPacks.
    * __PWM:__ The PWM driver uses the onboard LEDs (P01 and P02). These pins are configured for the PWM driver. While these pins can also be used by the GPIO driver, your application's board file must ensure that the pins are not simultaneously used by the GPIO and PWM drivers.
    * __SD Card:__ Uses FatFs and the SDSPI driver without interrupts to read and write to files.
    * __SPI:__ The SPI driver is configured for SPI communications.
    * __Timer:__ The Timer and PWM driver uses the timer peripheral.
    * __UART:__ The UART driver uses UART0, which is attached to the onboard emulator to facilitate serial communications.
    * __Watchdog:__ The Watchdog driver example uses the Watchdog Timer peripheral.
