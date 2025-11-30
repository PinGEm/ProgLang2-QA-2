#include <iostream> // used for input and output streams
#include <thread> // used to pause after each step
#include <conio.h> // used for getch()
#include <chrono> // used to pause after each step
#include <cmath> // used for sigmoid function in neural network
#include <vector> // used for dynamic lists in student class

// GRID LAYOUT
const int GRID_SIZE = 10;
const char STUDENTS = 'S';
const char WALL = '|';
const char SPACE = '.';
const char FRIEND = 'F';
const char GATHER = 'G'; 

// Using Definers to shorten certain common keyphrases
using std::cout;
using std::cin;

char grid[GRID_SIZE][GRID_SIZE]; // stores the actual map for the grid
int visited_count[GRID_SIZE][GRID_SIZE] = {0};  // stores how much times a space has been visited in a grid
int actualFriendX, actualFriendY; // stores actual location of the Friend

// ==========================================
//              NEURAL NETWORK 
// ==========================================
class NeuralNetwork {
	private:
		// Activation function mapping inputs between values of 0-1
		static double sigmoid(double x) {
			return 1 / (1 + exp(-x));
		}
		
		// Backpropogation function used for readjusting the weights of the training data
		static double sigmoidDerivative(double x) {
			return x * (1.0 - x);
		}

		double learningRate = 0.5; // rate at which the AI learns

	public:
	
		// Represents each individual neuron inside the neural network
		class Neuron {
			public:
				// Initiate variables
				double* weights;
				int num_weights;
				double bias;
				double output;
				double delta; // not used within this class, but used when training data.
				
				// When this class is instantiated, do the following:
				Neuron(int num_inputs) : num_weights(num_inputs) {
					// for now, randomly set the weights and biases of this neuron.
					weights = new double[num_weights];
					for (int i = 0; i < num_weights; ++i) {
						weights[i] = ((double)rand() / RAND_MAX) - 0.5;
					}
					bias = ((double)rand() / RAND_MAX) - 0.5;
				}

				~Neuron() {
					// Free up memory
					delete[] weights;
				}

				double activate(double* inputs) {
					// consider this the output of this neuron.
					double sum = bias;
					for (int i = 0; i < num_weights; ++i) {
						sum += inputs[i] * weights[i];
					}
					output = NeuralNetwork::sigmoid(sum); // evaluate the output between the inputs based on the weights and bias
					return output;
				}
		};
		
		// Represents each layer inside the neural network
		class Layer {
			public:
				// Initiate variables
				Neuron** neurons; // create a dynamic array of neurons
				int num_neurons;
				
				// Create all the necessary neurons for this layer.
				Layer(int num_neurons_, int num_inputs_per_neuron) : num_neurons(num_neurons_) {
					neurons = new Neuron * [num_neurons];
					for (int i = 0; i < num_neurons; ++i) {
						// Each neuron is created with the proper inputs per neuron
						neurons[i] = new Neuron(num_inputs_per_neuron);
					}
				}

				~Layer() {
					// free up all the neurons. make sure to delete all the neurons in the array.
					for (int i = 0; i < num_neurons; ++i) {
						delete neurons[i];
					}
					delete[] neurons;
				}
				
				// outputting all the neurons 
				double* feedForward(double* inputs) {
					double* outputs = new double[num_neurons];
					for (int i = 0; i < num_neurons; ++i) {
						// iterate through each neuron inside this layer and call the sigmoid function inside them.
						outputs[i] = neurons[i]->activate(inputs);
					}
					
					// return all the outputs.
					return outputs; // note: make sure to call the delete[] when using this!
				}
		};
		
		// Creation of variables for the layers of the neural network
		Layer** layers;
		int num_layers;

		// Instantiate all the layers and the number of neurons inside each layer.
		NeuralNetwork(int* topology, int size) : num_layers(size - 1) {
			layers = new Layer * [num_layers];
			for (int i = 0; i < num_layers; ++i) {
				// create all the layers and the necessary neurons and input for it
				layers[i] = new Layer(topology[i + 1], topology[i]);
			}
		}

