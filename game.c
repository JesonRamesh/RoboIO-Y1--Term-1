// Used Windows
#include <ncurses/ncurses.h> // Make sure this is correct for your OS
#include <ncurses/curses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define BOARD_ROWS 20
#define BOARD_COLS 100 
#define MAX_NAME 20
#define ROBOT_BODY "o"
#define ROBOT_HEAD "^"
#define PERSON 'o'
#define MINE '.'
#define NEW_LIFE 'N'


// Structs
typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    Position pos;
    char direction;  // N, S, E, W
} Robot;

typedef struct {
    char name[MAX_NAME];
    int score;
    int lives;
    int level;
} Player;

typedef struct {
    char name[MAX_NAME];
    int score;
} Leaderboard;


// Function prototypes
WINDOW* init_game(Player player, Robot robot, Position person, Position *mines, int mine_count);  
void draw_title_screen(Player *player);
void draw_second_screen(Player *player);
void update_UI(Player player, Robot robot, Position person, Position *mines, int mine_count);
void handle_input(Robot *robot, int input, Position *person, Position *mines, int mine_count, WINDOW *board);
void move_robot(Robot *robot, Position *person);
void move_robot_ai(Robot *robot, Position *person, Position *mines, int mine_count);
void clear_robot(Robot *robot);
void clear_mines(Position *mines, int mine_count);
void clear_person(Position *person);
void draw_robot(Robot *robot, WINDOW* board);
int check_collision(Robot *robot, Position *mines, int mine_count);
void spawn_person(Position *person);
void spawn_mines(Position *mines, int *mine_count);
void game_over_screen(Player *player, Leaderboard *leaderboard);
void save_score(Player *player);
void show_leaderboard(Leaderboard *leaderboard);
void random_coordinates_mines(int mine_count, Position *mines, Position *person, Robot *robot);
void random_coordinates_person(int mine_count, Position *mines, Position *person, Robot *robot);
void sort_leaderboard(Leaderboard *leaderboard, int num);
int absolute_distance(int robot_x, int robot_y, int person_x, int person_y, int xmax, int ymax);
void draw_commander(int xmax, int ymax);
void draw_soldier(int xmax, int ymax);

