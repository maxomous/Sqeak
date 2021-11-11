/*
 * adc1115.cpp
 */

#pragma once

#include <mutex>

// ADC pins
#define ADS1115_PIN_A0						(int)0x0000
#define ADS1115_PIN_A1 						(int)0x1000
#define ADS1115_PIN_A2 						(int)0x2000
#define ADS1115_PIN_A3 						(int)0x3000

// The multiplexer allows you to select 2 differential 
// values between 2 pins or 4 individual values
// this has not bee coded yet, pins 4-0 will require values
#define ADS1115_COMPARITOR_USE				(int)0x0000		//0000 (ads1115 default)
#define ADS1115_COMPARITOR_DONT_USE			(int)0x4000		//0100 (our default)

// Mode
#define ADS1115_MODE_CONTINUOUS 			(int)0x0000		//	Continuous Conversion mode - do while loop in ADC_ReadRegister must be commented out)
#define ADS1115_MODE_SINGLE_SHOT 			(int)0x0100		//	Single-Shot mode (goes into powerdown state when not in use)

// Sample time
#define ADS1115_SAMPLE_TIME_8SPS 			(int)0x0000
#define ADS1115_SAMPLE_TIME_16SPS 			(int)0x0020
#define ADS1115_SAMPLE_TIME_32SPS 			(int)0x0040
#define ADS1115_SAMPLE_TIME_64SPS 			(int)0x0060
#define ADS1115_SAMPLE_TIME_128SPS 			(int)0x0080		//	(default)
#define ADS1115_SAMPLE_TIME_250SPS 			(int)0x00A0
#define ADS1115_SAMPLE_TIME_475SPS 			(int)0x00C0
#define ADS1115_SAMPLE_TIME_860SPS 			(int)0x00E0

// Full scale range in volts
#define ADS1115_FSR_6_144V				6.144f		//	0000 : FSR = ±6.144 V (our default)
#define ADS1115_FSR_4_096V				4.096f		//	0010 : FSR = ±4.096 V
#define ADS1115_FSR_2_048V				2.048f		//	0100 : FSR = ±2.048 V (ADS1115 default)
#define ADS1115_FSR_1_024V				1.024f		//	0110 : FSR = ±1.024 V
#define ADS1115_FSR_0_512V				0.512f		//	1000 : FSR = ±0.512 V
#define ADS1115_FSR_0_256V				0.256f		//	1010 : FSR = ±0.256 V


class ADS1115 {
public:
	ADS1115(int addr, float fsr = ADS1115_FSR_6_144V, int mode = ADS1115_MODE_SINGLE_SHOT, int comparitor = ADS1115_COMPARITOR_DONT_USE, int sampleTime = ADS1115_SAMPLE_TIME_128SPS) {
		Init(addr);
		ConfigureFSRVolts(fsr);
		ConfigureMode(mode);
		ConfigureComparitor(comparitor);
		ConfigureSampleTime(sampleTime);
	}
	void Init(int addr);
	double Read(int input);
	
	void ConfigureFSRVolts(float val);
	void ConfigureMode(int val);
	void ConfigureComparitor(int val);
	void ConfigureSampleTime(int val);
	
private:
	std::mutex m_mutex;
	int fd = -1;
	float config_FSRvolts;	// set in ConfigureFSRVolts
	int config_FSR;			// set in ConfigureFSRVolts
	double volts_Per_Bits;	// set in ConfigureFSRVolts
	int config_Comparitor;
	int config_Mode;
	int config_SampleTime;
	
	void ResetRegister(unsigned int Register);
	int SelectInput (unsigned int input);
	int ReadRegister(unsigned int Register);
	double GetVoltage();

};