		// Free up the memory and delete all the layers.
		~NeuralNetwork() {
			for (int i = 0; i < num_layers; ++i) {
				delete layers[i];
			}
			delete[] layers;
		}
		
		// Based on the neural network, predict and evaluate the right output value based on the input.
		double* predict(double* inputs, int input_size) {
			
			// Create the array of inputs and their correct size
			double* current_inputs = new double[input_size];
			for (int i = 0; i < input_size; ++i)
			{
				current_inputs[i] = inputs[i];
			}
			
			// Iterate through each layer and feed them the input.
			for (int l = 0; l < num_layers; ++l) {
				// get the output from feeding the inputs to the layers, then set the current input the output value.
				double* outputs = layers[l]->feedForward(current_inputs);
				if (l > 0) 
				{
					delete[] current_inputs;
				}
				current_inputs = outputs;
			}
			
			// Return the predicted value
			return current_inputs; // make sure to handle deletion of the output here !
		}
		
		// Adjust for the neural network's ACTUAL weights and biases based on the expected value and trained data.
		void train(double* inputs, double* expected, int input_size) {
			double* outputs = predict(inputs, input_size); // to get the initial evaluation of the inputs

			// Compute delta/error for each neuron in the output layer
			Layer* outputLayer = layers[num_layers - 1];
			for (int i = 0; i < outputLayer->num_neurons; ++i) {
				// access each neuron's delta and compute the necessary adjustments based on the output, expected and the sigmoid
				double out = outputLayer->neurons[i]->output;
				outputLayer->neurons[i]->delta = (expected[i] - out) * sigmoidDerivative(out);
			}

			// Compute delta/error for each neuron in the hidden layers
			for (int l = num_layers - 2; l >= 0; --l) {
				Layer* current = layers[l];
				Layer* next = layers[l + 1];
				for (int i = 0; i < current->num_neurons; ++i) {
					// access each neuron's delta and compute the necessary adjustments based on errors of the NEXT layers
					double error = 0.0;
					for (int j = 0; j < next->num_neurons; ++j) {
						// we also gradually compute the adjustments for the next layer.
						error += next->neurons[j]->weights[i] * next->neurons[j]->delta;
					}
					current->neurons[i]->delta = error * sigmoidDerivative(current->neurons[i]->output);
				}
			}

			// Update the weights and biases in each layers neurons
			for (int l = 0; l < num_layers; ++l) {
				Layer* layer = layers[l];
				double* layerInputs;
				int num_inputs;
				
				
				if (l == 0) {
					// IF WE ARE AT THE INPUT LAYER
					layerInputs = inputs;
					num_inputs = input_size;
				}
				else {
					// IF WE ARE AT THE HIDDEN/OUTPUT LAYERS
					Layer* prevLayer = layers[l - 1];
					num_inputs = prevLayer->num_neurons;
					layerInputs = new double[num_inputs];
					for (int i = 0; i < num_inputs; ++i)
					{
						// store the outputs of each neuron inside the layerInputs.
						layerInputs[i] = prevLayer->neurons[i]->output;
					}
				}
				
				 
				for (int n = 0; n < layer->num_neurons; ++n) {
					// adjust the weights and bias of each neuron now that we have computed the delta.
					Neuron* neuron = layer->neurons[n];
					for (int w = 0; w < neuron->num_weights; ++w) 
					{
						// compute the weights based on learning rate, delta, and the layers output
						neuron->weights[w] += learningRate * neuron->delta * layerInputs[w];
					}
					neuron->bias += learningRate * neuron->delta;
				}

				if (l != 0) 
				{
					// if this is the input layer, just delete this.
					delete[] layerInputs;
				}
			}
		
		// clear up the memory now that we don't need to store this data
		delete[] outputs;
	}
};

NeuralNetwork* sharedBrain = nullptr; // brain to be used by all the students

// ==========================================
//             STUDENT LOGIC
// ==========================================

// Struct to track a specific move decision
struct MoveOption {
    int x, y;
    int dirIndex; // 0=Up, 1=Down, 2=Left, 3=Right, -1=None
};