int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    start_color();
    keypad(stdscr, TRUE);
    curs_set(0);
    srand(time(NULL));
    nodelay(stdscr, FALSE); 

    if (!has_colors()){
        addstr("Your system does not support colours!"); //Check if the system supports colors
        addstr("Press any key to continue playing...\n");
        getch();
        refresh(); 
    }

    // Initialize color pairs
    init_pair(1, COLOR_BLACK, COLOR_GREEN);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLACK, COLOR_RED);
    init_pair(4, COLOR_BLACK, COLOR_YELLOW);
    init_pair(5, COLOR_CYAN, COLOR_WHITE);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    

    // Initialize game variables 
    // TODO use DMA to allocate memory for these
    Player player = {.lives = 3, .score = 0, .level = 1};
    Robot robot = {.pos = {0, 0}, .direction = 'N'};
    Position person = {0, 0};
    Position new_life = {0, 0};
    Position *mines = (Position *)malloc(50 * sizeof(Position)); // Allocate memory for 50 mines
    if (mines == NULL){
        printw("Memory allocation failed!\n"); // Check if memory allocation failed
        return -1;
    }
    Leaderboard leaderboard;
    int collision = 0;
    int mine_count = 5;
    int delay = 250000;
    int min_delay = 5000;
    int ymax, xmax;

    

    getmaxyx(stdscr, ymax, xmax); // Get dimensions of the screen

    draw_title_screen(&player); //Draw title screen
    refresh();
    getch();
    clear(); // Clear title screen
    draw_second_screen(&player);
    refresh();
    getch();
    clear();

    nodelay(stdscr, TRUE); //Make getch blocking

    WINDOW* board = init_game(player, robot, person, mines, mine_count);
    random_coordinates_mines(mine_count, mines, &person, &robot);
    random_coordinates_person(mine_count, mines, &person, &robot);
    clear_robot(&robot); // Clear robot's position
   
    int ch;
    int flag_mines, flag_score = 0; // Variable to ensure that the mines spawn only once at each level
    int flag_newlife = 0;

    //Game loop
    while (1) {
        update_UI(player, robot, person, mines, mine_count);
        spawn_mines(mines, &mine_count);
        spawn_person(&person);
        
        refresh();
        
        //Check for level increment
        if (player.score != 0 && player.score % 5 == 0 && player.score != flag_score){
            player.level += 1;
            if (flag_newlife == 1){
                flag_newlife = 0;
            }
            flag_score = player.score;
            delay = delay / 2; // Increase the robot's speed (Increasing the speed by a small amount because the robot gets very fast during the initial stage)
            if (delay < min_delay){
                delay = min_delay; // Set delay to minimum delay as we've achieved the maximum speed of the system
            }
            if (mine_count <= 48){
                mine_count += 2; // Increase mine_count by 2
            } else {
                break; // Exit the loop as maximum score reached
            }
            clear_robot(&robot); // Bring robot to the center
            random_coordinates_mines(mine_count, mines, &person, &robot); // Generate new random cordinates for the mines
            wclear(stdscr);
            refresh();
            attrset(COLOR_PAIR(2));
            mvaddstr(ymax/2 - BOARD_ROWS/2 - 1, 10 + (xmax - BOARD_COLS)/2 + 25, "NEW LEVEL! Press any key to continue...");
            attroff(COLOR_PAIR(2));
            nodelay(stdscr, FALSE);
            getch();
            nodelay(stdscr, TRUE);
            clear();
            refresh();
        }

        //Change the position of the player after every two people saved
        if (player.score % 2 == 0 && player.score != 0 && flag_mines != player.score){
            clear_mines(mines, mine_count);
            random_coordinates_mines(mine_count, mines, &person, &robot);
            spawn_mines(mines, &mine_count);
            flag_mines = player.score;
            refresh();
        }

        // Handle input
        if (ch == 'q' || player.lives == 0){
            break; //break out of the loop 
        }

        ch = getch();
        handle_input(&robot, ch, &person, mines, mine_count,board);
        move_robot(&robot, &person);
        draw_robot(&robot, board);
        refresh();
        // Check for collision
        collision = check_collision(&robot, mines, mine_count);
        if (collision != 0){
            mvaddstr(ymax/2, xmax/2 - 20, "You lost a life! Press any key to continue playing!");
            clear_robot(&robot); // Reposition the robot to the center
            nodelay(stdscr, FALSE);
            getch();
            nodelay(stdscr, TRUE);
            clear();
            refresh();
            player.lives -= 1;
        }


        // If robot rescued a person, add points
        if (robot.pos.x + (xmax-BOARD_COLS)/2 + BOARD_COLS/2== person.x && robot.pos.y + (ymax-BOARD_ROWS)/2 + BOARD_ROWS/2 == person.y){
            player.score += 1;
            mvprintw(person.y, person.x, " "); // Replace the person with an empty character
            random_coordinates_person(mine_count, mines, &person, &robot);
            spawn_person(&person);
            refresh();
        }
        
        // Delay in microseconds
        usleep(delay);;
        WINDOW* board = init_game(player, robot, person, mines, mine_count);
    }

    // Wait for user input before exiting
    save_score(&player); // Save the score of the player to leaderboard.txt    

    nodelay(stdscr, FALSE);
    clear(); 
    refresh();
    game_over_screen(&player, &leaderboard);//Display the exit screen
    //getch();
    //nodelay(stdscr, TRUE);
    free(mines); // Free mines before exiting the program
    
    
    // Cleanup and exit
    endwin();
    return 0;
}

