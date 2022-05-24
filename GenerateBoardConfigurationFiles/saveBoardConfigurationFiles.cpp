#include <../AsierInho_DLL/src/SerialNumberList.h>
#include <Helper\HelperMethods.h>
#include <stdio.h>
#include <sstream>
int firstIDToGenerate = 1;
int lastIDToGenerate = 35; //Change to fit number of boards in the .h file.

/**
	This program reads the file describing our boards' configuration (SerialNumberList.h)
	and generates the configuration files for the boards. 
	Please note it will re-generate all files in the sequence (from firstIDToGenerate to lastIDToGenerate (included).
*/

void main(void) {
	for (int boardID = firstIDToGenerate; boardID  <= lastIDToGenerate; boardID ++) {
		int indexInSerialNumberFile = boardID - 1;//Board IDs go from 1..N, but the array in which they are stored starts at 0.
		FILE* file;
		std::stringstream filename;
		filename<< "board_" << (boardID) << ".pat";
		fopen_s(&file, filename.str().c_str(), "w");
		//Save hardware ID
		fprintf(file, "%s\n", BensSerialNumbers[indexInSerialNumberFile ]);
		//Save numtransducers
		fprintf(file, "256\n");
		//Save numdiscrete levels
		fprintf(file, "128\n");
		//Save transducers positions:
		for(int j=0; j<16; j++)
			for (int i = 0; i < 16; i++) {
				int t_id[] = { j,i };
				float pos[3];
				computeTransducerPos_SideBySide(t_id, 0.0105f, pos);
				fprintf(file, "(%f, %f, %f),", pos[0], pos[1], pos[2]);
			}
		fprintf(file, "\n");
		//Save transducer -> PIN mapping:
		for (int i = 0; i < 256; i++)
			fprintf(file, "%d,", BensMapping[i]);
		fprintf(file, "\n");
		//Save phase corrections
		for (int i = 0; i < 256; i++)
			fprintf(file, "%d,", BensPhaseAdjust[indexInSerialNumberFile][i]);
		fprintf(file, "\n");
		//Save amplitude corrections
		for (int i = 0; i < 256; i++)
			fprintf(file, "%f,", 1.0f);
		fprintf(file, "\n");
		fclose(file);
	}
}