// Used for the neural network program
struct HistoryStep {
    double inputs[4]; // The context seen
    int bestDir;      // The move that was chosen
};

class Student{
	public:
		// defining variables to be used for the student class
		int x,y;
		bool knowsFriend = false;
		std::vector<HistoryStep> pathHistory; // Remember path for training, used for the neural network
		
		// Called when the students' location needs to be updated ( usually preceded by tryMove() )
		void updateLocation(int newX, int newY, int knownFX, int knownFY){
			if (grid[x][y] == STUDENTS)
			{
				grid[x][y] = SPACE; // used to handle errors in case when 2 students happen to be in the same space
			}
			
			// Set the new x and y coordinate positions, and update the visited count.
			x = newX;
			y = newY;
			visited_count[x][y]++; 
			
			(x == actualFriendX && y == actualFriendY) ? grid[x][y] = GATHER : grid[x][y] = STUDENTS; // if true, student is at the gathering place
		}

		bool tryMove(bool friendFound, int knownFX, int knownFY) {
			int nextX = x; // set initial position for x coordinate 
			int nextY = y; // set initial position for y coordinate 
			int chosenDir = -1; // For recording history

			// Prepare Inputs for NN (Normalized 0.0 - 1.0)
			double inputs[4];
			inputs[0] = (double)x / GRID_SIZE;
			inputs[1] = (double)y / GRID_SIZE;
			inputs[2] = (double)knownFX / GRID_SIZE;
			inputs[3] = (double)knownFY / GRID_SIZE;
			
			// Get NN Prediction (Preferences for Up, Down, Left, Right)
			double* nnPrefs = sharedBrain->predict(inputs, 4);

			if (friendFound) {
				// --- IF FRIEND IS FOUND : GATHERING BEHAVIOR (Standard Greedy) ---
				int idealX = x, idealY = y;
				if (x != knownFX) idealX += (x < knownFX) ? 1 : -1; // move towards the friend's X coordinates
				if (y != knownFY) idealY += (y < knownFY) ? 1 : -1; // move towards the friend's Y coordinates
				
				// check if the move is within the confines of the grid, and is not a wall
				bool idealMoveValid = (idealX >= 0 && idealX < GRID_SIZE && idealY >= 0 && idealY < GRID_SIZE && grid[idealX][idealY] != WALL);
				
				if (idealMoveValid) 
				{
					// if ideal move is valid, proceed.
					nextX = idealX; nextY = idealY;
				} 
				else 
				{
					// Fallback: Random wiggle if blocked
					 do {
						nextX = x + (rand() % 3) - 1; 
						nextY = y + (rand() % 3) - 1; 
					} while ( (nextX != x || nextY != y) && (nextX < 0 || nextX >= GRID_SIZE || nextY < 0 || nextY >= GRID_SIZE || grid[nextX][nextY] == WALL) );
				}
			} 
			else 
			{
				// --- GREEDY EXPLORATION + NEURAL GUIDANCE ---
				int minVisits = 99999;
				std::vector<MoveOption> bestMoves; // create a list of best moves for the student to take

				// Map standard loops to direction indices for the NN
				// Indices: 0:Up (-1,0), 1:Down (1,0), 2:Left (0,-1), 3:Right (0,1)
				int dX[4] = {-1, 1, 0, 0};
				int dY[4] = {0, 0, -1, 1};

				// Check for the 4 cardinal directions, then count the visits (simplified for NN mapping)
				for(int i=0; i<4; i++) {
					// update the positions for x and y
					int checkX = x + dX[i];
					int checkY = y + dY[i];
					
					// Check for space validity
					if (checkX >= 0 && checkX < GRID_SIZE && checkY >= 0 && checkY < GRID_SIZE && grid[checkX][checkY] != WALL) 
					{
						// Count the total visit counts for this space
						int currentVisits = visited_count[checkX][checkY];

						// Standard Greedy Logic: Find lowest visit count
						if (currentVisits < minVisits) 
						{
							// If this space has been visited less times than the other, reset the list and push this one forward.
							minVisits = currentVisits;
							bestMoves.clear();
							bestMoves.push_back({checkX, checkY, i});
						} 
						else if (currentVisits == minVisits) 
						{
							// add to the best moves list
							bestMoves.push_back({checkX, checkY, i});
						}
					}
				}

				if (!bestMoves.empty()) {
					// HYBRID DECISION:
					// We have a list of "Best Greedy Moves" (bestMoves).
					// Use the Neural Network to pick the absolute best among them.
					
					int bestIndex = 0;
					double maxConfidence = -1.0;

					// If we have multiple equally good "Greedy" options, ask the Brain.
					if (bestMoves.size() > 1) 
					{
						// Run through all the best moves, and see which one the neural network is most confident in
						for(int i=0; i < bestMoves.size(); i++) 
						{
							int dir = bestMoves[i].dirIndex;
							// Use the NN output for this direction as the score
							if(nnPrefs[dir] > maxConfidence) 
							{
								maxConfidence = nnPrefs[dir];
								bestIndex = i;
							}
						}
					} 
					else 
					{
						// Only one greedy option, take it.
						bestIndex = 0;
					}
					
					// Set the next positions and directions based on whatever the best move the AI found.
					nextX = bestMoves[bestIndex].x;
					nextY = bestMoves[bestIndex].y;
					chosenDir = bestMoves[bestIndex].dirIndex;
				} 
			}

			// Record History for Training later
			if(chosenDir != -1 && !friendFound) {
				HistoryStep step;
				for(int i=0; i<4; i++) 
				{
					step.inputs[i] = inputs[i];
				}
				
				step.bestDir = chosenDir;
				pathHistory.push_back(step);
			}

			delete[] nnPrefs; // Clean up memory

			// EXECUTE MOVE
			if (nextX != x || nextY != y) 
			{
				// Check if we're going to be finding the friend
				if (grid[nextX][nextY] == FRIEND && !friendFound) 
				{
					knowsFriend = true;
					updateLocation(nextX, nextY, knownFX, knownFY);
					return true; // Found Friend!
				}
				
				// Update the students location
				updateLocation(nextX, nextY, knownFX, knownFY);
			}
			return false; // Friend is not found
		}
};

