#include <AsierInho.h>
#include <stdio.h>
#include <conio.h>
void print(const char* str) {
	printf("%s\n", str);
}

void main(void) {
	//Create handler and connect to it
	AsierInho::RegisterPrintFuncs(print, print, print);
	AsierInho::AsierInhoBoard* driver = AsierInho::createAsierInho();
	while(!driver->connect(AsierInho::BensDesign, 26,24))
		printf("Failed to connect to board.");
	driver->modeReset(AsierInho::MATD);
	//Program: Create a trap and move it with the keyboard
	AsierInho::vec3 curPos( 0,0,0.1f );
	AsierInho::vec3 rgb(1, 1, 0);
	//a. create a first trap and send it to the board: 
	driver->updateFrame(curPos, rgb, 1, 0, true);
	printf("\n Place a bead at (%f,%f,%f)\n ", curPos.x, curPos.y, curPos.z);
	printf("Use keys A-D , W-S and Q-E to move the bead\n");
	printf("Press 'X' to finish the application.\n");

	//b. Main loop (finished when space bar is pressed):
	bool finished = false;
	while (!finished) {
		//Update 3D position from keyboard presses
		switch (getch()) {
			case 'a':curPos.x += 0.0005f; break;
			case 'd':curPos.x -= 0.0005f; break;
			case 'w':curPos.y += 0.0005f; break;
			case 's':curPos.y -= 0.0005f; break;
			case 'q':curPos.z += 0.00025f; break;
			case 'e':curPos.z -= 0.00025f; break;
			case 'X':
			case 'x': 
				finished = true;

		}
		//Create the trap and send to the board:
		driver->updateFrame(curPos, rgb, 1, 0, true);
		
	}
	//Deallocate the AsierInho controller: 
	driver->modeReset(AsierInho::NORMAL);
	driver->turnTransducersOff();//This command is not available in MATD mode.
	driver->disconnect();
	delete driver;	
}