void draw_title_screen(Player *player) {
    //Display instructions
    clear(); // Clear the current screen 

    int ymax, xmax;
    getmaxyx(stdscr, ymax, xmax); // Get dimesion of the screen

    // Instructions for the game
    mvaddstr((ymax-BOARD_ROWS)/2, (xmax-BOARD_ROWS)/2 - 15, "Welcome to RoboIO!\n");
    mvaddstr((ymax-BOARD_ROWS)/2 + 1, (xmax-BOARD_ROWS)/2 - 15, "This is a brand new world where Robots coexist with Humans.\n");
    //mvaddstr((ymax-BOARD_ROWS)/2 + 2, (xmax-BOARD_ROWS)/2 - 15, "There'll be mines all over the battlefield. Make sure you don't run into them\n");
    mvaddstr((ymax-BOARD_ROWS)/2 + 2, (xmax-BOARD_ROWS)/2 - 15, "Let's see what you've got in you!\n\n");
 
    mvaddstr((ymax-BOARD_ROWS)/2 + 4, (xmax-BOARD_ROWS)/2 - 15, "Before we get things started, let's build our own robot!\n");

    //Ask for player's name
    mvaddstr((ymax-BOARD_ROWS)/2 + 5, (xmax-BOARD_ROWS)/2 - 15, "What is the name of your robot?\n");
    wmove(stdscr, (ymax-BOARD_ROWS)/2 + 6, (xmax-BOARD_ROWS)/2 - 15);
    wgetnstr(stdscr, player->name, MAX_NAME-1);

    wrefresh(stdscr);
    
    // Initialize the player's score, lives remaining and level.
    player->score = 0;
    player->lives = 3;
    player->level = 0;

    attrset(COLOR_PAIR(2));
    mvprintw((ymax-BOARD_ROWS)/2 + 7,(xmax-BOARD_ROWS)/2 - 15, "Welcome to the game agent %s.\n", player->name);
    mvprintw((ymax-BOARD_ROWS)/2 + 8,(xmax-BOARD_ROWS)/2 - 15, "You are one of the best and the most notorious soldier the world has ever seen\n");
    mvprintw((ymax-BOARD_ROWS)/2 + 9,(xmax-BOARD_ROWS)/2 - 15, "Your help might be needed anytime so STAY ALERT!");
    attroff(COLOR_PAIR(2));
    mvaddstr((ymax-BOARD_ROWS)/2 + 10,(xmax-BOARD_ROWS)/2 - 15, "Press any key to continue...");
    refresh();
}

void draw_second_screen(Player *player){
    //clear();
    int xmax, ymax;
    getmaxyx(stdscr, ymax, xmax);
    int flag_animation = 0;
    
    draw_soldier(xmax, ymax);
    draw_commander(xmax, ymax);
    attrset(COLOR_PAIR(2));
    mvprintw((ymax-BOARD_ROWS)/2 + 4,(xmax-BOARD_ROWS)/2 - 15, "SOLDIER:   COMMANDER! COMMMADER! OUR PEOPLE...\n");
    attroff(COLOR_PAIR(2));
    getch();
    mvprintw((ymax-BOARD_ROWS)/2 + 5,(xmax-BOARD_ROWS)/2 - 15, "COMMANDER: WHAT IS IT !?\n");
    getch();
    attrset(COLOR_PAIR(2));
    mvprintw((ymax-BOARD_ROWS)/2 + 6,(xmax-BOARD_ROWS)/2 - 15, "SOLDIER :  WE'VE BEEN LED TO A TRAP! OUR PEOPLE ARE IN THE MIDDLE OF A MINEFIELD! \n");
    attroff(COLOR_PAIR(2));
    getch();
    mvprintw((ymax-BOARD_ROWS)/2 + 7,(xmax-BOARD_ROWS)/2 - 15, "COMMANDER: WHAT!? DID YOU CONTACT THE RESCUE TEAM?\n");
    getch();
    attrset(COLOR_PAIR(2));
    mvprintw((ymax-BOARD_ROWS)/2 + 8,(xmax-BOARD_ROWS)/2 - 15, "SOLDIER:   BUT...CHIEF, THEY'RE ALL TOO SCARED TO ENTER THE FIELD. NOBODY WANTS TO DO IT\n");
    attroff(COLOR_PAIR(2));
    getch();
    mvprintw((ymax-BOARD_ROWS)/2 + 9,(xmax-BOARD_ROWS)/2 - 15, "COMMANDER: ARE YOU SERIOUS? OUR MEN ARE NOTHING BUT MERE COWARDS...I'M ASHAMED OF THEM\n");
    getch();
    attrset(COLOR_PAIR(2));
    mvprintw((ymax-BOARD_ROWS)/2 + 10,(xmax-BOARD_ROWS)/2 - 15, "SOLDIER:   UHM...CHIEF...you wouldn't go there yourself..will you?\n");
    attroff(COLOR_PAIR(2));
    getch();
    mvprintw((ymax-BOARD_ROWS)/2 + 11,(xmax-BOARD_ROWS)/2 - 15, "COMMANDER: WAIT! WHAT DID YOU SAY!?\n");
    getch();
    attrset(COLOR_PAIR(2));
    mvprintw((ymax-BOARD_ROWS)/2 + 12,(xmax-BOARD_ROWS)/2 - 15, "SOLDIER:   NOTHING! NOTHING CHIEF!.\n");
    attroff(COLOR_PAIR(2));
    getch();
    mvprintw((ymax-BOARD_ROWS)/2 + 13,(xmax-BOARD_ROWS)/2 - 15, "COMMANDER: ALRIGHT I GUESS IT'S TIME TO CALL HIM THEN\n");
    getch();
    attrset(COLOR_PAIR(2));
    mvprintw((ymax-BOARD_ROWS)/2 + 14,(xmax-BOARD_ROWS)/2 - 15, "SOLDIER:   NO! ARE YOU SURE ABOUT THAT!?\n");
    attroff(COLOR_PAIR(2));
    getch();
    mvprintw((ymax-BOARD_ROWS)/2 + 15,(xmax-BOARD_ROWS)/2 - 15, "COMMANDER: YES I AM, WE DON'T HAVE ANY OTHER OPTIONS DO WE? \n");
    getch();
    attrset(COLOR_PAIR(2));
    mvprintw((ymax-BOARD_ROWS)/2 + 16,(xmax-BOARD_ROWS)/2 - 15, "SOLDIER:   WELL, THINK ABOUT IT TWICE CHIEF\n");
    attroff(COLOR_PAIR(2));
    getch();
    mvprintw((ymax-BOARD_ROWS)/2 + 17,(xmax-BOARD_ROWS)/2 - 15, "''Commander picks up his phone and makes a call''\n");
    mvprintw((ymax-BOARD_ROWS)/2 + 18,(xmax-BOARD_ROWS)/2 - 15, "COMMANDER: HELLO %s, I THINK IT'S TIME...\n", player->name);
    getch();
    attrset(COLOR_PAIR(2));
    mvprintw((ymax-BOARD_ROWS)/2 + 20,(xmax-BOARD_ROWS)/2 - 15, "***********************%s DEPLOYED!************************\n", player->name);
    attroff(COLOR_PAIR(2));

    getch();
    mvprintw((ymax-BOARD_ROWS)/2 + 22,(xmax-BOARD_ROWS)/2 - 15, "1. Save as many people as you can while avoiding the mines");
    getch(); 
    mvprintw((ymax-BOARD_ROWS)/2 + 23,(xmax-BOARD_ROWS)/2 - 15, "2. The number of mines increase after each level");  
    getch();
    mvprintw((ymax-BOARD_ROWS)/2 + 24,(xmax-BOARD_ROWS)/2 - 15, "3. Mines change positions after every 2 people you save");
    getch();
    mvprintw((ymax-BOARD_ROWS)/2 + 25,(xmax-BOARD_ROWS)/2 - 15, "Press any key to start playing..."); 

    refresh();

}