// defining functions for the main() program
void showGrid(); // displays the current grid layout
bool checkGathered(Student students[], int numStudents); // checks if all the students are gathered at one place
void simulationStep(Student students[], int numStudents, bool &friendFound, int &knownFX, int &knownFY); // the movement of the student


int main()
{
	// title card sequence
    cout << "**********************************************\n";
    cout << "     COLLECTIVE AI : STUDY SESSION FINDER     \n";
    cout << "**********************************************\n\n";
    
	// defining variables to be used within the main() function
    srand(time(0)); // randomizing seed for this session
    bool retry = true; // indicates if the player wants to retry the maze
    char userInput; // gets user input
    const int NUM_STUDENTS = 5; // number of students to roam around the maze
    
	// Setup Variables: Layers
    int topology[] = {4, 8, 4}; // 4 Inputs (X, Y, TargetX, TargetY) -> 1 Hidden Layer (8 Neurons) -> 4 Outputs (Up, Down, Left, Right)
    sharedBrain = new NeuralNetwork(topology, 3);
    
	// Used to keep track of each reset count
    int generation = 0;

    do{
        generation++; // +1 to the reset count
        
		// Fill up the grid with empty spaces initially
        for(int i = 0; i < GRID_SIZE; i++) {
            for(int j = 0; j < GRID_SIZE; j++) {
                grid[i][j] = SPACE;
                visited_count[i][j] = 0; 
            }
        }
        
        // Spawn Walls 
        for(int i = 0; i < (rand() % 10) + 10; i++) {
            int rX = rand() % GRID_SIZE; // randomly set the x position for wall
            int rY = rand() % GRID_SIZE; // randomly set the y position for wall
            grid[rX][rY] = WALL; // set the wall inside the grid
        }
        
        // Spawn Friend 
        do{
            actualFriendX = rand() % GRID_SIZE; // randomly set the x position for friend
            actualFriendY = rand() % GRID_SIZE; // randomly set the y position for friend
        }while(grid[actualFriendX][actualFriendY] != SPACE);
        
		// Set the variables for all the friend information
        grid[actualFriendX][actualFriendY] = FRIEND;
        int knownFriendX = actualFriendX; 
        int knownFriendY = actualFriendY; 
    
        // Spawn Students 
        Student students[NUM_STUDENTS] = {};
        
        for(int i = 0; i < NUM_STUDENTS; i++) {
            int x, y;
			
			// Find a space where the students can spawn
            do{
                x = rand() % GRID_SIZE; // randomly set the x position for student
                y = rand() % GRID_SIZE; // randomly set the y position for student
            }while(grid[x][y] != SPACE);
                
			// Set the students location
            students[i].updateLocation(x,y, knownFriendX, knownFriendY);
        }
        
        bool friendFound = false;
        int stepCount = 0;
        
        // --- Simulation Loop ---
        while(!checkGathered(students, NUM_STUDENTS) && stepCount < 60) { // Safety break at 60 steps
            cout << "Gen " << generation << " | Step " << ++stepCount << "\n"; // displays generation and step count
            simulationStep(students, NUM_STUDENTS, friendFound, knownFriendX, knownFriendY);
            showGrid(); // display grid layout now
            std::this_thread::sleep_for(std::chrono::milliseconds(400)); // wait 400 miliseconds
        }
        
        if (checkGathered(students, NUM_STUDENTS)) 
		{
			// Display a success message!
            cout << "\nSUCCESS! Students gathered at (" << actualFriendX << "," << actualFriendY << ").\n";
        } 
		else 
		{
			// Display a failed message
             cout << "\nTIMEOUT. Resetting...\n";
        }

        // --- TRAINING PHASE ---
        // Teach the brain based on the student who actually found the friend.
        // We assume the greedy path that WORKED is a path worth learning.
		
        cout << "Training Neural Network on successful paths...\n";
		
		// Iterate and check through each student to see if they found the friend
        for(int i=0; i < NUM_STUDENTS; i++) 
		{
            if(students[i].knowsFriend) 
			{
				// Train it based on each of the students' step in their path history.
                for(auto& step : students[i].pathHistory) 
				{
                    double expected[4] = {0,0,0,0};
                    expected[step.bestDir] = 1.0; // The direction that led to success is correct
                    sharedBrain->train(step.inputs, expected, 4);
                }
            }
        }
        
		// Ask the user if they would like to retry the maze?
        cout << "Retry? (Y/N): ";
        cin >> userInput;
        retry = (userInput == 'Y' || userInput == 'y');

    }while(retry);
    
    delete sharedBrain;
    return 0;
}


