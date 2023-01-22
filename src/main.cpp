#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h> 
#include <Adafruit_Sensor.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLEDWIDTH 128
#define OLEDHEIGHT 64
#define WIRE Wire
Adafruit_SSD1306 oled = Adafruit_SSD1306(OLEDWIDTH, OLEDHEIGHT, &WIRE);

#include "DHT.h"
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#include <time.h>
#include <DS3232RTC.h>
DS3232RTC myRTC;

#include <SPI.h>
#include <SD.h>

#define sdPIN 49
bool sdAvailable = false;




SemaphoreHandle_t xSerialSemaphore;
QueueHandle_t dataQueue;  
QueueHandle_t sdQueue;  

void senosrsTask( void *pvParameters);
void initTask( void *pvParameters );
void oledTask( void *pvParameters );
void sdCardTask(void *pvParameters);

TaskHandle_t initTaskHandle;
#define ldrPIN A0
#define pressurePin A1
#define pressureError -1.52

double temp =0;
double humidity =0;
double light =0;
double pressure =0;

time_t t;


struct dataItem
{
  char temp[10] ;
  char humidity[10] ;
  char light[10] ;
  char pressure[10] ;
  time_t time;
};

void setup() {

  Serial.begin(57600);
  
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }


   if ( xSerialSemaphore == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
    xSerialSemaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore we will use to manage the Serial Port
    if ( ( xSerialSemaphore ) != NULL )
      xSemaphoreGive( ( xSerialSemaphore ) );  // Make the Serial Port available for use, by "Giving" the Semaphore.
  }
    xTaskCreate(initTask, "initTask", 256, NULL, 1,NULL);

  
}

void loop() {
  // put your main code here, to run repeatedly:
}

/**********************Tasks**************************/
void initTask(void *pvParameters)
{
  myRTC.begin();
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  dht.begin();
  for(;;)
  {
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      Serial.println("Initializing!...");
      
      oled.clearDisplay();
      oled.display();
      oled.setTextSize(1);
      oled.setTextColor(SSD1306_WHITE);
      oled.setCursor(OLEDWIDTH/2-48,OLEDHEIGHT/2);
      oled.print(F("WeatherStation"));
      oled.display();
      vTaskDelay(50/portTICK_PERIOD_MS);
      oled.print(F("."));
      oled.display();
      vTaskDelay(200/portTICK_PERIOD_MS);
      oled.print(F("."));
      oled.display();
      vTaskDelay(200/portTICK_PERIOD_MS);
      oled.print(F("."));
      oled.display();
      oled.setCursor(OLEDWIDTH/2-22,OLEDHEIGHT/2+20);
      oled.println(F("V1.0.0"));
      oled.display();
      
      dataQueue = xQueueCreate(10,sizeof(dataItem));
      Serial.println(uxTaskGetStackHighWaterMark( NULL ));
      Serial.println("initStack");

      xTaskCreate(senosrsTask,  "sensorsRead"  ,  512  ,  NULL ,  3  ,  NULL );
      xTaskCreate(oledTask,  "oledTask"  ,  192  ,  NULL ,  2  ,  NULL );
      xTaskCreate(sdCardTask, "sdTask", 512,NULL,1,NULL);
      xSemaphoreGive(xSerialSemaphore); 
      vTaskDelete(initTaskHandle);
    }



    
  }
}
void sdCardTask(void *pvParameters)
{
  for(;;)
  {
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
    Serial.println(uxTaskGetStackHighWaterMark( NULL ));
    Serial.println("SdCard");
    dataItem item;
    xQueueReceive(dataQueue,&item,portMAX_DELAY);
    char buffer[60] ;
    snprintf(buffer,59,"%s,%s,%s,%s,%d:%d:%d %d:%d:%d",item.temp,item.humidity,item.pressure,item.light,day(item.time),month(item.time),year(item.time),hour(item.time),minute(item.time),second(item.time));
    if(SD.begin(sdPIN))
      {
        sdAvailable=true;
        Serial.println(F("SD card available"));
        File dataFile = SD.open("datalog.txt", FILE_WRITE);

        // if the file is available, write to it:
        if (dataFile) 
        {
        dataFile.println(buffer);
        dataFile.close();
        // print to the serial port too:
        Serial.println(F("Data Written"));
        }
        // if the file isn't open, pop up an error:
        else 
        {
        Serial.println(F("error opening datalog.txt"));
        }
        SD.end();
      }else{
        sdAvailable =false;
        Serial.println(F("No SD card available"));
      }
      SD.end();
    
    
    Serial.println(uxTaskGetStackHighWaterMark( NULL ));
    Serial.println(buffer);
    xSemaphoreGive(xSerialSemaphore); 
    }
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }

}
void oledTask(void *pvParameters)
{
  for(;;)
  {
    dataItem item;
    if(xQueuePeek(dataQueue,(void*)&item,portMAX_DELAY)==pdTRUE)
    {
      oled.clearDisplay();
      oled.setCursor(0,0);
      oled.print(F("Temperature: "));oled.print(item.temp);oled.println(F("C\n"));
      oled.print(F("Humidity   : "));oled.print(item.humidity);oled.println(F("%\n"));
      oled.print(F("Pressure   : "));oled.print(item.pressure);oled.println(F("kPa\n"));
      oled.print(F("Light      : "));oled.println(item.light);
      oled.display();
      
    }
     if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
    Serial.println(F("Oled"));
    Serial.print(F("Queue: "));Serial.println(uxQueueMessagesWaiting(dataQueue));
    xSemaphoreGive(xSerialSemaphore); 
    }
    vTaskDelay(500/portTICK_PERIOD_MS);
  }

}



void senosrsTask( void *pvParameters )
{
  for(;;)
  {
    xQueueReset(dataQueue);

    t=myRTC.get();                                            //time now 

    pressure = ((((((float)analogRead(pressurePin)*5)/1023)/5)+0.095)/0.009)+pressureError;   //get pressure sensor value (ADC)
    
    light = analogRead(ldrPIN);   //get ldr sensor value (ADC)
    
    temp =dht.readTemperature(false,true);  //get dht temperature value

    humidity =dht.readHumidity(true);       //get dht humidity value

    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      // We were able to obtain or "Take" the semaphore and can now access the shared resource.
      // We want to have the Serial Port for us alone, as it takes some time to print,
      // so we don't want it getting stolen during the middle of a conversion.
      // print out the TEMP:
      Serial.println(uxTaskGetStackHighWaterMark( NULL ));                //get the stack free space (Debugging)                    
      Serial.println(F("ok"));

      Serial.print(F("Light: "));Serial.println(light);
      Serial.print(F("Temp: ")); Serial.print(temp);Serial.println(F(" C"));

      Serial.print(F("Humidity: ")); Serial.print(humidity);Serial.println(F(" RH"));

      Serial.print(F("Press: ")); Serial.println(pressure);

      Serial.println(uxTaskGetStackHighWaterMark( NULL ));
      Serial.println(F("ok"));

      dataItem item;
      dtostrf(temp,1,2,item.temp);                  //temp float value to string
      dtostrf(humidity,1,2,item.humidity);
      dtostrf(pressure,1,2,item.pressure);
      dtostrf(light,1,2,item.light);
      item.time =t;
      Serial.print("i: ");Serial.println(item.temp);
      if(xQueueSend(dataQueue,( void * ) &item,portMAX_DELAY))
      {
        Serial.println(F("Queued Successfully!"));
      }
      


      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    } 
    Serial.println(uxTaskGetStackHighWaterMark( NULL ));
    vTaskDelay(500/portTICK_PERIOD_MS);
    
  }
}


