# grblLCD
Small LCD to show some status info of grbl / grblESP32 controllers

Based on a Wemos D1 Mini Board with 1.44" LCD Shhield.

It sends ? messages to request the status info of grbl. This status info is then parsed to display the data.
Tested with GRBL_ESP32.

The display coneccts to the RX/TX line of the grbl controller. As such it can not be used together with the USB connection.
Might work together with USB when just connecting the TX line of grbl to the RX of the Display. 


![header image](https://raw.githubusercontent.com/derFrickler/grblLCD/master/grblLCD.jpg)
