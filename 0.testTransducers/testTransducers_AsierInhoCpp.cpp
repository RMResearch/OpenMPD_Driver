#include <AsierInho.h>
#include <Helper\HelperMethods.h>
#include <conio.h>
#include <stdio.h>
#include <Windows.h>
#include <sstream>


void print(const char* str) {
	printf("%s\n", str);
}

void triggerAll(float* amplitudes, float* phases, float amplitudeCorrections[512], bool removeLoudOnes = true) {
	for (int i = 0; i < 512; i++) {
		bool shut = (removeLoudOnes &&(amplitudeCorrections[i]<0.5f));
		amplitudes[i] = 1.0f - shut;
		phases[i] = 0;
	}	

}

void triggerTransducer(int transducer, float* amplitudes, float* phases, float amplitudeCorrections[512], bool removeLoudOnes=true) {
	for (int i = 0; i < 512; i++) {
		bool shut = (i != transducer)
					|| (removeLoudOnes &&(amplitudeCorrections[i]<0.5f));
		amplitudes[i] = 1.0f - shut;
		phases[i] = 0;
	}	
}

void saveAmplitudeAdjust(int board, float amplitudeAdjust[]) {
	FILE* file;
	std::stringstream filename;
	filename<< "amplitudeAdjust_" << board  << ".pat";
	fopen_s(&file, filename.str().c_str(), "w");
	if (! file)
		printf("Could not open configuration file for ID %d\n", board);
	for (int i = 0; i < 256; i++)
		fprintf(file, "%f,", amplitudeAdjust[i]);
	fclose(file);
}

void main(void) {
	int topBoard = 30, bottomBoard = 32;
	//Create handler and connect to it
	AsierInho::RegisterPrintFuncs(print, print, print);
	AsierInho::AsierInhoBoard* driver = AsierInho::createAsierInho();
	if(!driver->connect(AsierInho::BensDesign, bottomBoard, topBoard))
		printf("Failed to connect to board.");

	float t_positions[512 * 3], amplitudeAdjust[512];
	int phaseAdjust[512], transducerIDs[512], numDiscreteLevels;
	driver->readParameters(t_positions, transducerIDs, phaseAdjust, amplitudeAdjust, &numDiscreteLevels);
	//Program: Create a trap and move it with the keyboard
	int curTransducer = 0;
	float phases[512], amplitudes[512];
	unsigned char phases_disc[512], amplitudes_disc[512];
	//a. create a first trap and send it to the board: 
	for (int s = 0; s < 16;s++)
		driver->turnTransducersOff();
	printf("\n Press 1/2 for PREV/NEXT transducer\n");
	printf("\n Press 3 for +20 transducers (i.e. jump quickly)\n");
	printf("\n Press a/A to turn ALL transducers on (i.e. hear final effect)\n");
	printf("\n Press q/Q to traverse all transducers one-by-one quickly\n");
	printf("\n Press z/Z to add/remove transducer from list.");
	printf("\n Press SPACE for turning REMOVE_LOUD_ONES ON/OFF\n");
	printf("\n Press ENTER to save amplitude adjustments to file\n");
	printf("Press 'X' to finish the application.\n");
	//b. Main loop (finished when space bar is pressed):
	bool finished = false;
	bool removeLoudOnes = true;
	while (!finished) {
		//Update 3D position from keyboard presses
		switch (getch()) {
			case '1':
				curTransducer = (curTransducer+511)%512; 
				triggerTransducer(curTransducer, amplitudes, phases, amplitudeAdjust,removeLoudOnes);
				break;
			case '2':
				curTransducer = (curTransducer+1)%512; 
				triggerTransducer(curTransducer, amplitudes, phases, amplitudeAdjust, removeLoudOnes);
				break;
			case '3': 
				curTransducer = (curTransducer+20)%512; 
				triggerTransducer(curTransducer, amplitudes, phases, amplitudeAdjust, removeLoudOnes);
				break;

			case 'z':
			case 'Z':
				amplitudeAdjust[curTransducer] = 1 - amplitudeAdjust[curTransducer];
				printf("Transducer %d - Status: %d\n", curTransducer, amplitudeAdjust[curTransducer]);
				break;
			case 'a':
			case 'A':
				triggerAll(amplitudes, phases, amplitudeAdjust, removeLoudOnes);
				break;
			case ' ':
				removeLoudOnes = !removeLoudOnes;
				printf("Switch REMOVE_LOUD_ONES: %d\n", removeLoudOnes);
				continue;
				break;//(yeah, I know it's useless).
			case 'X':
			case 'x': 
				finished = true;
				break;

			case '\n':
			case '\r':
				saveAmplitudeAdjust(bottomBoard, amplitudeAdjust);
				saveAmplitudeAdjust(topBoard, &(amplitudeAdjust[256]));
				printf("Current transducer status saved to amplitude files.\n Remember to copy amplitude arrays to your board files.\n");
				break;
			case 'q':
			case 'Q':
				for (int t = 0; t < 512; t++) {
					triggerTransducer(t, amplitudes, phases, amplitudeAdjust, removeLoudOnes);
					driver->discretizePhases(phases, phases_disc);
					driver->discretizeAmplitudes(amplitudes, amplitudes_disc);

					printf("\nTransducer %d (%f,%f,%f); LOUD: %d", t, t_positions[3 * t], t_positions[3 * t + 1], t_positions[3 * t + 2], removeLoudOnes);
					int alternate = 5;
					while (alternate--) {
						//for (int s = 0; s < 16;s++)//Make sure we fill the buffers and the update is processed without delay
						//driver->turnTransducersOff();
						//Sleep(5);
						for (int s = 0; s < 16;s++)//Make sure we fill the buffers and the update is processed without delay
						driver->updateDiscretePhasesAndAmplitudes(phases_disc, amplitudes_disc);
						Sleep(5);
					}
				}
				continue;
				break;

		}
		//Activate the transducer and send to the board:
		driver->discretizePhases( phases, phases_disc);
		driver->discretizeAmplitudes( amplitudes, amplitudes_disc);

		printf("\nTransducer %d (%f,%f,%f); LOUD: %d", curTransducer, t_positions[3 * curTransducer], t_positions[3 * curTransducer + 1], t_positions[3 * curTransducer + 2], removeLoudOnes);
		int alternate = 10;
		while (alternate--) {
			//for (int s = 0; s < 16;s++)//Make sure we fill the buffers and the update is processed without delay
			//	driver->turnTransducersOff();
			//	Sleep(25);
			for (int s = 0; s < 16;s++)//Make sure we fill the buffers and the update is processed without delay
				driver->updateDiscretePhasesAndAmplitudes(phases_disc, amplitudes_disc);
				Sleep(25);
		}		
	}
	//Deallocate the AsierInho controller: 
	for (int s = 0; s < 16;s++)
		driver->turnTransducersOff();
	driver->disconnect();
	delete driver;	
}