WINDOW* init_game(Player player, Robot robot, Position person, Position *mines, int mine_count) {
    // Initialize the board
    int ymax, xmax;
    getmaxyx(stdscr, ymax, xmax);
    WINDOW* board = newwin(BOARD_ROWS, BOARD_COLS, (ymax-BOARD_ROWS)/2, (xmax-BOARD_COLS)/2);
    box(board, 0, 0);
    refresh();
    wrefresh(board);
    // Update the UI
    update_UI(player, robot, person, mines, mine_count);
    return board;
}

void update_UI(Player player, Robot robot, Position person, Position *mines, int mine_count) {
    int xmax, ymax;
    getmaxyx(stdscr, ymax, xmax);
    
    int length = strlen(player.name); // A length variable to adjust the positioning of each printed text.
    mvwprintw(stdscr, 1, (xmax - BOARD_COLS)/2 + 30 , "PLAYER: %s", player.name);
    mvwprintw(stdscr, 1, 39 + length + (xmax - BOARD_COLS)/2, "LIVES: %d", player.lives);
    mvwprintw(stdscr, 1, 49 + length + (xmax - BOARD_COLS)/2, "SCORE: %d", player.score);
    mvwprintw(stdscr, 1, 59 + length + (xmax - BOARD_COLS)/2, "LEVEL: %d", player.level + 1);


    wrefresh(stdscr);
}

void handle_input(Robot *robot, int input, Position *person, Position *mines, int mine_count, WINDOW *board) {
    switch(input) {
        // Change the direction of the robot according to the key pressed by the user
        case KEY_UP:
            robot -> direction = 'N';
            break;
        case KEY_DOWN:
            robot -> direction = 'S';
            break;
        case KEY_LEFT:
            robot -> direction = 'W';
            break;
        case KEY_RIGHT:
            robot -> direction = 'E';
            break;
        default:
            // Auto movement if no key pressed
            move_robot_ai(robot, person, mines, mine_count);
    }
}