// *************************
// --- HELPER FUNCTIONS ---
// *************************


// Display the grid layout: shows all the walls, users, friends, and empty spaces
void showGrid()
{
	// Iterate through each grid, displaying the location
    for(int i = 0; i < GRID_SIZE; i++) {
        for(int j = 0; j < GRID_SIZE; j++) cout << grid[i][j] << ' ';
        cout << '\n';
    }
    cout << "\n";
}

void simulationStep(Student students[], int numStudents, bool &friendFound, int &knownFX, int &knownFY) 
{
    bool newlyFound = false;
    for (int i = 0; i < numStudents; ++i) 
	{
		// Check if the friend has been found for the first time
        if (!friendFound) 
		{
            if (students[i].tryMove(friendFound, knownFX, knownFY))
			{
				newlyFound = true;
			} 
        } 
		else 
		{
			// Move normally
			students[i].tryMove(friendFound, knownFX, knownFY);
        }
    }
	
    if (newlyFound) 
	{
		// If friend has just been found, display the message
        friendFound = true;
        cout << "FRIEND FOUND! Converging...\n";
    }
}

bool checkGathered(Student students[], int numStudents) 
{
	// Iterate through each student and check if they all found the student
    for (int i = 0; i < numStudents; ++i) 
	{
        if (!(students[i].x == actualFriendX && students[i].y == actualFriendY))
		{
			return false;
		}
    }
    return true; // returns true if the previous check failed
}