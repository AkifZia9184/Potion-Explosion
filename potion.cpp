#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <iomanip>
using namespace std;


const int DISPENSER_ROWS = 7;
const int DISPENSER_COLS = 5;
const int PLAYER_BOARDS = 2;
const int PLAYER_BOARD_COLS = 5;


// Shared Memory IDs
int shmid_dispenser;
int shmid_playerBoard[PLAYER_BOARDS];
int shmid_playerScores;




// using Mutex
pthread_mutex_t mutex_dispenser = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_playerBoard[PLAYER_BOARDS] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

struct PipeArgs
{
    int mainToGamePipe[2];
    int gameToMainPipe[2];
};


enum GameState
{
    START,
    PLAYING,
    OVER
};
GameState gameState = START;


const char PLAYER_COLORS[PLAYER_BOARDS] = {'R', 'B'};
const char RED = 'R';
const char YELLOW = 'Y';
const char BLUE = 'B';
const char BLACK = 'K';
char *dispenser;
char *playerBoard[PLAYER_BOARDS];
int *playerScores;

void checkReaction(int player);

int* createSharedMemoryInt(int& shmid, int size)
{
    shmid = shmget(IPC_PRIVATE, sizeof(int) * size, IPC_CREAT | 0666);
    int* memory = (int*)shmat(shmid, NULL, 0);
    return memory;
}


char* createSharedMemoryChar(int& shmid, int size)
{
    shmid = shmget(IPC_PRIVATE, sizeof(char) * size, IPC_CREAT | 0666);
    char* memory = (char*)shmat(shmid, NULL, 0);
    return memory;
}


void SharedMemoryAllocation()
{
    // random number generator
    srand(time(NULL));
    dispenser = createSharedMemoryChar(shmid_dispenser, DISPENSER_ROWS * DISPENSER_COLS);
    for (int i = 0; i < PLAYER_BOARDS; ++i)
    {
        playerBoard[i] = createSharedMemoryChar(shmid_playerBoard[i], PLAYER_BOARD_COLS);
    }
    playerScores = createSharedMemoryInt(shmid_playerScores, PLAYER_BOARDS);
    for (int i = 0; i < DISPENSER_ROWS * DISPENSER_COLS; ++i)
    {
        dispenser[i] = rand() % 4 + 'A'; // Using 'A', 'B', 'C', 'D' randomly as ingredients
    }
    for (int i = 0; i < PLAYER_BOARDS; ++i)
    {
        for (int j = 0; j < PLAYER_BOARD_COLS; ++j)
        {
            playerBoard[i][j] = ' ';
        }
    }
    for (int i = 0; i < PLAYER_BOARDS; ++i)
    {
        playerScores[i] = 0;
    }
}

//code for clearing the terminal
void clearTerminal()
{
    cout << "\033[2J\033[1;1H";
}




