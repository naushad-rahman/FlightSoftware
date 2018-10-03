#include <Arduino.h>

#ifndef KICKSAT_SENSOR_H_
#define KICKSAT_SENSOR_H_
#define ADC_CS A2
#define ADC_RST A1
#define SD_CS 4

class KickSat_Sensor
{
  public:
    KickSat_Sensor(uint8_t adc_cs, uint8_t adc_rst);
    String cf_list = "";
    void operate(float* dataOut, String boardNum);
    void burstWriteRegs(byte start, uint8_t len, byte* data);
    void startADC();
    void stopADC();
    void resetADC();
    void shutdownADC();
    void wakeADC();
    void regReadout();
    void GPIO(byte pins, byte state);
    void writeReg(byte start, byte value);    
    float readTemp();
    float readPins(byte pinNums, byte idacPin, byte vbPin, int wait, int bufflen, byte idacMag);    
    float hallGen(uint8_t inp, uint8_t inn, byte idacMag, uint8_t idacMux, uint8_t vb, int delayT);    
    float dataConvert( byte a, byte b, byte c);   
    float voltageApplied;
    uint8_t bufflen = 0;
  private:
    uint8_t _ADCchipSelect;
    uint8_t _ADCreset;
    uint8_t _SDchipSelect;
    byte readOut[17];
    const float refV = 2.5;
    const float pgaGain = 1;
    const float FSR = (refV*2)/pgaGain;
    const float LSBsize = FSR/pow(2,24);
};
#endif /* KICKSAT_SENSOR_H_ */