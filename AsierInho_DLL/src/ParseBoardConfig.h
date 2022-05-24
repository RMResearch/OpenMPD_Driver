#pragma once
#include <stdio.h>
#include <sstream>

struct BoardConfig {
	char hardwareID[20];
	int numTransducers;
	int numDiscreteLevels;
	float positions[256 * 3];
	int pinMapping[256];
	int phaseAdjust[256];
	float amplitudeAdjust[256];
};

class ParseBoardConfig {
	//Clauses: 
	static bool S(int boardID, FILE* file, struct BoardConfig& parameters){
	if (!HardwareID(file, parameters)) {
		printf("Error parsing hardware ID for board %d\n", boardID);
		return false;
	}
	if (! NumTransducers (file, parameters)){
		printf("Error parsing number of transducers in board %d\n", boardID);
		return false;
	}
	if (! NumDiscreteLevels(file, parameters)){
		printf("Error parsing number of discrete levels used for board %d\n", boardID);
		return false;
	}
	if (! TransducerPositions (file, parameters)){
		printf("Error parsing transducer positions for board %d\n", boardID);
		return false;
	}
	if (! PINMapping (file, parameters)){
		printf("Error parsing PIN mapping for board %d\n", boardID);
		return false;
	}
	if (! PhaseCorrection (file, parameters)){
		printf("Error parsing phase corrections for board %d\n", boardID);
		return false;
	}
	if (! AmplitudeCorrection(file, parameters)){
		printf("Error parsing amplitude corrections for board %d\n", boardID);
		return false;
	}

	return feof(file);
}
	static bool HardwareID(FILE* file, struct BoardConfig& parameters){
		return (fscanf(file, "%s\n", parameters.hardwareID)!=0);
	}
	static bool NumTransducers(FILE* file, struct BoardConfig& parameters){
		return (fscanf(file, "%d\n", &(parameters.numTransducers))!=0);	
	}
	static bool NumDiscreteLevels(FILE* file, struct BoardConfig& parameters){
		return (fscanf(file, "%d\n", &(parameters.numDiscreteLevels))!=0);	
	}
	static bool TransducerPositions(FILE* file, struct BoardConfig& parameters){
		for (int t = 0; t < parameters.numTransducers; t++) 
			if(fscanf(file, "(%f,%f,%f),", &(parameters.positions[3 * t]), &(parameters.positions[3 * t+1]), &(parameters.positions[3 * t+2]))==0)
				printf("Error reading transducer position %d\n", t);

		return (fscanf(file, "\n") != EOF);
		//return true;
	}
	static bool PINMapping(FILE* file, struct BoardConfig& parameters){
		for (int t = 0; t < parameters.numTransducers; t++) 
			if(fscanf(file, "%d,", &(parameters.pinMapping[t]))==0)
				printf("Error reading transducer PIN mapping %d\n", t);
		return (fscanf(file, "\n") != EOF);
		//return true;
	}
	static bool PhaseCorrection(FILE* file, struct BoardConfig& parameters){
		for (int t = 0; t < parameters.numTransducers; t++) 
			if(fscanf(file, "%d,", &(parameters.phaseAdjust[t]))==0)
				printf("Error reading transducer phase adjustment %d\n", t);
		return (fscanf(file, "\n") != EOF);
		//return true;
	}
	static bool AmplitudeCorrection(FILE* file, struct BoardConfig& parameters){
		for (int t = 0; t < parameters.numTransducers; t++) 
			if(fscanf(file, "%f,", &(parameters.amplitudeAdjust[t]))==0)
				printf("Error reading transducer amplitude adjustment %d\n", t);
		return (fscanf(file, "\n") != EOF);
		//return true;
	}
public: 
	static BoardConfig readParameters(int boardID) {
		FILE* file;
		std::stringstream filename;
		filename<< "board_" << boardID  << ".pat";
		fopen_s(&file, filename.str().c_str(), "r");
		if (! file)
			printf("Could not open configuration file for ID %d\n", boardID);
		//Parse:
		struct BoardConfig parameters;
		if (! S(boardID, file, parameters))
			printf("Error parsing configuration for ID %d\n", boardID);
		return parameters;
	}

	static void saveParameters(int boardID, BoardConfig config, const char* path="./") {
		FILE* file;
		std::stringstream filename;
		filename<< path<<"board_" << boardID  << ".pat";
		fopen_s(&file, filename.str().c_str(), "w");
		if (! file)
			printf("Could not open configuration file for ID %d.\n Maybe the path did not exist or the file is already open.\n", boardID);
		
		//Save hardware ID
		fprintf(file, "%s\n", config.hardwareID);
		//Save numtransducers
		fprintf(file, "%d\n", config.numTransducers);
		//Save numdiscrete levels
		fprintf(file, "%d\n", config.numDiscreteLevels);
		//Save transducers positions:
		for (int t = 0; t < config.numTransducers;t++) {
				fprintf(file, "(%f, %f, %f),", config.positions[3*t], config.positions[3*t+1], config.positions[3*t+2]);
			}
		fprintf(file, "\n");
		//Save transducer -> PIN mapping:
		for (int t = 0; t < config.numTransducers; t++)
			fprintf(file, "%d,", config.pinMapping[t]);
		fprintf(file, "\n");
		//Save phase corrections
		for (int t = 0; t < config.numTransducers; t++)
			fprintf(file, "%d,", config.phaseAdjust[t]);
		fprintf(file, "\n");
		//Save amplitude corrections
		for (int t = 0; t < config.numTransducers; t++)
			fprintf(file, "%f,", config.amplitudeAdjust[t]);
		fprintf(file, "\n");
		fclose(file);
	}

};


