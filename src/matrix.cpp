#include "matrix.h"

Matrix::Matrix() {
	this->currentLevel = 0;
	this->numConnectedIndices = 0;

	this->directionOffsets = {
		{ 0, UP }, { 1, DOWN }, { 2, LEFT }, { 3, RIGHT }
	};

	this->CalculateEdgeLimit();
}

/*
 * 	CORE
 */

void Matrix::Initialize(int seed) {
	this->numConnectedIndices = 0;

	this->Generate(seed);
	this->Shuffle(seed);
	this->CalculateConnections();
}

void Matrix::ClearAll() {
	this->currentLevel = 0;
	this->numConnectedIndices = 0;

	this->connectionPaths.clear();
	// we don't need to reset all the tile values, but this makes me feel better so why not
	for (int i = 0; i < 144; i ++)
		this->data[i] = this->Tiles::NONE;
}

void Matrix::Generate(int seed) {
	// i didn't realized how hard it is to get a random number in c++, holy shit
	std::uniform_int_distribution<int> distribution(1, 35);
	std::default_random_engine generator;

	generator.seed(seed);
	
	// with each possible tile value, we wanna have 2 pairs of them
	// this of course means there are 4 left undistributed, so we take care of them
	// under there
	int index = 0;
	for (int i = this->Tiles::BUTTER_FREE; i <= this->Tiles::PIKACHU; i ++) {
		for (int j = 0; j < 4; j ++) {
			this->data[index] = i;
			index ++;
		}
	}

	int remainderValue = distribution(generator);	
	for (int i = 0; i < 4; i ++) {
		this->data[index] = remainderValue;
		index ++;
	}
}

void Matrix::Shuffle(int seed) {
	std::uniform_int_distribution<int> distribution(0, 143);
	std::default_random_engine generator;

	generator.seed(seed);

	for (int i = 0; i < 144; i ++) {
		int& value = this->data[i];

		if (value > 0) {
			while (true) {
				// generate a random index, grab it's value
				// and then swap it if it isn't empty
				int index = distribution(generator);
				int& targetValue = this->data[index];

				if (targetValue > 0 && value != targetValue) {
					std::swap(value, targetValue);
					break;
				}	
			}
		}
	}
}

void Matrix::CalculateEdgeLimit() {
	for (int y = 0; y < 9; y ++) {
		for (int x = 0; x < 16; x ++) {
			
			int numNorth = 8 - y;
			int numSouth = y;
			int numWest = x;
			int numEast = 15 - x;

			this->indicesToEdge.push_back({ numNorth, numSouth, numWest, numEast });
		}
	}
};

void Matrix::CalculateConnections() {
	// a vector that contains a vector that contains all the indices that has the same
	// tile value as their index
	// NOTE: the 0th value is not used
	std::vector<std::vector<int>> tilemap(36, std::vector<int>());
	
	for (int i = 0; i < 144; i ++) {
		int tilemapIndex = this->data[i];
		if (tilemapIndex) {
			tilemap[tilemapIndex].push_back(i);
		}
	}

	for (int i = this->Tiles::BUTTER_FREE; i <= this->Tiles::PIKACHU; i ++) {
		std::vector<int> indices = tilemap[i];

		for (int index : indices) {
			// get all the indeces that isn't the same as the index we're current checking
			std::vector<int> otherIndices;	
			std::copy_if(indices.begin(), indices.end(), std::back_inserter(otherIndices), [&index](int& otherIndex) { return index != otherIndex; });
		
			for (int remainingIndex : otherIndices) {
				std::vector<int> path = this->Connect(index, remainingIndex);
				
				// if the Connect() function returns an empty vector then the connection
				// is invalid
				 if (path.size() > 0)
				 	this->connectionPaths.push_back(path); 
			}
		}
	}
}

void Matrix::Update(int startIndex, int targetIndex, int seed) {
	this->numConnectedIndices += 2;

	this->data[startIndex] = this->Tiles::NONE;
	this->data[targetIndex] = this->Tiles::NONE;

	this->UpdateLevel(startIndex, targetIndex);

	this->connectionPaths.clear();
	this->CalculateConnections();
}

