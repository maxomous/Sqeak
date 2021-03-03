
#include "common.hpp"

// - I dont quite understand new - i get that it allocates memory
// 		-is it just used when making a pointer and thats all? if so that makes sense
// - Whats the naming convention for classes? im still using x_t
// - Inlcudes are probably for c std libraries
// - static int grblBufferSize = MAX_GRBL_BUFFER; in grbl.cpp are global, should I do these differently?
// - do i need to destruct gcList_t?
// - dont like how we are handling real time commands in grblRead
int main(int argc, char **argv)
{
	(void)argc, (void) argv;
		
	int wiringPiSetup(void);

	int fd = serialOpen(SERIAL_DEVICE, SERIAL_BAUDRATE);
	if(fd == -1)
		exitf("ERROR: Could not open serial device\n");
	
	serialPuts(fd, "\r\n\r\n");
	delay(2000);
	serialFlush(fd);	
	
	char REALTIME_RESET = 0x18;
	serialPutchar(fd, REALTIME_RESET);
	
	serialPuts(fd, "?\n");
	
	queue_t* q = new queue_t(128);			
	
	gcList_t* gcList = new gcList_t;
	
	gcList->add("$X\n");
	//gcList->add("$$\n");
	gcList->add("G91\n");
	gcList->add("G1 X10 Y20 Z50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X-10 Y-20 Z-50 F6000\n");
	gcList->add("G4 P1\n");
	
	gcList->add("G1 X10 Y20 Z50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X-10 Y-20 Z-50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X10 Y20 Z50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X-10 Y-20 Z-50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X10 Y20 Z50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X-10 Y-20 Z-50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X10 Y20 Z50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X-10 Y-20 Z-50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X10 Y20 Z50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X-10 Y-20 Z-50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X10 Y20 Z50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X-10 Y-20 Z-50 F6000\n");
	gcList->add("G4 P1\n");
	
	/*serialPuts(fd, "$\n");
	serialPuts(fd, "$$\n");

	serialPuts(fd, "$X\n");
	
	serialPuts(fd, "$#\n");*/
	//serialPuts(fd, "G00 X5\n");
	//serialPuts(fd, "G00 y5\n");
	
	//uint requestTime = millis() + 1000;
	
	do {
		grblWrite(fd, gcList, q);
		grblRead(fd, gcList, q, GRBL_ADD_TO_STREAM);
		
		
		
		/*
		
		if(millis() > requestTime) {
			//grblRTCommand();
			printf("sent: ?\n");
			serialPuts(fd, "?\n");
			grblRead(fd, gcList, q, GRBL_DONT_STREAM);
			
			requestTime += 1000;
		}*/
		
	} while (1);
	
	// close serial connection
	serialClose(fd);
	
	return 0;
}
