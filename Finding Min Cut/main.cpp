#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <math.h>  

using std::vector;
using namespace std;

vector<vector<int>>* ReadAdjList(std::string txtFile, const int NO_VERTS);
int GetIndex(int homeVert, int targetVert, vector<vector<int>>* adjList);
void DeleteSelfLoops(vector<vector<int>>* adjList, int mergerIndex, int merger);

int main(){
	//read the adj list from the file

	const int NO_VERTS = 200;
	const int NO_REPS = 200; //NO_REPS = n^2 * lnN where n = number verts.
	
	vector<vector<int>>* adjList;
	srand(time(NULL));

	//run the algorithm (n^2)*ln(n) times, where n is the number of vertices in the graph. This lowers chance of failure to 1/n.
	//see Stanford Algo course 1 Analysis of Contraction Algorithm lecture for how to generate this bound.

	int* minCut_attempts = new int[NO_REPS]; 
	int remainingVerts, merger, mergee, mergerIndex, mergeeIndex, mergeeIndexInMerger, mergerDegree, currVertex, temp, minCut = 0;

	for (int q = 0; q < NO_REPS; q++){
		remainingVerts = NO_VERTS;
		adjList = ReadAdjList("kargerMinCut.txt", NO_VERTS);

		//while there are >2 vertices remaining randomly choose an edge to remove (aka choose two vertices to merge)

		while (remainingVerts > 2){
			//randomly select a vertex ("merger") then randomly select a vertex connected to it ("mergee").
			//The edge between these vertices will be removed. Merge mergee into merger.

			mergerIndex = rand() % NO_VERTS;
			while ((*adjList)[mergerIndex][0] == -1) {//-1 if we've already merged the edge we're examining
				mergerIndex++;
				if (mergerIndex == NO_VERTS) mergerIndex = 0;
			}

			merger = mergerIndex + 1;//first vertex stored at [0], second at [1]...
			mergerDegree = (*adjList)[mergerIndex].size();

			mergeeIndexInMerger = rand() % mergerDegree;
			while (mergeeIndexInMerger == 0 || (*adjList)[mergerIndex][mergeeIndexInMerger] == -1){//can't select the first entry of (*adjList)[merger] since it holds flag that tracks if vertex has been deleted.
				mergeeIndexInMerger++;
				if (mergeeIndexInMerger == (*adjList)[mergerIndex].size()) mergeeIndexInMerger = 1;
			}
			mergee = (*adjList)[mergerIndex][mergeeIndexInMerger];
			mergeeIndex = mergee - 1;

			//remove the vertex that is going to be merged (the "mergee") from the adj list of the vertex that will absorb the mergee 
			//( the "merger"). Do the same thing in the other direction. This removes the edge that we want to delete.

			(*adjList)[mergerIndex][mergeeIndexInMerger] = -1;
			(*adjList)[mergeeIndex][GetIndex(mergeeIndex, merger, adjList)] = -1;

			//look at the remaining entries of the mergee's adj list. For each entry, change the associated vertex in the adj list s.t they are adjacent to the merger instead
			//of the mergee (after the merge any vertex that was adjacent to the mergee will be adjacent to the merger). -1 means the vertex has already been deleted.
			//We also append what remains of the mergee's list to the merger's list and "delete" the mergee from the adj list (ie flag as deleted).
			
			for (int i = 1; i < (*adjList)[mergeeIndex].size(); i++){
				currVertex = (*adjList)[mergeeIndex][i];
				if (currVertex != -1){
					(*adjList)[currVertex-1][GetIndex(currVertex-1, mergee, adjList)] = merger;//currVertex-1 = index of currVertex in the adj list.
					(*adjList)[mergerIndex].push_back(currVertex);
					(*adjList)[mergeeIndex][i] = -1;
				}
			}
			(*adjList)[mergeeIndex][0] = -1;//mark vertex as completely deleted.
			DeleteSelfLoops(adjList, mergerIndex, merger);
			remainingVerts--;
		}

		//once there are only 2 vertices left, return the number of edges connecting them. This number represents the algorithm's attempt at finding the mid cut.
		
		for (int i = 1; i < (*adjList)[mergerIndex].size(); i++){
			if ((*adjList)[mergerIndex][i] != -1) minCut++;
		}
		minCut_attempts[q] = minCut;
		minCut = 0;
		delete adjList;//need to read in again since we've chanced all the entries but 2 to -1
	}
	minCut = minCut_attempts[0];
	for (int i = 1; i < NO_REPS; i++){
		temp = minCut_attempts[i];
		if (temp < minCut) minCut = temp;
	}
	delete[] minCut_attempts;
	cout << minCut;
	return 0;
}

/*Convert the adjaceny list in the text file to an vector-of-vectors adjaceny list within our program.*/
vector<vector<int>>* ReadAdjList(std::string fileName, const int NO_VERTS) {
	std::ifstream adjList_txt(fileName);
	vector< vector<int> >* adjList;
	adjList = new vector< vector<int> >(NO_VERTS);

	if (!adjList_txt.is_open()) {
		std::cout << "Failed to open file.";
		return 0;
	}
	else {
		// Safely use the file stream
		std::string line;
	
		int i = 0;
		int vertex;

		while (std::getline(adjList_txt,line))
		{
			std::istringstream iss(line);
			while (iss >> vertex)
			{
				(*adjList)[i].push_back(vertex);
			}
			i++;
		}
	}
	return adjList;
}

/*Given a vertex (homeVert), find the location of targetVert in homeVert's adjaceny list. 
Return the index of the targetVert, -1 if we can't find it.*/
int GetIndex(int homeVertIndex, int targetVert, vector<vector<int>>* adjList) {

	for (int i = 0; i < (*adjList)[homeVertIndex].size(); i++){
		if ((*adjList)[homeVertIndex][i] == targetVert) return i;
	}
	return -1;
}

//If the merging of two vectors results in a self loop (resulting vector has an edge to itself)
//we need to delete it for Karger's algo to work properly.
void DeleteSelfLoops(vector<vector<int>>* adjList, int mergerIndex, int merger){
	for (int i = 1; i < (*adjList)[mergerIndex].size(); i++){
		if ((*adjList)[mergerIndex][i] == merger) (*adjList)[mergerIndex][i] = -1;
	}
	return;
}