void Matrix::UpdateLevel(int startIndex, int targetIndex) {
	if (!this->currentLevel)
		return;

	int startDirIndex; int targetDirIndex; bool split;
	switch (this->currentLevel) {
		case 1:
		case 2:
		case 3:
		case 4: {
			// re-using variables because #save_nature
			startDirIndex = targetDirIndex = (this->currentLevel - 1); 
			split = false;
			break;
		}
		case 5: {
			startDirIndex = (this->IndexToPoint(startIndex).y() < 5) ? 1 : 0;
			targetDirIndex = (this->IndexToPoint(targetIndex).y() < 5) ? 1 : 0;
			split = true;
			break;
		}
		case 6: {
			startDirIndex = (this->IndexToPoint(startIndex).x() < 8) ? 3 : 2;
			targetDirIndex = (this->IndexToPoint(targetIndex).x() < 8) ? 3 : 2;
			split = true;
		}
		default:
			break;
	}
	this->Shift(startIndex, startDirIndex, split);
	this->Shift(targetIndex, targetDirIndex, split);
}

// if the size is empty, we re-shuffle the matrix data and recalculate the connections
// i know the chance of it re-shuffling and still doesn't have any valid connections
// is very low but better safe than sorry xd
void Matrix::UpdateData(int seed) {
	const int maxTries = 5; // capping it to prevent it from asiodsaidjsaidjsaiodjsad when the game is actually over
	int tries = 0;

	while (tries < maxTries) {
		this->Shuffle(seed);
		this->CalculateConnections();
		tries ++;
	}
}

