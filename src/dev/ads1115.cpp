/*
 * ADS1115.cpp
 */

#include <iostream>
#include <bitset> 	// to view binary in terminal
#include <mutex>	// threads

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "ads1115.h"

using namespace std;

// Internal
#define CONVERSION_REG			(int)0x00				// ADC Registers
#define CONFIG_REG				(int)0x01
#define RESET_CONVERSION_REG		(int)0x0000				// ADC Register reset vals
#define RESET_CONFIG_REG			(int)0x8583				// 1000 0101 1000 0011
#define ADC_CONFIG_MASK			(int)0x8003				// basic format 
#define OS_BIT					(int)0x8000				// used to check when new value is ready in conversion reg when using single shot mode
// Full scale range in bits
#define FSRbits 				(int)((2 << 14) -1)		// 16 bit controller but first bit is '-'   so 2^15 - 1
// Full scale voltage range
#define FSR_6_144				(int)0x0000				//	0000 : FSR = ±6.144 V (our default)
#define FSR_4_096				(int)0x0200				//	0010 : FSR = ±4.096 V
#define FSR_2_048				(int)0x0400				//	0100 : FSR = ±2.048 V (ADS1115 default)
#define FSR_1_024				(int)0x0600				//	0110 : FSR = ±1.024 V
#define FSR_0_512				(int)0x0800				//	1000 : FSR = ±0.512 V
#define FSR_0_256				(int)0x0A00				//	1010 : FSR = ±0.256 V
//#define FSR_0_256				(int)0x0C00				//	1100 : FSR = ±0.256 V  -  Unused - This also corrolate to +=0.256V
//#define FSR_0_256				(int)0x0E00				//	1110 : FSR = ±0.256 V  -  Unused - TThis also corrolate to +=0.256V


// Init ADC I2C 
void ADS1115::Init(int addr) 
{
	{    // lock the mutex
		std::lock_guard<std::mutex> gaurd(m_mutex);
		
		fd = wiringPiI2CSetup(addr);
		if(fd < 0) {
			cout << "Error: Couldn't connect to I2C address " << addr << endl;
			cout << "Hint: To find I2C address, use command 'sudo i2cdetect -y 1'" << endl;
			return;
		}
	}
	//dummy read
	Read(ADS1115_PIN_A0);
}

void ADS1115::ConfigureFSRVolts(float val) 
{
	// lock the mutex
	std::lock_guard<std::mutex> gaurd(m_mutex);
	
	if(val == ADS1115_FSR_6_144V) {
		config_FSRvolts = val;
		config_FSR 		= FSR_6_144;
	}
	else if(val == ADS1115_FSR_4_096V) {
		config_FSRvolts = val;
		config_FSR 		= FSR_4_096;
	}
	else if(val == ADS1115_FSR_2_048V) {
		config_FSRvolts = val;
		config_FSR 		= FSR_2_048;
	}
	else if(val == ADS1115_FSR_1_024V) {
		config_FSRvolts = val;
		config_FSR 		= FSR_1_024;
	}
	else if(val == ADS1115_FSR_0_512V) {
		config_FSRvolts = val;
		config_FSR 		= FSR_0_512;
	}
	else if(val == ADS1115_FSR_0_256V) {
		config_FSRvolts = val;
		config_FSR 		= FSR_0_256;
	}
	else {
		cout << "Error: ADS1115 FSR Volts " << val << " unrecognised" << endl;
		return;
	}
	volts_Per_Bits = (double)config_FSRvolts / (double)FSRbits;
}

void ADS1115::ConfigureComparitor(int val) 
{
	// lock the mutex
	std::lock_guard<std::mutex> gaurd(m_mutex);
	
	if(val == ADS1115_COMPARITOR_USE) {
		cout << "Error: Multiplex comparitor has not yet been coded, bits 4-0 will require ammending - see datasheet, shouldn't be too much work" << endl;
		return;
	}
	if(val != ADS1115_COMPARITOR_USE  &&  val != ADS1115_COMPARITOR_DONT_USE)
		cout << "Error: ADS1115 comparitor value " << val << " unrecognised" << endl;
	else
		config_Comparitor = val;
}

