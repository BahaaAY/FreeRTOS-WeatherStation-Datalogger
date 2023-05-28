# FreeRTOS-WeatherStation-Datalogger


### Introduction: 

  The project was developed to create a system for collecting and displaying data from multiple sensors, specifically a DHT11 temperature and humidity sensor, a DS3232 real-time clock (RTC), a light sensor using an LDR, and an MPX4115 pressure sensor. The data from these sensors is displayed on an SSD1306 OLED display and saved on an SD card. The project utilizes the Arduino Mega 2560 as the main microcontroller board, along with the Arduino_FreeRTOS library to schedule tasks for collecting and displaying the sensor data.

![alt text](https://github.com/BahaaAY/FreeRTOS-WeatherStation-Datalogger/blob/master/Images/fig1.png "Circuit Design in Proteus")

___
### Hardware Design:

  The hardware design for this project involves connecting the sensors to the Arduino Mega 2560 board through various pins and buses. The OLED display and the RTC are connected to the Arduino board via the I2C bus, while the SD card is connected via the SPI bus with the chip select pin set to pin 48. The DHT11 temperature and humidity sensor is connected to digital pin 7, the LDR light sensor is connected to analog input pin A0, and the MPX4115 pressure sensor is connected to analog input pin A1.

![alt text](https://github.com/BahaaAY/FreeRTOS-WeatherStation-Datalogger/blob/master/Images/fig2.png "Hardware Used")

___
### Sample Result:

The weather station and data logger project was successfully implemented and tested. The sensors were able to continuously collect data and the OLED display was able to display the data in a clear and easy-to-read manner. The data was also successfully saved to the SD card and could be easily accessed and analyzed later as shown in the following test sample:

![alt text](https://github.com/BahaaAY/FreeRTOS-WeatherStation-Datalogger/blob/master/Images/fig3.png "Startup Sequence")

![alt text](https://github.com/BahaaAY/FreeRTOS-WeatherStation-Datalogger/blob/master/Images/fig4.png "Sample Result")