std::vector<int> Matrix::Connect(int startIndex, int targetIndex) {
	QPoint startPoint = this->IndexToPoint(startIndex);
	QPoint targetPoint = this->IndexToPoint(targetIndex);

	// checking if the indecies are right next to each other, in that case we don't have to waste time calculating stuff
	QPoint deltaPoint (std::abs (startPoint.x() - targetPoint.x()), std::abs (startPoint.y() - targetPoint.y()));

	if (deltaPoint == QPoint(0, 1) || deltaPoint == QPoint(1, 0)) {
		// std::cout << "\033[1mDEBUG: \033[1;32mAccepted\033[0m (Adjacent case) " << startIndex << ", " << targetIndex << std::endl;
		// std::cout << std::endl;
		return { startIndex, targetIndex };
	}

	for (int directionIndex = 0; directionIndex < 4; directionIndex ++) {
		int directionOffset = this->directionOffsets[directionIndex];
		// we need to add one because we also wanna take into account the startIndex as well
		int numSquaresToEdge = this->indicesToEdge[startIndex][directionIndex] + 1;

		int parallelDistance = (directionIndex >= 2) ? deltaPoint.y() : deltaPoint.x();	
		int parallelOffset = (directionIndex >= 2) ? 16 : 1;
		
		// determining which sign to use for the parallelOffset
		if (directionIndex >= 2) 
			parallelOffset *= (startPoint.y() < targetPoint.y() ? 1 : -1);
		else
			parallelOffset *= (startPoint.x() < targetPoint.x() ? 1 : -1);
		
		for (int i = 0; i < numSquaresToEdge; i ++) {
			int stepIndex = startIndex + i * directionOffset;
			int stepValue = this->data[stepIndex];
			
			// parallelIndex is basically the index that is on the same column as the stepIndex (on the axis which we're aren't working with, 
			// i.e: X axis if the current directionIndex is 0 (UP)), the reason why it is named parallelIndex is because the way how I came up with this
			// method, I drew two lines starting from startIndex and targetIndex on the X and Y axis, and so the parallelIndex is on the targetIndex line
			// which is "parallel" to the startIndex line, yeah
			int parallelIndex = stepIndex + parallelDistance * parallelOffset;
		
			QPoint stepPoint = this->IndexToPoint(stepIndex);
			QPoint parallelPoint = this->IndexToPoint(parallelIndex);

			// stores the current pass we're doing, this is nessescary for the edge case check
			int currentPass = 0;
			// the distance between parallelIndex and targetIndex
			int targetDistance = std::abs((directionIndex >= 2) ? targetPoint.x() - parallelPoint.x() : targetPoint.y() - parallelPoint.y());
			
			// the directionOffset used to get from targetIndex to parallelIndex
			int targetOffset = (directionIndex >= 2) ? 1 : 16;
		
			// fancy multiply to get the accurate offset otherwise shit happens
			if (directionIndex >= 2)
				targetOffset *= (targetPoint.x() < parallelPoint.x()) ? 1 : -1;
			else 
				targetOffset *= (targetPoint.y() < parallelPoint.y()) ? 1 : -1;
			
			bool isOnEdge = (this->IsIndexOnEdge(stepIndex, directionIndex) && this->IsIndexOnEdge(parallelIndex, directionIndex)) &&
						((directionIndex >= 2) ? stepPoint.x() == parallelPoint.x() : stepPoint.y() == parallelPoint.y());
			// first pass that checks whether or not the stepIndex is blocked, we don't care
			// about whether or not the parallelIndex is actually blocked or not, that is handled
			// in the 2nd and 3rd pass (and it should be, if i were to put that check here it would just break one of the many cases that the two indecies
			// can be connected)
			currentPass = 1;
			if (stepValue && stepIndex != startIndex)
				goto endDirectionLoop;

			// gonna use a for loop to deal with the 2 remaining passes because they shared the same idea and code pretty much
			for (int j = 0; j < 2; j ++) {
				currentPass ++; // advance the pass counter

				// the rest should be self explainatory
				// 	second pass: 	checks whether or not between targetIndex and parallelIndex has any blockades
				// 	third pass: checks whether or not between stepIndex and parallelIndex has any blockades
				//
				// please note that the third pass can failed if the two indices are on edge, it is still a valid connection
				// and is handled in the endEdgeLoop label thingy
				int offset 	= (currentPass == 2) ? targetOffset 	: parallelOffset;
				int distance 	= (currentPass == 2) ? targetDistance 	: parallelDistance;
				int origin 	= (currentPass == 2) ? targetIndex 	: stepIndex;
				
				for (int k = 0; k < distance; k ++) {		
					int deltaIndex = origin + (k + 1) * offset;
					// fail safe to prevent it from going out of bounds
					if (this->IsIndexOutOfBounds(deltaIndex))
						goto endDirectionLoop;
					
					int deltaValue = this->data[deltaIndex];
					if (deltaValue)
						goto endEdgeLoop;
				}
			}
			// std::cout << "\033[1mDEBUG: \033[1;32mAccepted\033[0m (Non edge-case): " << startIndex << ", " << targetIndex << std::endl;
			// std::cout << "Additional info: { currentPass: " << currentPass << ", directionIndex: " << directionIndex << ", iteration: " << i << ", stepIndex: " << stepIndex << ", parallelIndex: " << parallelIndex << " } " << std::endl;
			// std::cout << std::endl;
			// pretty normal stuff, i think
			return { startIndex, stepIndex, parallelIndex, targetIndex };

			endEdgeLoop: {
				// the 3rd pass is optional and can be omitted if the two indecies are on edge
				if (isOnEdge && currentPass == 3) {
					// std::cout << "\033[1mDEBUG: \033[1;32mAccepted\033[0m (Edge-case): " << startIndex << ", " << targetIndex << std::endl;
					// std::cout << "Additional info: { currentPass: " << currentPass << ", directionIndex: " << directionIndex << ", iteration: " << i << ", stepIndex: " << stepIndex << ", parallelIndex: " << parallelIndex << " } " << std::endl;
					// std::cout << std::endl;
				
					// returns a vector with 5 elements in it, the last one being directionIndex as we're
					// gonna need it when we draw the connect line, and also it doubles as a indicator
					// for whether the connection is an edge-case or not
					return { startIndex, stepIndex, parallelIndex, targetIndex, directionIndex };
				} else {
					// we can still continue on this direction however
					// also note that the only case where we cannot go any further is if
					// stepIndex is non-empty (which means there is a blockade in that direction)
					// or if the second or third pass checks for an invalid index (the this->IsIndexOutOfBounds() thingy is for that)
					continue;
				}
			};

			endDirectionLoop: {
				// std::cout << "\033[1mDEBUG: \033[1;31mRejected\033[0m: " << startIndex << ", " << targetIndex << std::endl;
				// std::cout << "Additional info: { currentPass: " << currentPass << ", directionIndex: " << directionIndex << ", iteration: " << i << " }" <<std::endl;
				// std::cout << std::endl;
				break;
			};
		}
	}
	// empty vector because, duh
	return {};
}

/*
 * 	SOME USEFUL STUFF
 */

