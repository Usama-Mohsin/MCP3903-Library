// To run this code you need Parameters library (available on my Github Profile)

#include <SPI.h>                    //SPI Library (built in)
#include <MCP3903.h>                //ADC Sensor Library
#include <Parameters.h>             //For Electrical Parameters Calculation
#include <math.h>                   //For using Cos in PF calculation         


/////////////////////////////////////////         MCP3903          /////////////////////////////////////////////////////////////////
MCP3903 adc; // create an instance

// SPI Pins for MCP3903
#define SCK_PIN 18  
#define MISO_PIN 19 
#define MOSI_PIN 23 
#define CS_PIN 5    


/////////////////////////////////////////         Calculations Variables          /////////////////////////////////////////////////
double* Voltage1; // Pointer for ADC channel 0 data
double* Voltage2; // Pointer for ADC channel 2 data
double* Voltage3; // Pointer for ADC channel 4 data
double* Current1; // Pointer for ADC channel 1 data
double* Current2; // Pointer for ADC channel 3 data
double* Current3; // Pointer for ADC channel 5 data

double result_ch0 , result_ch1 , result_ch2 ,result_ch3 ,result_ch4 ,result_ch5;
const int DataPoints = 1563; // Total number of points for all channels
int pointIndex = 0; // Current point index
int precision = 4; // Precision for printing
int Sig_F =50;
int OSR=256;
//PHASE 1
double V_callibration_1=1014.14;
double I_callibration_1=2*15.25697541;
int Phase_index_1=6;
//PHASE 2
double V_callibration_2=1014.14;
double I_callibration_2=2*15.25697541;
int Phase_index_2=6;
//PHASE 3
double V_callibration_3=1014.14;
double I_callibration_3=2*15.25697541;
int Phase_index_3=6;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////           SETUP               /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() 
{
  Serial.begin(115200);  // Initialize serial communication
  delay(1000);  // Small delay to stabilize serial connection

  // Allocate memory for arrays
  Voltage1 = (double*)malloc(DataPoints * sizeof(double));
  Current1 = (double*)malloc(DataPoints * sizeof(double));
  Voltage2 = (double*)malloc(DataPoints * sizeof(double));
  Current2 = (double*)malloc(DataPoints * sizeof(double));
  Voltage3 = (double*)malloc(DataPoints * sizeof(double));
  Current3 = (double*)malloc(DataPoints * sizeof(double));

  // Safety check for memory allocation
  if (Voltage1 == nullptr || Current1 == nullptr || 
      Voltage2 == nullptr || Current2 == nullptr || 
      Voltage3 == nullptr || Current3 == nullptr) 
  {
    Serial.println("ERROR: Memory allocation failed!");
    while (1); // Halt execution
  }

  // Initialize MCP3903
  Serial.println("Initializing MCP3903...");
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  adc.reset();
  adc.init_config(MCP3903::o256, MCP3903::p1, 0, 0, 0, 1, 1, 1, 1, 1, 1);
  adc.init_status(MCP3903::type, 24, 0, MCP3903::lag, MCP3903::lag, MCP3903::lag);
  //adc.Gain(2, MCP3903::g1);   
  //adc.Gain(3, MCP3903::g1);
  //adc.phase('a', 0);
  Serial.println("MCP3903 Initialized.");

  Serial.println("System Ready. Please wait...");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////           LOOP               /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() 
{ ///////////////////////////////////////////Pts Collection///////////////////////////
  if (pointIndex < DataPoints) 
  { adc.read6ADC(result_ch0, result_ch1, result_ch2, result_ch3, result_ch4, result_ch5);
    //PHASE 1
    Voltage1[pointIndex] = ((result_ch0)*V_callibration_1); 
    Current1[pointIndex] = ((result_ch1)*I_callibration_1); 
    //PHASE 2
    Voltage2[pointIndex] = ((result_ch2)*V_callibration_2); 
    Current2[pointIndex] = ((result_ch3)*I_callibration_2);
    //PHASE 3
    Voltage3[pointIndex] = ((result_ch4)*V_callibration_3); 
    Current3[pointIndex] = ((result_ch5)*I_callibration_3);
    pointIndex++;
  }
  ///////////////////////////////////////////Calculation and Storing/////////////////
  else 
  { ////////////////// Calculation /////////////////////
    double Vrms1 = Parameters::RMS(Voltage1, DataPoints);    
    double Irms1 = Parameters::RMS(Current1, DataPoints);  
    double S1    = Vrms1*Irms1;      
    double freq1 = Parameters::Frequency(Voltage1,DataPoints,(1000000/OSR));
    double P1=Parameters::RealPower(Voltage1,Current1, DataPoints,Phase_index_1);
    double angle1=acos(P1/S1);
    double pf1   = cos(angle1);
    if(S1<P1){pf1=1;}

    double Vrms2 = Parameters::RMS(Voltage2, DataPoints);    
    double Irms2 = Parameters::RMS(Current2, DataPoints);  
    double S2    = Vrms2*Irms2;      
    double freq2 = Parameters::Frequency(Voltage2,DataPoints,(1000000/OSR));
    double P2=Parameters::RealPower(Voltage2,Current2, DataPoints,Phase_index_2);
    double angle2=acos(P2/S2);
    double pf2   = cos(angle2);
    if(S2<P2){pf2=1;}

    double Vrms3 = Parameters::RMS(Voltage3, DataPoints);    
    double Irms3 = Parameters::RMS(Current3, DataPoints);  
    double S3    = Vrms3*Irms3;      
    double freq3 = Parameters::Frequency(Voltage3,DataPoints,(1000000/OSR));
    double P3=Parameters::RealPower(Voltage3,Current3, DataPoints,Phase_index_3);
    double angle3=acos(P3/S3);
    double pf3   = cos(angle3);
    if(S3<P3){pf3=1;}


    ////////////////// Safety Catches /////////////////////
    //phase 1
    if (Vrms1<50.0)
    {
      Vrms1=0.0; S1=0.0; P1=0.0; pf1=1.0;  freq1=0.0;
    }
    if (Irms1<0.1)
    {
      Irms1=0.0;  S1=0.0; P1=0.0; pf1=1.0; 
    }

    //phase 2
    if (Vrms2<50.0)
    {
      Vrms2=0.0; S2=0.0; P2=0.0; pf2=1.0; freq2=0.0;
    }
    if (Irms2<0.1)
    {
      Irms2=0.0;  S2=0.0; P2=0.0; pf2=1.0; 
    }

    //phase 3
    if (Vrms3<50.0)
    {
      Vrms3=0.0; S3=0.0; P3=0.0; pf3=1.0;  freq3=0.0;
    }
    if (Irms3<0.1)
    {
      Irms3=0.0;  S3=0.0; P3=0.0; pf3=1.0; 
    }
    
    ////////////////// Printing// /////////////////////
   
    Serial.println("======= Phase 1 Parameters =======");
    Serial.print("Vrms1 : "); Serial.println(Vrms1, precision);
    Serial.print("Irms1 : "); Serial.println(Irms1, precision);
    Serial.print("Apparent Power (S1) : "); Serial.println(S1, precision);
    Serial.print("Real Power (P1) : "); Serial.println(P1, precision);
    Serial.print("Frequency (Hz) : "); Serial.println(freq1, precision);
    Serial.print("Power Factor (pf1) : "); Serial.println(pf1, precision);
    Serial.print("Phase Angle (rad) : "); Serial.println(angle1, precision);
    Serial.println();

    Serial.println("======= Phase 2 Parameters =======");
    Serial.print("Vrms2 : "); Serial.println(Vrms2, precision);
    Serial.print("Irms2 : "); Serial.println(Irms2, precision);
    Serial.print("Apparent Power (S2) : "); Serial.println(S2, precision);
    Serial.print("Real Power (P2) : "); Serial.println(P2, precision);
    Serial.print("Frequency (Hz) : "); Serial.println(freq2, precision);
    Serial.print("Power Factor (pf2) : "); Serial.println(pf2, precision);
    Serial.print("Phase Angle (rad) : "); Serial.println(angle2, precision);
    Serial.println();

    Serial.println("======= Phase 3 Parameters =======");
    Serial.print("Vrms3 : "); Serial.println(Vrms3, precision);
    Serial.print("Irms3 : "); Serial.println(Irms3, precision);
    Serial.print("Apparent Power (S3) : "); Serial.println(S3, precision);
    Serial.print("Real Power (P3) : "); Serial.println(P3, precision);
    Serial.print("Frequency (Hz) : "); Serial.println(freq3, precision);
    Serial.print("Power Factor (pf3) : "); Serial.println(pf3, precision);
    Serial.print("Phase Angle (rad) : "); Serial.println(angle3, precision);
    Serial.println();



    pointIndex = 0;  //////////Start Again

  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void freeMemory() 
{  
  free(Voltage1);
  free(Current1);
  free(Voltage2);
  free(Current2);
  free(Voltage3);
  free(Current3);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