void move_robot(Robot *robot, Position *person) {
    // Move robot according to its direction
    switch(robot->direction){
        case('N'):
            robot -> pos.y -= 1;
            break;
        case ('S'):
            robot -> pos.y += 1;
            break;
        case ('W'):
            robot -> pos.x -= 1;
            break;
        case ('E'):
            robot -> pos.x += 1;
            break;
    }
}

void move_robot_ai(Robot *robot, Position *person, Position *mines, int mine_count) {
    // Target for the robot to reach
    int x_target = person -> x;
    int y_target = person -> y;
    int new_distance, new_x, new_y, xmax, ymax, i, j;
    int flag_mines = 0;
    char flag_direction = '\0'; // To store the direction that leads to the person
    getmaxyx(stdscr, ymax, xmax);

    int possible_movement[4][2] = {{0,-1}, {1,0}, {0,1}, {-1,0}}; // Possible (x,y) movements of the robot
    char possible_directions[4] = {'N', 'E', 'S', 'W'}; // Directions of the corresponding movements

    char previous_opposite_direction = '\0';

    // Check for the direction opposite to the robot's previous direction
    switch (robot->direction){
        case 'N':
            previous_opposite_direction = 'S';
            break;
        case 'S':
            previous_opposite_direction = 'N';
            break;
        case 'E':
            previous_opposite_direction = 'W';
            break;
        case 'W':
            previous_opposite_direction = 'E';
            break;
    }
    

    int distance = INT_MAX; // Sets the safe distance to the highest possible integer number

    // Iterate through each possible movement
    for (i = 0; i < 4; i++){
        flag_mines = 0;
        new_x = robot -> pos.x + possible_movement[i][0];
        new_y = robot -> pos.y + possible_movement[i][1];
        // Check if the movement leads to a wall crash
        if (new_x != BOARD_COLS/2 && new_x != -BOARD_COLS/2 && new_y != BOARD_ROWS/2 && new_y != -BOARD_ROWS/2){ 
            for (j = 0; j < mine_count; j++){
                // Check if the movement leads to collision with a mine
                if (new_x + BOARD_COLS/2 + (xmax-BOARD_COLS)/2 == mines[j].x && new_y + BOARD_ROWS/2 + (ymax-BOARD_ROWS)/2 == mines[j].y){ 
                    flag_mines = 1; // Movement is unsafe
                } 
            }    

            // If mines are not detected and the robot is not moving back and forth
            if (flag_mines == 0 && possible_directions[i] != previous_opposite_direction){
                new_distance = absolute_distance(new_x, new_y, x_target, y_target, xmax, ymax);
                // Check if the new distance is smaller than the old distance
                if (new_distance < distance){ 
                    distance = new_distance; // Change old distance to the new distance
                    flag_direction = possible_directions[i]; // Set the flag to the new direction corresponding to the movement
                } 
            }
        }
    }

    // If flag is a null character, the robot travels in its original path
    if (flag_direction == '\0'){
        mvwprintw(stdscr, 14, 11 + 5 + (xmax - BOARD_COLS)/2, "No valid move. Staying in place.\n");
        flag_direction = robot -> direction;
    }
    robot -> direction = flag_direction; // Change direction of the robot accordingly
}

void clear_robot(Robot *robot) {
    // Reset robot's position to zero
    robot -> pos.x = 0;
    robot -> pos.y = 0;
}

void clear_mines(Position *mines, int mine_count){
    // Erase mine locations
    for (int i = 0; i < mine_count; i++){
        mvprintw(mines[i].y, mines[i].x, " ");
    }
    refresh();
}

void clear_person(Position *person){
    // Reset person's position to 0
    person -> x = 0;
    person -> y = 0;
}

void draw_robot(Robot *robot, WINDOW* board) {
    wattron(board, COLOR_PAIR(2)); // Apply color
    switch (robot -> direction){
        case ('N'):
            mvwprintw(board, robot->pos.y + BOARD_ROWS/2 - 0.5, robot->pos.x + BOARD_COLS/2, "^");
            break;
        case ('S'):
            mvwprintw(board, robot->pos.y + BOARD_ROWS/2 + 1, robot->pos.x + BOARD_COLS/2, "v");
            break;
        case ('E'):
            mvwprintw(board, robot->pos.y + BOARD_ROWS/2, robot->pos.x + BOARD_COLS/2 + 1, ">");
            break;
        case ('W'):
            mvwprintw(board, robot->pos.y + BOARD_ROWS/2, robot->pos.x + BOARD_COLS/2 - 1, "<");
            break;
    }
    wattron(board, COLOR_PAIR(2));
    wattron(board, COLOR_PAIR(1)); // Apply color
    mvwprintw(board, robot->pos.y + BOARD_ROWS/2, robot->pos.x + BOARD_COLS/2, ROBOT_BODY); // The board starts at (0,0) therefore you locate the center and add the postion of the robot to those values
    wattroff(board, COLOR_PAIR(1));
    wrefresh(board);
}