void displayDispenser()
{
    clearTerminal();
    pthread_mutex_lock(&mutex_dispenser);
    cout<< "\033[1;34m"<<"               PPPP   OOO   TTTTT III  OOO  N   N      EEEEE X   X PPPP  L     OOO   SSSS  III  OOO  N   N"<<endl;
    cout<<"               P   P O   O    T    I  O   O NN  N      E      X X  P   P L    O   O  s      I  O   O NN  N"<<endl;
    cout<<"               PPPP  O   O    T    I  O   O N N N      EEEEE   x   PPPP  L    O   O  SSSs   I  O   O N N N"<<endl;
    cout<<"               P     O   O    T    I  O   O N  NN      E      X X  P     L    O   O     S   I  O   O N  NN"<<endl;
    cout<<"               P      OOO     T   III  OOO  N   N      EEEEE X   X P     LLLL  OOO   SSSS  III  OOO  N   N"<< "\033[0m"<<endl;

   
    for(int i=0;i<5;i++)
    {
      cout<<endl;
    }
    cout <<setw(20)<< "\033[1;31m"<< "   •••••••••••••••••••••"<< "\033[0m" << endl;
    cout <<setw(20)<< "\033[1;31m"<< "   •     DISPENSER     •"<< "\033[0m" <<"               "<<" RULES TO PLAY GAME"<< endl;
    cout <<setw(20)<< "\033[1;31m"<< "   •••••••••••••••••••••"<< "\033[0m" << endl;
    cout<<"             "<<"     0   1   2   3   4   "<<endl;
    cout<<"             "<<"   ---------------------"<<endl;
    for (int i = 0; i < DISPENSER_ROWS; i++)
    {
        cout <<setw(15)<<i<<" | ";
        for (int j = 0; j < DISPENSER_COLS; j++)
        {
            char ingredient = dispenser[i * DISPENSER_COLS + j];
            // Add color codes for different ingredients
            switch (ingredient)
            {
            case 'A':
                cout << "\033[1;31m" << ingredient << "\033[0m"<<" | "; // Red
                break;
            case 'B':
                cout << "\033[1;33m" << ingredient << "\033[0m"<<" | "; // Yellow
                break;
            case 'C':
                cout << "\033[1;32m" << ingredient << "\033[0m"<<" | "; // Green
                break;
            case 'D':
                cout << "\033[1;34m" << ingredient << "\033[0m"<<" | "; // Blue
                break;
            default:
                cout << ingredient << " | ";
            }
        }
       
         if(i==0)
        {
           cout<<"               "<<"1:Select the ingredient from the dispenser by specifying row and column number. ";
        }
         if(i==1)
        {
           cout<<"               "<<"2:Ingredients will be selected and removed from dispenser";
        }
         if(i==2)
        {
           cout<<"               "<<"3:Selected ingredients will be add to player board.";
        }
         if(i==3)
        {
           cout<<"               "<<"4:You have to collect the ingredient to fill the flask.";
        }
         if(i==4)
        {
           cout<<"               "<<"  Every flask must contain the specific ingredient of quantity 3";
        }
         if(i==5)
        {
           cout<<"               "<<"6:Score will be increased by certain number each time player fills the flask.";
        }
        if(i==6)
        {
           cout<<"               "<<"7:at the end of the game player with more score wins";
        }
        cout << endl;
    }
    cout<<"             "<<"   ---------------------"<<endl;
    for(int i=0;i<3;i++)
    {
      cout<<endl;
    }
    pthread_mutex_unlock(&mutex_dispenser);
}

void displayPlayerInfo()
{
    for (int player = 0; player < PLAYER_BOARDS; player++)
    {
        pthread_mutex_lock(&mutex_playerBoard[player]);
        if (player % 2 == 0)
        {
            cout << "\033[1;35m"; // pink color for Player 1
        }
        else
        {
            cout << "\033[1;33m"; // Yellow color for Player 2
        }
        cout << "Player " << (player + 1) << "'s Potion: "<< "\033[0m";
        for (int j = 0; j < PLAYER_BOARD_COLS; j++)
  {
     cout << playerBoard[player][j] << " ";
  }
 
if(player>0)
{
           cout<<endl;
           cout << "\033[1;35m"<< "Score: " << playerScores[player-1] << "\033[0m"<<"                    ";
           cout << "\033[1;33m"<< "Score: " << playerScores[player] << "\033[0m"<<endl;
        }
        pthread_mutex_unlock(&mutex_playerBoard[player]);
    }
}

int PlayerMove(int player)
{
    int row,col;
    while (true)
    {
    cout<<endl;
    if(player==0)
    {
     cout << "Player " << "\033[1;35m"<< (player + 1)<< "\033[0m" << " turn "<<endl<<"enter row number (0-6): ";
     cin >> row;
     cout<<"enter col number (0-4): ";
     cin>>col;
    }
    else
    {
          cout << "Player " << "\033[1;33m"<< (player + 1)<< "\033[0m" << " turn"<<endl<<"enter row number (0-6): ";
          cin >> row;
     cout<<"enter col number (0-4): ";
     cin>>col;
        }
       
        if (row >= 0 && col>=0 && row <= DISPENSER_ROWS && col<=DISPENSER_COLS)
        {
            return (row*DISPENSER_COLS)+col;
        }
        else
        {
            cout << "Invalid move. Please enter a number of row and column" << endl;
        }
    }
}

