// Open Serial Monitor to see the simaltaneous waveforms from Channel 0 and 1 (you can check simaltaneous sampling by applying same wave on both channels)

#include <MCP3903.h>
#include <SPI.h>
MCP3903 adc;    // create an instance

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Define SPI pins for the ESP32
#define SCK_PIN 18   // Serial Clock
#define MISO_PIN 19  // Master In Slave Out
#define MOSI_PIN 23  // Master Out Slave In
#define CS_PIN 5     // Chip Select

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double result_ch0, result_ch1;

double* adcData1[10];  // Create 20 arrays for channel 0
double* adcData2[10];  // Create 20 arrays for channel 1
const int dataPointsPerArray = 500; // 500 points per array (5000 total)
int arrayIndex = 0; // To track current array being filled
int pointIndex = 0; // To track index within the current array

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  // Allocate memory dynamically for each array
  for (int i = 0; i < 10; i++) 
  { adcData1[i] = (double*)malloc(dataPointsPerArray * sizeof(double));
    adcData2[i] = (double*)malloc(dataPointsPerArray * sizeof(double));
  }

  Serial.begin(500000);

  // Initialize SPI with custom pins for ESP32 and set max SPI speed to 10 MHz
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));  // 10 MHz, MSBFIRST, Mode 0

  // Initialize MCP3903 configuration
  adc.reset();
  adc.init_config(MCP3903::o128, MCP3903::p1, 0, 0, 1, 1, 1, 0, 0, 0, 0);
  adc.init_status(MCP3903::group, 24, 0, MCP3903::lag, MCP3903::lag, MCP3903::lag);
  adc.Gain(0, MCP3903::g1);
  adc.Gain(1, MCP3903::g1);
  adc.phase('a', 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() 
{ // Collect data from ADC channels
  if (pointIndex < dataPointsPerArray) 
  { adc.read2ADC(0, 1, result_ch0, result_ch1);
    adcData1[arrayIndex][pointIndex] = result_ch0;  // Store ADC channel 0 data
    adcData2[arrayIndex][pointIndex] = result_ch1;  // Store ADC channel 1 data
    pointIndex++;  // Move to the next array index
  } 

  else if (arrayIndex < 9) 
  { // Move to next array when 500 points are collected
    arrayIndex++;
    pointIndex = 0;  // Reset index for the new array
  } 

  else 
  { // When 5000 data points are collected, print the entire data set in series
    for (int arr = 0; arr < 10; arr++)
    {    for (int i = 0; i < dataPointsPerArray; i++) 
          {     // Print both ADC values on the same line, separated by a space for the Serial Plotter
                  Serial.print(adcData1[arr][i],7);  // Print the first ADC value , 7 decimal pts
                  Serial.print(" ");  // Space separates the two values for Serial Plotter
                  Serial.println(adcData2[arr][i],7);  // Print the second ADC value , 7 decimal pts
          }
    }

    // Reset indices to start collecting new data
    arrayIndex = 0;
    pointIndex = 0;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Free up memory on exit or reset if needed
void freeMemory() {
  for (int i = 0; i < 10; i++) {
    free(adcData1[i]);
    free(adcData2[i]);
  }
}
