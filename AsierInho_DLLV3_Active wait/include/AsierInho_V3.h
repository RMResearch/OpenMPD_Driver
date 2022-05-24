#ifndef _ASIER_INHO_V3
#define _ASIER_INHO_V3
#include <AsierInho_V3_Prerequisites.h>
/**
This project contains a simple driver to communicate with out Acoustophoretic board (our PAT).
This provides direct control over each transducer’s phase and amplitude, as well as access to
our MATD mode (high speed computation of single levitation traps, with amplitude and illumination control).
The module is encapsulated into a DLL, which you can use through interfaces in either C++ or C. 
This namespace contains a pure virtual interface (AsierInhoBoard), which cleints can use to control the boards.
It also contains an abstract factory, to allow creation of board controllers (AsierInhoBoards)
without exposing implementation details; as well as other utility functions for the DLL to print
notification, warning and error messages. 
*/
namespace AsierInho_V3 {
	class _AsierInho_Export_V3 AsierInhoBoard_V3 {
	protected:
		/**
			AsierInho's cannot be created directly, but you should use the method:
				AsierInho::createAsierInho(BoardType boardType, int bottomBoardID, int topBoardID)
			This allows separating interface from implementation and should allow modifications to the DLL without breaking clients.
		*/
		AsierInhoBoard_V3() { ; }

	public:
		/**
			Virtual destructor (required so that derived classes are deleted properly)
		*/
		virtual ~AsierInhoBoard_V3() { ; }

		//PUBLIC METHODS USED BY CLIENTS:

		/**
			Connects to the boards, using the board IDs specified.
			ID corresponds to the index we labeled on each board.
			Returns NULL if connection failed.
		*/
		virtual bool connect(int bottomBoardID, int topBoardID = 0) = 0;
		virtual bool connect(int numBoards, int* boardIDs, float* matBoardToWorld4x4) = 0;
		
		/**
			DEBUG VERSION: Reads the parameters required to use the boards (transducer positions, phase corrections, etc). 
			This method allows you to read the information required by GS_PAT to compute solutions, and create the messages to send to the board.			
			NOTE: Asumes the second board is on top of the first one (23.8cm)
		*/
		virtual void readParameters(float* transducerPositions, int* transducerIds, int* phaseAdjust, float* amplitudeAdjust, int* numDiscreteLevels)=0;

		//RUN-TIME METHODS: 
		/**
			The method sends these messages to the boards (512B for bottom board, 512B for top)xnumMessages. These are already properly formattted (e.g. first byte in each package is set to 128 + phase[0]).
			NOTE: The GPU solver (GS-PAT) directly discretises and formats the messages, so clients using GS-PAT only need to call this method
			(but not methods to discretise, etc.).
		*/
		virtual void updateMessage(unsigned char* message) = 0;

		/**
			This method turns the transducers off (so that the board does not heat up/die misserably)
			The board is still connected, so it can later be used again (e.g. create new traps)
		*/
		virtual void turnTransducersOn() = 0;
		virtual void turnTransducersOff() = 0;
		/**
			This method disconnects the board, closing the ports.
		*/
		virtual void disconnect() = 0;

# ifdef _TIME_PROFILING
		virtual void _profileTimes() = 0;
# endif
		};
	
	_AsierInho_Export_V3 AsierInhoBoard_V3* createAsierInho();

	void printMessage(const char*);
	void printError(const char*);
	void printWarning(const char*);

	_AsierInho_Export_V3 void RegisterPrintFuncs(void(*p_Message)(const char*), void(*p_Warning)(const char*), void(*p_Error)(const char*));
};


#endif