int check_collision(Robot *robot, Position *mines, int mine_count) {
    int ymax, xmax;
    getmaxyx(stdscr, ymax, xmax);
    srand(time(NULL));
    if (robot->pos.x == BOARD_COLS/2 || robot->pos.x == -BOARD_COLS/2 || robot->pos.y == BOARD_ROWS/2 || robot->pos.y == -BOARD_ROWS/2){ 
        return 1; // Collision with wall
    } 

    for (int i = 0; i < mine_count; i++){
        if (robot->pos.x + BOARD_COLS/2 == mines[i].x - (xmax-BOARD_COLS)/2 && robot->pos.y + BOARD_ROWS/2 == mines[i].y - (ymax-BOARD_ROWS)/2){
            return 2;
        } // Collision with mines
    }
    
    return 0;
}

void spawn_person(Position *person) {
    attrset(COLOR_PAIR(4)); // Apply colours
    mvaddch(person->y, person->x, PERSON); // Print person on the screen
    attroff(COLOR_PAIR(4));
}

void spawn_mines(Position *mines, int *mine_count) {
    for (int i = 0; i < *mine_count; i++){
        attrset(COLOR_PAIR(3)); // apply colors
        mvaddch(mines[i].y, mines[i].x, MINE); //print mines
        attroff(COLOR_PAIR(3));
    }
}

void game_over_screen(Player *player, Leaderboard *leaderboard) {
    clear(); //Clear the current screen
    refresh();

    int ymax, xmax;
    getmaxyx(stdscr, ymax, xmax);

    attrset(COLOR_PAIR(6));
    // Display score and final message
    if (player->level < 23){
        mvprintw((ymax-BOARD_ROWS)/2,(xmax-BOARD_ROWS)/2 - 10,"%s WAS DESTROYED!\n", player -> name);
        mvprintw((ymax-BOARD_ROWS)/2+1, (xmax-BOARD_ROWS)/2 - 10, "BUT, GREAT JOB CHAMP! YOU SAVED %d PEOPLE\n", player -> score);
    } else {
        mvprintw((ymax-BOARD_ROWS)/2,(xmax-BOARD_ROWS)/2 - 10,"LET'S GO CAPTAIN! \n");
        mvprintw((ymax-BOARD_ROWS)/2+1, (xmax-BOARD_ROWS)/2 - 10, "BUT, GREAT JOB CHAMP! YOU SAVED %d PEOPLE\n", player -> score);

    }
    

    mvaddstr((ymax-BOARD_ROWS)/2 + 5, (xmax-BOARD_ROWS)/2 - 10, "Here's the leaderboard:\n");
    
    show_leaderboard(leaderboard);
    
    mvaddstr((ymax-BOARD_ROWS)/2+3, (xmax-BOARD_ROWS)/2 - 10, "Press any key to exit...\n");
    attroff(COLOR_PAIR(6));
    refresh();

    getch();
    
}

void save_score(Player *player) {
    FILE *file_pointer;
    file_pointer = fopen("leaderboard.txt", "a");

    if (file_pointer == NULL){
        printw("Error accessing the leaderboard file!\n");
        exit(EXIT_FAILURE); // Exit if file cannot be accessed
    }

    fprintf(file_pointer, "%s\n%d\n", player->name, player->score); // Print player's name and score to the leaderboard text file.
    fclose(file_pointer);
}

