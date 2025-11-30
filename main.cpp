#include <iostream>
#include <chrono>
#include <conio.h>

const int GRID_SIZE = 10;
const char STUDENTS = 'S';
const char WALL = '|';
const char SPACE = '.';
const char FRIEND = 'F';

using std::cout;
using std::cin;

char grid[GRID_SIZE][GRID_SIZE];

class Student{
	public:
	int x,y;
	bool knowsFriend = false;
	
	void updateLocation(int newX, int newY){
		grid[x][y] = SPACE;
		x = newX;
		y = newY;
		grid[x][y] = STUDENTS;
	}
};

void showGrid();

int main()
{
	srand(time(0));
	
	// Spawn Spaces
	for(int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			grid[i][j] = SPACE;
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
	int friendX, friendY;
	
	do{
		friendX = rand() % GRID_SIZE;
		friendY = rand() % GRID_SIZE;
	}while(grid[friendX][friendY] != SPACE);
		
	grid[friendX][friendY] = FRIEND;

	// Spawn Other Students
	Student students[5] = {};
	
	for(int i = 0; i < 5; i++)
	{
		int x, y;
		do{
			x = rand() % GRID_SIZE;
			y = rand() % GRID_SIZE;
		}while(grid[x][y] != SPACE);
			
		students[i].updateLocation(x,y); 
	}
	
	cout << "**********************************************\n";
	cout << "COLLECTIVE ARTIFICIAL INTELLIGENCE : HIVE MIND\n";
	cout << "**********************************************\n\n";
	
	showGrid();
	
	getch();
	return 0;
}

void showGrid(){
	for(int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			cout << grid[i][j];
		}
		
		cout << '\n';
	}
	
	cout << "\n\n";
}