void ADS1115::ConfigureMode(int val) 
{
	// lock the mutex
	std::lock_guard<std::mutex> gaurd(m_mutex);
	
	if(val != ADS1115_MODE_SINGLE_SHOT  &&  val != ADS1115_MODE_CONTINUOUS)
		cout << "Error: ADS1115 mode " << val << " unrecognised" << endl;
	else
		config_Mode = val;
}

void ADS1115::ConfigureSampleTime(int val) 
{
	// lock the mutex
	std::lock_guard<std::mutex> gaurd(m_mutex);
	
	if(val != ADS1115_SAMPLE_TIME_8SPS  &&  val != ADS1115_SAMPLE_TIME_16SPS  &&  val != ADS1115_SAMPLE_TIME_32SPS  &&  val != ADS1115_SAMPLE_TIME_64SPS &&
	val != ADS1115_SAMPLE_TIME_128SPS  &&  val != ADS1115_SAMPLE_TIME_250SPS  &&  val != ADS1115_SAMPLE_TIME_475SPS  &&  val != ADS1115_SAMPLE_TIME_860SPS)
		cout << "Error: ADS1115 sample time " << val << " unrecognised" << endl;
	else
		config_SampleTime = val;
}

double ADS1115::Read(int input)
{
	// lock the mutex
	std::lock_guard<std::mutex> gaurd(m_mutex);
	
	//Reset config reg
	ResetRegister(CONFIG_REG);
	delay(10);
	//Select ADC input
	if(SelectInput(input))
		return 0;	// return if error
	delay(5);
	//Reset conversion reg
	ResetRegister(CONVERSION_REG);
	delay(10);
	//Retrieve ADC input voltage
	return GetVoltage();
}

//************* ALL FUNCTIONS BELOW HERE SHOULD ALREADY HAVE MUTEX LOCKED! *************//

void ADS1115::ResetRegister(unsigned int Register)
{
	if(Register == CONFIG_REG)
		wiringPiI2CWriteReg16(fd, CONFIG_REG, RESET_CONFIG_REG);
	else if (Register == CONVERSION_REG)
		wiringPiI2CWriteReg16(fd, CONVERSION_REG, RESET_CONVERSION_REG);
}
	
int ADS1115::SelectInput (unsigned int adcInput)
{
	int data = (ADC_CONFIG_MASK | config_Comparitor | config_FSR | config_Mode | config_SampleTime | adcInput);
	// swap 1st and 2nd 16 bits
	int dout = ((data >> 8) & 0x00FF) | ((data << 8) & 0xFF00);
	/*
	bitset<16> bits = data;
	cout << "[15] OS Bit  |  [14-12] Multiplex(Comparitor)  |  [11-9] Gain  |  [8] Mode" << endl;
	cout << "sending to config reg: " << bits << endl;
	cout << "[7-5] SampleRate  |  [4] Comparitor Mode  |  [3] Comparitor Polarity  |  [2] Latching Comparitor  |  [1-0] N/A" << endl << endl;
	*/
	wiringPiI2CWriteReg16(fd, CONFIG_REG, dout);
	//check that the correct input has been selected	(3rd & 4th bit)
	if ((ReadRegister(CONFIG_REG) & 0x3000) != adcInput) {
		cout << "Error: ADC input value is incorrect" << endl;
		return 1;
	}
	return 0;
	
}
int ADS1115::ReadRegister(unsigned int Register)
{
	auto read = [this](uint Register) 
	{
		int din = wiringPiI2CReadReg16(fd, Register);
		// swap the 1st 2-bytes with the 2nd 2-bytes
		return ((din >> 8) & 0x00FF) | ((din << 8) & 0xFF00);
	};
	auto twosComplement = [](int& data) 
	{
		if(data & 0x8000) {			
			data ^= 0x8000;
			data = -data;
		}	
	};
	int data;
	
	if(Register == CONFIG_REG)
		data = read(Register);
	else if(Register == CONVERSION_REG && config_Mode == ADS1115_MODE_CONTINUOUS) {
		data = read(Register);
		twosComplement(data);	
	}
	else if (Register == CONVERSION_REG) {
		do {
			data = read(Register);
			// wait until OS bit has been set to 1
		} while (!(ReadRegister(CONFIG_REG) & OS_BIT));
		twosComplement(data);
	}	
	return data;
}

double ADS1115::GetVoltage()
{
	return (double)ReadRegister(CONVERSION_REG) * volts_Per_Bits;	
}
