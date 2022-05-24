#include <AsierInho.h>
#include <AsierInho_CWrapper.h>
#include <Helper\HelperMethods.h>
#include <conio.h>
void print(const char* str) {
	printf("%s\n", str);
}

void createTrap(float position[3], float t_positions[512*3], float* amplitudes, float* phases) {
	float pitch = 0.0105f;
	int buffer_index=0;
	//1. Traverse each transducer:
	for (buffer_index=0; buffer_index<512; buffer_index++)
		{
			//a. Get its position:
			float* t_pos=&(t_positions[3*buffer_index]);
			//b. Compute phase delay and amplitudes from transducer to target point: 
			float distance, amplitude, signature;
			computeAmplitudeAndDistance(t_pos, position, &amplitude, &distance);
			signature = (t_pos>0? (float)M_PI: 0);//Add PI to transducers on top board.
			float phase = fmodf(-K()*distance + signature, (float)(2 * M_PI));

			//c. Store it in the output buffers.
			phases[buffer_index] = phase;
			amplitudes[buffer_index] = amplitude;
		}
}

void main(void) {
	//Create handler and connect to it
	AsierInho_CWrapper_Initialize();
	AsierInho_CWrapper_RegisterPrintFuncs(print, print, print);
	AsierInho_Handler handler = AsierInho_CWrapper_createHandler();	
	while(!AsierInho_CWrapper_connect(handler, AsierInho::BensDesign, 2,4))
		printf("Failed to connect to board.");
	//Read parameters describing your board setup
	float t_positions[512 * 3], amplitudeAdjust[512];
	int phaseAdjust[512], transducerIDs[512], numDiscreteLevels;
	AsierInho_CWrapper_readParameters(handler, t_positions, transducerIDs, phaseAdjust, amplitudeAdjust, &numDiscreteLevels);
	//Program: Create a trap and move it with the keyboard
	float curPos[] = { 0,0,0.1f };
	float phases[512], amplitudes[512];
	unsigned char phases_disc[512], amplitudes_disc[512];
	//a. create a first trap and send it to the board: 
	createTrap(curPos, t_positions,amplitudes, phases);
	AsierInho_CWrapper_discretizePhases(handler, phases, phases_disc);
	AsierInho_CWrapper_updateDiscretePhases(handler, phases_disc);
	printf("\n Place a bead at (%f,%f,%f)\n ", curPos[0], curPos[1], curPos[2]);
	printf("Use keys A-D , W-S and Q-E to move the bead\n");
	printf("Press 'X' to finish the application.\n");

	//b. Main loop (finished when space bar is pressed):
	bool finished = false;
	while (!finished) {
		//Update 3D position from keyboard presses
		switch (getch()) {
			case 'a':curPos[0] += 0.0005f; break;
			case 'd':curPos[0] -= 0.0005f; break;
			case 'w':curPos[1] += 0.0005f; break;
			case 's':curPos[1] -= 0.0005f; break;
			case 'q':curPos[2] += 0.00025f; break;
			case 'e':curPos[2] -= 0.00025f; break;
			case 'X':
			case 'x': 
				finished = true;

		}
		//Create the trap and send to the board:
		createTrap(curPos, t_positions,amplitudes, phases);
		AsierInho_CWrapper_discretizePhases(handler, phases, phases_disc);
		for (int s = 0; s < 16;s++)//Fill FPGA buffers so update is processed directly
			AsierInho_CWrapper_updateDiscretePhases(handler, phases_disc);
	}
	//Deallocate the AsierInho controller: 
	AsierInho_CWrapper_turnTransducersOff(handler);
	AsierInho_CWrapper_disconnect(handler);
	AsierInho_CWrapper_destroyHandler(handler);
	AsierInho_CWrapper_Release();
}