void show_leaderboard(Leaderboard *leaderboard) {
    int xmax, ymax;
    getmaxyx(stdscr, ymax, xmax);
    int num = 20; // Number of player details to be stored
    int count = 0; // Keep track of the number of players
    leaderboard = (Leaderboard *)malloc(num * sizeof(Leaderboard));

    if (leaderboard == NULL){
        printw("Memory Allocation Failes!\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num; i++){
        leaderboard[i].score = 0;
    }

    FILE *file_pointer;
    file_pointer = fopen("leaderboard.txt", "r");

    if (file_pointer == NULL){
        printw("File cannot be found!\n");
        exit(EXIT_FAILURE); //Exit if file cannot be accessed
    }

    char line[256]; // Buffer to store the string from a line
    while (fgets(line, sizeof(line), file_pointer)){
        strncpy(leaderboard[count].name, line, sizeof(leaderboard[count].name)-1); // Store players name to the array of leaderboard structures
        leaderboard[count].name[sizeof(leaderboard[count].name) - 1] = '\0'; // Add a null character in the end to terminate string.

        if (fgets(line, sizeof(line), file_pointer)){ // Next line
            leaderboard[count].score = atoi(line); // Convert string to integer 
        } else {
            printw("Player's score mising\n");
            break;
        }

        count++;

        //if memory is full, reallocate memory
        if (count == num){
            num += num; // Increase the quantity
            leaderboard = (Leaderboard *)realloc(leaderboard, num*sizeof(Leaderboard)); // Reallocate memory
            if (leaderboard == NULL){
                printw("Memory Allocation Failed!\n");
                exit(EXIT_FAILURE); // Exit if memory allocation failed
            }
            for (int i = 0; i < num; i++){
                leaderboard[i].score = 0;
            }
        }
    }
    fclose(file_pointer);

    //Sorting the scores using bubble sort algorithm
    sort_leaderboard(leaderboard, num);

    // Print top 10
    attrset(COLOR_PAIR(5));
    mvprintw((ymax-BOARD_ROWS)/2 + 7, (xmax-BOARD_ROWS)/2 - 10, "| Position |");
    mvprintw((ymax-BOARD_ROWS)/2 + 7, (xmax-BOARD_ROWS)/2 + 4, "|       Name       |");
    mvprintw((ymax-BOARD_ROWS)/2 + 7, (xmax-BOARD_ROWS)/2 + 26, "| Score |");
    attroff(COLOR_PAIR(5));


    if (num > 10){
        for (int i = 0; i < 10; i++){
            mvprintw((ymax-BOARD_ROWS)/2 + 8+i, (xmax-BOARD_ROWS)/2 - 5, "%d", i+1);
            mvprintw((ymax-BOARD_ROWS)/2 + 8+i, (xmax-BOARD_ROWS)/2 + 5, "%s", leaderboard[i].name);
            mvprintw((ymax-BOARD_ROWS)/2 + 8+i, (xmax-BOARD_ROWS)/2 + 31, "%d", leaderboard[i].score);
        }
    } else {
        for (int i = 0; i < num; i++){
            printw("%d. ", i + 1);
            printw("%s %d\n", leaderboard[i].name,leaderboard[i].score);
        }
    }

    wrefresh(stdscr);
    free(leaderboard);
}

void random_coordinates_mines(int mine_count, Position *mines, Position *person, Robot *robot){
    int ymax, xmax;
    getmaxyx(stdscr, ymax, xmax);
    srand(time(NULL));
    for (int i = 0; i < mine_count; i++){
        do{
            mines[i].x = (xmax-BOARD_COLS)/2 + 3 + rand() % (BOARD_COLS - 4); //Random x position within walls
            mines[i].y = (ymax-BOARD_ROWS)/2 + 3 + rand() % (BOARD_ROWS - 4); //Random y position within walls
        } while ((mines[i].x == person->x && mines[i].y == person->y) || (mines[i].x == (xmax-BOARD_COLS)/2 + BOARD_COLS/2 + robot->pos.x && mines[i].y == (ymax-BOARD_ROWS)/2 + BOARD_ROWS + robot->pos.y)); //Condition to ensure that the mine doesn't spawn on the person or the robot.
    }
}

void random_coordinates_person(int mine_count, Position *mines, Position *person, Robot *robot){
    int ymax, xmax;
    getmaxyx(stdscr, ymax, xmax);
    int flag1, flag2 = 0;
    srand(time(NULL));

    while (flag2 == 0){
        person->x = (xmax-BOARD_COLS)/2 + 2 + rand() % (BOARD_COLS-4); //Random x position within walls
        person->y = (ymax-BOARD_ROWS)/2 + 2 + rand() % (BOARD_ROWS-4); //Random y position within walls

        if (person->x == (xmax-BOARD_COLS)/2 + BOARD_COLS/2 + robot->pos.x && person->y == (ymax-BOARD_ROWS)/2 + BOARD_ROWS/2 + robot->pos.y){
            flag1 = 1; // Check if the person spawns at the robot location
        }

        for (int i = 0; i < mine_count; i++){
            if (person->x == mines[i].x && person->y == mines[i].y){
                flag1 = 1; // Check if the person spawns at a mine location
            }
        }

        if (flag1 != 1){
            flag2 = 1;
        } else {
            continue;
        }
    }
}

void sort_leaderboard(Leaderboard *leaderboard, int num){
    // Bubble sort the leaderboard array
    char temp_name[MAX_NAME];
    int temp_score;
    for (int i = 0; i < num; i++){
        for (int j = 0; j < num - i - 1; j++){
            if (leaderboard[j].score < leaderboard[j+1].score){
                // Swap names and scores
                strcpy(temp_name,leaderboard[j].name);
                temp_score = leaderboard[j].score;
                strcpy(leaderboard[j].name,leaderboard[j+1].name);
                leaderboard[j].score = leaderboard[j+1].score;
                strcpy(leaderboard[j+1].name,temp_name);
                leaderboard[j+1].score = temp_score;
            }
        }
    }
}

int absolute_distance(int robot_x, int robot_y, int person_x, int person_y, int xmax, int ymax){
    return abs((robot_x + (xmax-BOARD_COLS)/2 + BOARD_COLS/2) - person_x) + abs((robot_y + (ymax-BOARD_ROWS)/2 + BOARD_ROWS/2) - person_y); // Add the total distance to travel in x and y directions
    
}

void draw_commander(int xmax, int ymax){

    // Draw commander
    mvprintw((ymax-BOARD_ROWS)/2 - 6, (xmax-BOARD_ROWS)/2 - 10, " _______");
    mvprintw((ymax-BOARD_ROWS)/2 - 5, (xmax-BOARD_ROWS)/2 - 10, " |_____|");
    mvprintw((ymax-BOARD_ROWS)/2 - 4, (xmax-BOARD_ROWS)/2 - 10, "  (^ ^)");
    mvprintw((ymax-BOARD_ROWS)/2 - 3, (xmax-BOARD_ROWS)/2 - 10, "  ( O )");
    mvprintw((ymax-BOARD_ROWS)/2 - 2, (xmax-BOARD_ROWS)/2 - 10, "||-----||");
    mvprintw((ymax-BOARD_ROWS)/2 - 1, (xmax-BOARD_ROWS)/2 - 10, "||  '  ||");
    mvprintw((ymax-BOARD_ROWS)/2 + 0, (xmax-BOARD_ROWS)/2 - 10, "||  '  ||");
    mvprintw((ymax-BOARD_ROWS)/2 + 1, (xmax-BOARD_ROWS)/2 - 10, " |-----|");
    mvprintw((ymax-BOARD_ROWS)/2 + 2, (xmax-BOARD_ROWS)/2 - 10, " -------");


    wrefresh(stdscr);
}

void draw_soldier(int xmax, int ymax){
    // Draw soldier
    attrset(COLOR_PAIR(2));
    mvprintw((ymax-BOARD_ROWS)/2 - 6, (xmax-BOARD_ROWS)/2 + 10, "   _____");
    mvprintw((ymax-BOARD_ROWS)/2 - 5, (xmax-BOARD_ROWS)/2 + 10, "   |___|");
    mvprintw((ymax-BOARD_ROWS)/2 - 4, (xmax-BOARD_ROWS)/2 + 10, "   (* *)");
    mvprintw((ymax-BOARD_ROWS)/2 - 3, (xmax-BOARD_ROWS)/2 + 10, "   ( O )");   
    mvprintw((ymax-BOARD_ROWS)/2 - 2, (xmax-BOARD_ROWS)/2 + 10, "   -----");
    mvprintw((ymax-BOARD_ROWS)/2 - 1, (xmax-BOARD_ROWS)/2 + 10, " /| ~~~ |\\");
    mvprintw((ymax-BOARD_ROWS)/2 + 0, (xmax-BOARD_ROWS)/2 + 10, " || ~~~ ||");
    mvprintw((ymax-BOARD_ROWS)/2 + 1, (xmax-BOARD_ROWS)/2 + 10, " || ~~~ ||");
    mvprintw((ymax-BOARD_ROWS)/2 + 2, (xmax-BOARD_ROWS)/2 + 10, " ---------");
    attroff(COLOR_PAIR(2));
    wrefresh(stdscr);
}