void processMove(int move, int player)
{
    int col = move;
    int temp;
     if(dispenser[col]==' ')
    {
        cout << "Invalid move. Please choose a column with marbles." << endl;
    }
    else if (dispenser[col] != ' ')
    {
        char ingredient = dispenser[col];
        for (int i = 0; i < DISPENSER_ROWS; i++)
        {
          temp=col;
          if(dispenser[col]!= ' ')
          {
            dispenser[col] = dispenser[col-DISPENSER_COLS];
            col=col-DISPENSER_COLS;
          }
          if(col<0||dispenser[col]== ' ')
          {
            break;
          }
        }
        dispenser[temp] = ' ';
        for (int i = 0; i < PLAYER_BOARD_COLS; i++)
        {
           if (playerBoard[player][i] == ' ')
            {
                playerBoard[player][i] = ingredient;
                break;
            }
            if(i==PLAYER_BOARD_COLS-1)
            {
               checkReaction(player);
            }
            if(i==PLAYER_BOARD_COLS-1&&playerBoard[player][PLAYER_BOARD_COLS-1]==' ')
            {
               break;
            }
            if(i==PLAYER_BOARD_COLS-1)
            {
              for (int i = 0; i < PLAYER_BOARD_COLS; i++)
              {
                playerBoard[player][i] = ' ';
              }
              i=0;
              playerScores[player] -=1;
              playerBoard[player][0] = ingredient;
              break;
            }
        }
       
       
       
        checkReaction(player);
    }
   
}


void checkReaction(int player)
{
    int count = 0;
    char lastIngredient = ' ';
    for (int i = 0; i < PLAYER_BOARD_COLS; i++)
    {
        if (playerBoard[player][i] == ' ')
        {
            break;
        }
        if (playerBoard[player][i] == lastIngredient)
        {
            count++;
        }
        else
        {
            count = 1;
            lastIngredient = playerBoard[player][i];
        }      
        if (count >= 3)
        {
            // Removing ingredients from the player's board
            for (int j = i; j >= i - 2; j--)
            {
                playerBoard[player][j] = ' ';
            }
            if (lastIngredient == RED)
            {
                playerScores[player] += 2;
            }
            else if (lastIngredient == YELLOW)
            {
                playerScores[player] += 3;
            }
            else if (lastIngredient == BLUE)
            {
                playerScores[player] += 4;
            }
            else
            {
                playerScores[player] += 5;
            }
            checkReaction(player);
        }
    }
}

bool isGameOver()
{
    for (int j = 0; j < DISPENSER_COLS*DISPENSER_ROWS; j++)
    {
        if (dispenser[j] != ' ')
        {
            return false;
        }
    }
    return true;
}

void declareWinner()
{
    if (playerScores[0] > playerScores[1])
    {
        cout << "Player 1 wins!" << endl;
    }
    else if (playerScores[1] > playerScores[0])
    {
        cout << "Player 2 wins!" << endl;
    }
    else
    {
        cout << "It's a tie!" << endl;
    }
}

// Function for the game loop in a separate thread
void* continuegame(void* args)
{
    while (!isGameOver())
    {
        displayDispenser();
        displayPlayerInfo();
        int move1 = PlayerMove(0);
        processMove(move1, 0);
        displayDispenser();
        displayPlayerInfo();
        if (isGameOver())
        {
            break;
        }
        int move2 = PlayerMove(1);
        processMove(move2, 1);
    }
    declareWinner();
    shmdt(dispenser);
    for (int i = 0; i < PLAYER_BOARDS; ++i)
    {
        shmdt(playerBoard[i]);
    }
    shmdt(playerScores);
    shmctl(shmid_dispenser, IPC_RMID, NULL);
    for (int i = 0; i < PLAYER_BOARDS; ++i)
    {
        shmctl(shmid_playerBoard[i], IPC_RMID, NULL);
    }
    shmctl(shmid_playerScores, IPC_RMID, NULL);
    pthread_exit(NULL);
    return NULL;
}

int main()
{
    int mainToGamePipe[2]; // Pipe from main process to game loop
    int gameToMainPipe[2]; // Pipe from game loop to main process

    SharedMemoryAllocation();

    if (pipe(mainToGamePipe) == -1 || pipe(gameToMainPipe) == -1)
    {
        cerr << "Failed to create pipes." << endl;
        return 1;
    }

    // Pass pipe file descriptors to continue game
    PipeArgs pipeArgs;
    pipeArgs.mainToGamePipe[0] = mainToGamePipe[0];
    pipeArgs.mainToGamePipe[1] = mainToGamePipe[1];
    pipeArgs.gameToMainPipe[0] = gameToMainPipe[0];
    pipeArgs.gameToMainPipe[1] = gameToMainPipe[1];

    pthread_t thread;
    pthread_create(&thread, NULL, continuegame, (void*)&pipeArgs);
    pthread_join(thread, NULL);

    // Close pipe file descriptors after the game loop finishes
    close(mainToGamePipe[0]);
    close(mainToGamePipe[1]);
    close(gameToMainPipe[0]);
    close(gameToMainPipe[1]);

    return 0;
}
