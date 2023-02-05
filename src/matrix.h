#pragma once

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <string>
#include <map>
#include <list>
#include <functional>

#include <QPoint>

#define UP 16
#define DOWN -16
#define LEFT -1
#define RIGHT 1

class Matrix {
	public:
		Matrix();

		void Initialize(int seed);
		void ClearAll();

		void Generate(int seed);
		void Shuffle(int seed);

		void CalculateEdgeLimit();
		void CalculateConnections();

		// returns a vector with the indecies that the connection is going to pass through
		std::vector<int> Connect(int startIndex, int targetIndex);
		// converts an index to a QPoint	
		QPoint IndexToPoint(int index) const;
		// returns a vector with all the indices that are on the same X column as index
		std::vector<int> GetXColumn(int index) const;
		// returns a vector with all the indices that are on the same Y column as index
		std::vector<int> GetYColumn(int index) const;
		// pretty obvious
		std::vector<int> GetConnectionData(int startIndex, int targetIndex);

		void Update(int startIndex, int targetIndex, int seed);
		void UpdateLevel(int startIndex, int targetIndex);
		void UpdateData(int seed);

		// shift the matrix at index
		void Shift(int index, int directionIndex, bool split);
		// checks if an index is on edge
		bool IsIndexOnEdge(int index, int directionIndex) const;
		bool IsIndexOutOfBounds(int index) const;
		// checks if a connection is valid
		bool IsConnectionValid(int startIndex, int targetIndex);
		// store each cells values
		int data[144] { 0 };
		
		// should be obvious what those are
		std::vector<std::vector<int>> indicesToEdge;
		std::list<std::vector<int>> connectionPaths;

		std::map<int, int> directionOffsets;
		
		int currentLevel;
		int numConnectedIndices;

		// all the possible values for a tile, not sure if their names are 100% correct
		// but i tried
		enum Tiles {
			NONE, 		BUTTER_FREE,
			FLAREON, 	TOGETIC,
			AMPHAROS, 	PSYDUCK,
			MEW, 		GROWLITHE,
			HORSEA, 	MARILL,
			NIDORAN, 	BULBASAUR,
			PIDGEY, 	SEEL,
			TOTODILE, 	POLITOED,
			MILTANK, 	GLIGAR,
			PONYTA, 	OMANYTE,
			CHIKORITA, 	CHARMANDER, 
			CLEFFA, 	AIPOM,
			LEDYBA, 	CORSOLA,
			SQUIRTLE, 	MAREEP,
			DELIBIRD, 	POLYWHIRL,
			VAPOREON, 	BEEDRILL,
			NATU, 		CATERPIE,
			MEOWTH, 	PIKACHU
		};		
};
