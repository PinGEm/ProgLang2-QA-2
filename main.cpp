#include <iostream>
#include <thread>
#include <conio.h>
#include <chrono>
#include <cmath>
#include <vector>

const int GRID_SIZE = 10;
const char STUDENTS = 'S';
const char WALL = '|';
const char SPACE = '.';
const char FRIEND = 'F';
const char GATHER = 'G'; 

using std::cout;
using std::cin;

char grid[GRID_SIZE][GRID_SIZE];
int visited_count[GRID_SIZE][GRID_SIZE] = {0}; // NEW: Track how often each cell is visited
int actualFriendX, actualFriendY; 

// --- Student Class ---
class Student{
public:
	int x,y;
	bool knowsFriend = false;
	
	// Modified to accept and use the visited_count grid
	void updateLocation(int newX, int newY, int knownFX, int knownFY){
        // Remove old marker
        if (grid[x][y] == STUDENTS) {
		    grid[x][y] = SPACE;
        }
        
		x = newX;
		y = newY;
        
        // Increment visited count for the new cell
        visited_count[x][y]++; 

        // Place new marker
        if (x == actualFriendX && y == actualFriendY) {
            grid[x][y] = GATHER;
        } else {
            grid[x][y] = STUDENTS;
        }
	}

    bool tryMove(bool friendFound, int knownFX, int knownFY) {
        int nextX = x;
        int nextY = y;
        
        if (friendFound) {
            // **Gathering Behavior (Greedy Movement towards Target)**
			
			// 1. Calculate ideal move towards the target
            int idealX = x, idealY = y;
            if (x != knownFX) idealX += (x < knownFX) ? 1 : -1;
            if (y != knownFY) idealY += (y < knownFY) ? 1 : -1;

            // 2. Check if the ideal move is valid
            bool idealMoveValid = (idealX >= 0 && idealX < GRID_SIZE && idealY >= 0 && idealY < GRID_SIZE && grid[idealX][idealY] != WALL);
            
            if (idealMoveValid) {
                nextX = idealX;
                nextY = idealY;
            } 
			else {
                // 3. Fallback: If ideal move is blocked, try a random one-step adjustment
                do {
                    nextX = x + (rand() % 3) - 1; 
                    nextY = y + (rand() % 3) - 1; 
                    // Break if no movement or if a valid (non-wall, in-bounds) random move is found
                } while ( (nextX != x || nextY != y) && (nextX < 0 || nextX >= GRID_SIZE || nextY < 0 || nextY >= GRID_SIZE || grid[nextX][nextY] == WALL) );
                // Note: If stuck, nextX/nextY will be (x,y), meaning no move.
            }	
        } 
        else {
            // **Greedy Exploration Behavior (Move to Least Visited)**
            int minVisits = 99999;
            std::vector<std::pair<int, int>> bestMoves;

            // Check all 8 possible surrounding cells (including diagonals)
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    if (dx == 0 && dy == 0) continue; // Skip current cell

                    int checkX = x + dx;
                    int checkY = y + dy;

                    // Check bounds and walls
                    if (checkX >= 0 && checkX < GRID_SIZE && checkY >= 0 && checkY < GRID_SIZE && grid[checkX][checkY] != WALL) {
                        
                        int currentVisits = visited_count[checkX][checkY];

                        if (currentVisits < minVisits) {
                            minVisits = currentVisits;
                            bestMoves.clear();
                            bestMoves.push_back({checkX, checkY});
                        } else if (currentVisits == minVisits) {
                            bestMoves.push_back({checkX, checkY});
                        }
                    }
                }
            }

            if (!bestMoves.empty()) {
                // Choose randomly from the best (least-visited) moves
                int randomIndex = rand() % bestMoves.size();
                nextX = bestMoves[randomIndex].first;
                nextY = bestMoves[randomIndex].second;
            } else {
                // Stuck: No valid moves available, stay put
                nextX = x;
                nextY = y;
            }
        }
        
        // --- EXECUTE MOVE AND CHECK FOR FRIEND ---
        if (nextX != x || nextY != y) { // Only proceed if a movement occurred
            
            // Check for friend BEFORE updating location
            if (grid[nextX][nextY] == FRIEND && !friendFound) {
                knowsFriend = true;
                updateLocation(nextX, nextY, knownFX, knownFY);
                return true; // SIGNAL: Friend was just found by this agent!
            }
            
            updateLocation(nextX, nextY, knownFX, knownFY);
        }
        return false;
    }
};