// location can be any index on that direction axis (which basically means the axis we're moving, i.e: UP -> Y axis)
void Matrix::Shift(int location, int directionIndex, bool split) {	
	// the idea here is: we wanna grab all the index on the direction axis, maybe split it in half if
	// split is true, convert that into tile values, delete the locationIndex and finally push back an empty tile value
	// to the values, and then we apply that to our matrix's data
	std::vector<int> columnIndices = (directionIndex >= 2) ? this->GetXColumn(location) : this->GetYColumn(location);
	// represent where the index of location in columnIndices
	int locationIndex = (directionIndex >= 2) ? this->IndexToPoint(location).x() : this->IndexToPoint(location).y();

	// fancy iterator stuff that removes half of the columnIndices (the part that doesn't contain location)
	if (split) {
		int splitIndex = columnIndices.size() / 2;
		std::vector<int>::iterator begin = (locationIndex > splitIndex) ? 
							columnIndices.begin() : columnIndices.begin() + (splitIndex + 1);
		std::vector<int>::iterator end = (locationIndex > splitIndex) ?
							columnIndices.begin() + splitIndex : columnIndices.end();
		columnIndices.erase(begin, end);
		locationIndex = (locationIndex > splitIndex) ? locationIndex - splitIndex : locationIndex;
	}

	std::vector<int> columnValues = columnIndices;
	std::for_each(columnValues.begin(), columnValues.end(), [this](int& index)->void { index = this->data[index]; });
	
	// erase-remove bullshit, see https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
	// erasing every element that contains 0
	columnValues.erase(std::remove_if(columnValues.begin(), columnValues.end(), [](int& value)->bool { return !value; }), columnValues.end());

	// pushing back an empty value for that many deleted elements
	int numRemovedElements = columnIndices.size() - columnValues.size();
	for (int i = 0; i < numRemovedElements; i ++) {
		std::vector<int>::iterator it = (directionIndex == 0 || directionIndex == 2) ? columnValues.end() : columnValues.begin();
		columnValues.insert(it, this->Tiles::NONE);
	}
	// finally, setting the values in the matrix's data
	for (int i = 0; i < columnIndices.size(); i ++) {
		int& index = columnIndices[i];
		int& value = columnValues[i];
		this->data[index] = value;
	}
}

QPoint Matrix::IndexToPoint(int index) const {
	int y = index / 16;
	int x = index - y * 16;

	return QPoint(x, y);
}

std::vector<int> Matrix::GetXColumn(int index) const {
	QPoint point = this->IndexToPoint(index);
	int startX = 16 * point.y();

	std::vector<int> result;
	for (int i = 0; i < 16; i ++)
		result.push_back(startX + i);
	return result;
}

std::vector<int> Matrix::GetYColumn(int index) const {
	QPoint point = this->IndexToPoint(index);
	int startY = point.x();

	std::vector<int> result;
	for (int i = 0; i < 9; i ++)
		result.push_back(startY + 16 * i);
	return result;
}

std::vector<int> Matrix::GetConnectionData(int startIndex, int targetIndex) {
	auto f = [startIndex, targetIndex](std::vector<int>& data) {
		bool isAdjacentCase = (data.size() == 2);
		
		int front = data[0];
		int back = (isAdjacentCase) ? data[1] : data[3];
		
		return front == startIndex && back == targetIndex;
	};

	std::list<std::vector<int>>::iterator i = std::find_if(this->connectionPaths.begin(), this->connectionPaths.end(), f);
	return *i;
}

// the directionIndex here is mainly used to determine 
// on which axis do we wanna check 
bool Matrix::IsIndexOnEdge(int index, int directionIndex) const {
	QPoint point = this->IndexToPoint(index);

	if (directionIndex == 0 || directionIndex == 1)
		return (directionIndex == 1) ? point.y() == 0 : point.y() == 8;
	
	else if (directionIndex == 2 || directionIndex == 3)
		return (directionIndex == 2) ? point.x() == 0 : point.x() == 15;

	return false;
}

bool Matrix::IsIndexOutOfBounds(int index) const {
	return index < 0 || index > 143;
}

bool Matrix::IsConnectionValid(int startIndex, int targetIndex) {
	auto f = [startIndex, targetIndex](std::vector<int>& data) {
		bool isAdjacentCase = (data.size() == 2);
		
		int front = data[0];
		int back = (isAdjacentCase) ? data[1] : data[3];
		
		return front == startIndex && back == targetIndex;
	};
	return std::find_if(this->connectionPaths.begin(), this->connectionPaths.end(), f) != this->connectionPaths.end();
}