void showGrid();
bool checkGathered(Student students[], int numStudents);
void simulationStep(Student students[], int numStudents, bool &friendFound, int &knownFX, int &knownFY);


int main()
{
	cout << "**********************************************\n";
    cout << "      COLLECTIVE AI : SWARM INTELLIGENCE      \n";
    cout << "**********************************************\n\n";
	
	// Set the seed for randomization
	srand(time(0));
	bool retry = true;
	char userInput;
    const int NUM_STUDENTS = 5;
    
    bool friendFound = false;
    int knownFriendX = -1;
    int knownFriendY = -1;
    
    do{
        // Spawn Spaces
    	for(int i = 0; i < GRID_SIZE; i++)
    	{
    		for(int j = 0; j < GRID_SIZE; j++)
    		{
    			grid[i][j] = SPACE;
                visited_count[i][j] = 0; // Initialize visited count
    		}
    	}
    	
    	// Spawn Walls 
    	for(int i = 0; i < (rand() % 10) + 21; i++)
    	{
    		int rX = rand() % GRID_SIZE;
    		int rY = rand() % GRID_SIZE;
    		grid[rX][rY] = WALL;
    	}
    	
    	// Spawn Friend 
    	do{
    		actualFriendX = rand() % GRID_SIZE;
    		actualFriendY = rand() % GRID_SIZE;
    	}while(grid[actualFriendX][actualFriendY] != SPACE);
    		
    	grid[actualFriendX][actualFriendY] = FRIEND;
        knownFriendX = actualFriendX; 
        knownFriendY = actualFriendY; 
    
    	// Spawn Students 
    	Student students[NUM_STUDENTS] = {};
    	
    	for(int i = 0; i < NUM_STUDENTS; i++)
    	{
    		int x, y;
    		do{
    			x = rand() % GRID_SIZE;
    			y = rand() % GRID_SIZE;
    		}while(grid[x][y] != SPACE);
    			
    		students[i].updateLocation(x,y, knownFriendX, knownFriendY);
    	}
    	
    	// --- Simulation Loop ---
        int stepCount = 0;
    	while(!checkGathered(students, NUM_STUDENTS)) {
            cout << "--- Step " << ++stepCount << " ---\n";
            simulationStep(students, NUM_STUDENTS, friendFound, knownFriendX, knownFriendY);
            showGrid();
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep for 500 milliseconds
    
    	}
        
        if (checkGathered(students, NUM_STUDENTS)) {
            cout << "\nSIMULATION COMPLETE! All students gathered at the friend's location (" 
                 << actualFriendX << "," << actualFriendY << ").\n";
        }
        
        
        // ASK FOR RETRY
		friendFound = false;
        cout << "Do you want to retry the maze? Type 'Y' if so.";
        cin >> userInput;
        retry = (userInput == 'Y' || userInput == 'y');
    }while(retry);
	
	cout << "**********************************************\n";
    cout << "       ENDING COLLECTIVE AI APPLICATION       \n";
    cout << "**********************************************\n\n";
	
	getch();
	return 0;
}

// --- Helper Functions ---

void showGrid(){
	for(int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			cout << grid[i][j] << ' ';
		}
		
		cout << '\n';
	}
	
	cout << "\n\n";
}

void simulationStep(Student students[], int numStudents, bool &friendFound, int &knownFX, int &knownFY) {
    bool newlyFound = false;

    // Students move
    for (int i = 0; i < numStudents; ++i) {
        if (!friendFound) {
            if (students[i].tryMove(friendFound, knownFX, knownFY)) {
                newlyFound = true; 
            }
        } else {
             students[i].tryMove(friendFound, knownFX, knownFY);
        }
    }
    
    // Instant Broadcast
    if (newlyFound) {
        friendFound = true;
        cout << "FRIEND LOCATION BROADCASTED GLOBALLY! All students are now converging (Greedy Gathering).\n";
    }
}

// Checks if all students have gathered at the friend's location
bool checkGathered(Student students[], int numStudents) {
    for (int i = 0; i < numStudents; ++i) {
        if (!(students[i].x == actualFriendX && students[i].y == actualFriendY)) {
            return false;
        }
    }
    return true;
}