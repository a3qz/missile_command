// Missile Command
//Ryan Michalec and Jessica Cioffi
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "gfx4.h"
#define MAX_MISSILES 100
#define XSIZE 800
#define YSIZE 600

/*
Structs
*/

typedef struct Missile { // what is falling from the sky
  double start_x;
  double start_y;
  double current_pos_x;
  double current_pos_y;
  double end_x;
  double end_y;
  double delta_x;
  double delta_y;
  int status;  // 1 for alive, 0 for has been killed
} Missile_t;

typedef struct Bomb{ // what the user is launching
  int x_pos;
  int y_pos;
  double rad;
  int start_time;
  double delta_rad;
  int status;
} Bomb_t;

/*
Function Prototypes
*/

void draw_base(int level); // Draw the level
void gameover_screen(); // Game Over screen display
void init_structs(int level, Missile_t stock[MAX_MISSILES], Bomb_t shots[MAX_MISSILES]); // Initialize structs using random starting and ending positions
void missile_position_calc(Missile_t stock[MAX_MISSILES]); // Calculate the current position of each missile as a function of time
int count_missiles(); // Count how many missiles are remaining
void draw_missiles(Missile_t stock[MAX_MISSILES]); // Draw the missiles
int num_missiles(int level); // Define the max number of missiles per level
int title_screen(); // Title Screen Display
void check_missile_pos(int cities[6], Missile_t stock[MAX_MISSILES]); // Checks if missile is low enough to destroy a city
void check_city_status(int cities[6], Missile_t stock[MAX_MISSILES], int i); // Check if city is killed & set that it is dead
void draw_pause_menu(); // Draw pause menu
void reset_pause_menu(); // Clear space that was the pause menu
void check_missile_status(Missile_t stock[MAX_MISSILES]); // Check if missiles are too low
void draw_cities(int city_mid[6]); // Draws all the cities
void draw_city(int x, int b); // Draw one city at given x
void blot_cities(int cities[6], int city_mid[6]); // Draws black square over dead cities 
void draw_score(); // Display the score 
void inspect_damages(int mouse_pos_x, int mouse_pos_y); // Using same radius as circle, use distance formula to find distance of tip of each missile at current time t
void get_mouse_pos(int mouse_pos[2]); // get mouse position
int distance(double x1, double y1, double x2, double y2); // Literally just a distance formula function
int starting_y(int level, int i); // Calculate initial y positions
int cities_remaining(int cities[6]); // Counts cities left  (for seeing how many cities are remining for quit case)
void level_screen(int level); // Draws display between levels

/*
Global Variables
*/

double t = 0;
int score = 0;

/*
Main
*/

int main(){

  /*
  Declarations
  */

  int cities[6] = {1,1,1,1,1,1}; // Initializes all six cities as 1 (Live)
  int city_mid[6] = {105, 186, 265, 530, 612, 695}; // Initializes the midpoints of each of the cities
  int mouse_pos[2]; // Mouse coordinates x and y
  Missile_t stock[MAX_MISSILES];
  Bomb_t shots[25];

  srand (time(NULL));
  int missiles_remaining = count_missiles();

  char c, d;
  int l, k, j = 0, x = 1, level = -1, looper, shots_fired;

  gfx_open(XSIZE, YSIZE, "Missile Command"); // Opens the game window
  looper = title_screen();  // put clear in title screen

  /*
  Game
  */

  while(looper){
    if(gfx_event_waiting()){
    } 

    level++; 
    if(level > 0){
      score += cities_remaining(cities)*100;  // Adds to your score 100 points for every city left alive at the end of the level
    }
    level_screen(level);
    gfx_clear();
    init_structs(level, stock, shots);
    shots_fired = 0;  // Tracks position in array of bombs fired
    draw_cities(city_mid);
    draw_base(level);

      while(count_missiles(stock) > 0){
        
        c = 10;
        missile_position_calc(stock);
        draw_missiles(stock);
        check_missile_status(stock);
        check_missile_pos(cities, stock);
        blot_cities(cities, city_mid);   

        c = 'd'; 
        if (gfx_event_waiting()){   // Checks for a mouseclick (shot being fired) or escape key (pause menu)
          c = gfx_wait();
          switch(c){
            case 1: // If mouseclick, adds point to array of shot structs  
              get_mouse_pos(mouse_pos);

              shots[shots_fired].x_pos = mouse_pos[0];  // Stores position of mouseclick
              shots[shots_fired].y_pos = mouse_pos[1];  // Stores position of mouseclick
              shots[shots_fired].rad = 0;    // Initial radius of explosion
              shots[shots_fired].delta_rad = .25;     // Initial change in radius of explosion
              shots[shots_fired].status = 1;    // Tells program to look for this element of the array
              shots_fired++;    // Moves to next position of array for next shot
              if (shots_fired == 9){ // puts array back at start
                shots_fired = 0;
              }
              break;
            case 27: // If escape enters pause menu - exited with 1 or escape, 2 starts new game, 3 quits program
              c = 10;
              draw_pause_menu();
              x = 1;
              while(x){
                d = gfx_wait();
                switch(d){
                  
                  case '1': // Continue
                    x = 0;
                    reset_pause_menu();
                    break;  
                  case 27:  // Continue
                    x = 0;
                    reset_pause_menu();
                    break; 
                  case '2': // Restart game
                    score = 0;
                    title_screen();   // t = -1;
                    t = 0;
                    t--;
                    x = 0;
                    for(k = 0; k < MAX_MISSILES; k++ ){ // Resets missiles so level will exit
                      stock[k].status = 0;
                    }
                    for(k = 0; k < 6; k++){ // Turns all cities back on so will not game over
                      cities[k] = 1;
                    }
                    
                    level = -1;
                    break;
                  case '3':
                    return 0; 
                }
              }
          }
        }


        for(j = 0; j < 9; j++){  // Checker to animate bombs and delete missiles if in radius
          if(shots[j].status == 1){   // Only explodes if bomb is set to on in previous loop
            if (shots[j].y_pos >= 500) { // If too low to shoot at, turns it off
              shots[j].status= 0;
              continue;
            }
            shots[j].rad += shots[j].delta_rad; // Increments the radius of the explosion
            if(shots[j].rad == 30){   // If at max reverses the change in radius
              shots[j].delta_rad *= -1;
            }
            gfx_color(255, 153, 80);
            gfx_line(400, 514, shots[j].x_pos, shots[j].y_pos); // Draws a line from gun tower to explosion
            if(shots[j].delta_rad > 0){  // Alternates color of explosion from yellow to orange for effect 
                if((int)shots[j].rad%2) {
                    gfx_color(255, 255, 0);
                }
                else {
                    gfx_color(255, 153, 51);
                 }
            }
            if(shots[j].delta_rad < 0){
              gfx_color(0,0,0);
            }
            gfx_circle(shots[j].x_pos, shots[j].y_pos, shots[j].rad); // Draws circle around point of explosion (circle used over fill_circle intentionally for effect)
            
            if(shots[j].rad <= 0){
              shots[j].status = 0;
              gfx_color(0,0,0);
              gfx_line(400, 514, shots[j].x_pos, shots[j].y_pos);  // Erases line from tower to explosion when explosion is over
            }
            for(l = 0; l < MAX_MISSILES; l++){  // Goes through the array of missiles to check if any are in the range of the explosion, turns them off it not
              if(stock[l].status == 1){
                if(distance(stock[l].current_pos_x, stock[l].current_pos_y, shots[j].x_pos, shots[j].y_pos) <= shots[j].rad){
                  stock[l].status = 0;
                  gfx_color(0,0,0);
                  gfx_line(stock[l].current_pos_x, stock[l].current_pos_y, stock[l].start_x, stock[l].start_y);  // Erases trail of the missile
                  score = score + 25;  // Boosts the player's score on a successful shot
                }
              }
            
            }
          } 
        }
        if(cities_remaining(cities) ==  0){  // Checks how many cities are alive, if zero, does gameover
          gameover_screen();
          draw_score(score);
          score = 0;
            x = 1;
          while(x){   // Operates menu on gameover screen - 1 is new game, 2 is exit program
            c = 10;
            c = gfx_wait();
            if(c == '1'){  
              title_screen();
              x = 0;
              for(k = 0; k < MAX_MISSILES; k++){
                stock[k].status = 0;
              }
              for(k = 0; k < 6; k++){
                cities[k] = 1;
              }
              t = -1;
              level = -1;
            }
            else if(c == '2'){
              return 0;
            }
          }
        } 
        t += .01;   // Increments the gloabl time
        usleep(5000);  // Wait until next animation
      }
    } 
} //end of main

/*
Functions
*/

void get_mouse_pos(int mouse_pos[2]){ // get mouse position
  mouse_pos[0] = gfx_xpos();
  mouse_pos[1] = gfx_ypos();
}

int distance(double x1, double y1, double x2, double y2){ // Literally just a distance formula function
  return sqrt(pow((x1-x2),2) + pow((y1-y2),2));
}

int num_missiles(int level){ // Define the max number of missiles per level
  int count = 0;
  count = 20 + 1.5*level;
  return count;
}

void init_structs(int level, Missile_t stock[MAX_MISSILES], Bomb_t shots[MAX_MISSILES]){ // Initialize structs using random starting and ending positions 
  int i;
  for(i = 0; i < MAX_MISSILES; i++){
    stock[i].start_x = 50+rand()%(XSIZE-100);   // Randomly chooses a starting x value between edges of the base for each missile
    stock[i].end_x = 50+rand()%(XSIZE-100);     // Randomly chooses a final x value between edges of the base for each missile
    stock[i].start_y = starting_y(level, i);    // Sets the starting height of each missile based on alrorighm in starting_y()
    stock[i].end_y = 550;   // Sets final height of each missile - it is always 550
    stock[i].current_pos_x = stock[i].start_x;  // sets the initial current position of each as the starting value
    stock[i].current_pos_y = stock[i].start_y;  // sets the initial current position of each as the starting value
    if(i<= num_missiles(level)){  // If that missile is in the set that will be used for that level, turn it on
      stock[i].status = 1;
    }
    else{
      stock[i].status = 0;    // if not, turn it off
    }
    stock[i].delta_y = .5+level*.01;  // Calculate the speed of each missile based on level
    stock[i].delta_x = -1*(stock[i].start_x - stock[i].end_x)/((stock[i].end_y-stock[i].start_y)/(stock[i].delta_y));  // Calculates the amount it changes in the x direction every iteration
  }
  for(i = 0; i < 10; i++){  // starts every bomb in the off position
    shots[i].status = 0;
  }

}

int starting_y(int level, int i){ // Calculate initial y positions based on wave
  if(level <= 5){
    if(i <= num_missiles(level)*.25){
      return 0;
    }
    else if(i <= num_missiles(level)*.5){
      return -300;
    }
    else if(i <= num_missiles(level)*.75){
      return -600;
    }
    else if(i <= num_missiles(level)){
      return -900;
    }
  }
  else if(level <= 15){
    if(i <= num_missiles(level)*.2){
      return 0;
    }
    else if(i <= num_missiles(level)*.4){
      return -300;
    }
    else if(i <= num_missiles(level)*.6){
      return -600;
    }
    else if(i <= num_missiles(level)*.8){
      return -900;
    }
    else{
      return -1200;
    }
  }
  else if(level <= 25){
    if(i <= num_missiles(level)*.2){
      return 0;
    }
    else if(i <= num_missiles(level)*.3){
      return -300;
    }
    else if(i <= num_missiles(level)*.5){
      return -450;
    }
    else if(i <= num_missiles(level)*.7){
      return -600;
    }
    else if(i <= num_missiles(level)*.8){
      return -900;
    }
    else{
      return -1200;
    }
  }
  else if(level <= 30){
    if(i <= num_missiles(level)*.2){
      return 0;
    }
    else if(i <= num_missiles(level)*.3){
      return -300;
    }
    else if(i <= num_missiles(level)*.5){
      return -450;
    }
    else if(i <= num_missiles(level)*.7){
      return -600;
    }
    else if(i <= num_missiles(level)*.8){
      return -900;
    }
    else if(i <= num_missiles(level)*.9){
      return -1050;
    }
    else{
      return -1200;
    }
  }
  else if(level > 30){
    if(i <= num_missiles(level)*.2){
      return 0;
    }
    else if(i <= num_missiles(level)*.3){
      return -300;
    }
    else if(i <= num_missiles(level)*.4){
      return -400;
    }
    else if(i <= num_missiles(level)*.5){
      return -500;
    }
    else if(i <= num_missiles(level)*.6){
      return -600;
    }
    else if(i <= num_missiles(level)*.7){
      return -750;
    }
    else if(i <= num_missiles(level)*.8){
      return -900;
    }
    else if(i <= num_missiles(level)*.9){
      return -1050;
    }
    else{
      return -1200;
    }
  }

}

void missile_position_calc(Missile_t stock[MAX_MISSILES]){ // Calculate the current position of each missile as a function of time

  int i;
  for(i = 0; i < MAX_MISSILES; i++){
    if(stock[i].status == 1){
      stock[i].current_pos_x += stock[i].delta_x; 
      stock[i].current_pos_y += stock[i].delta_y; 
    }
    else{
      continue;
    }

  }

}

void draw_missiles(Missile_t stock[MAX_MISSILES]){ // Draw the missiles
  int i;
  for(i = 0; i < MAX_MISSILES; i++){
    if(stock[i].status == 1){
      gfx_color(0,0,0);
      gfx_line(stock[i].start_x,stock[i].start_y, stock[i].current_pos_x-stock[i].delta_x, stock[i].current_pos_y-stock[i].delta_y);
      gfx_color(255,255,255);
      gfx_line(stock[i].start_x,stock[i].start_y, stock[i].current_pos_x, stock[i].current_pos_y);
      gfx_color(255,0,0);
    }
  }
}

void check_missile_status(Missile_t stock[MAX_MISSILES]){ // Check if missiles are too low
  int i;
  for(i = 0; i < MAX_MISSILES; i++){
    if(stock[i].status == 1){
      if(stock[i].current_pos_y >= 549){
        stock[i].status = 0;
        gfx_color(0,0,0);
        gfx_line(stock[i].current_pos_x, stock[i].current_pos_y, stock[i].start_x, stock[i].start_y);
      }
    }
  }
}

void gameover_screen(){ // Game Over screen display
  int x;
  char c = 0;
  gfx_clear();
  gfx_color(255,255,255);
  gfx_rectangle(0,0,XSIZE-1, YSIZE-1);
  gfx_changefont("9x15bold" );
  gfx_text(XSIZE/2 - 50, YSIZE/4, "Game over!"); 

  gfx_changefont("9x15" );
  gfx_text(XSIZE/2 - 100, YSIZE/4+100, "Please select an option: "); 
  gfx_text(XSIZE/2 - 100, YSIZE/4+150, "(1) New Game"); 
  gfx_text(XSIZE/2 - 100, YSIZE/4+200 , "(2) Exit Program"); 
}

void draw_pause_menu(){ // Draw pause menu
  gfx_color(0,0,0);
  gfx_fill_rectangle(XSIZE/4, YSIZE/4, XSIZE/2, YSIZE/2);
  gfx_color(255,255,255);
  gfx_rectangle(XSIZE/4, YSIZE/4, XSIZE/2, YSIZE/2);
  gfx_changefont("9x15bold" );
  gfx_text(XSIZE/2 - 50, YSIZE/4+50, "Game Paused"); 
  gfx_changefont("9x15" );
  gfx_text(XSIZE/2 - 100, YSIZE/4+100, "Please select an option: "); 
  gfx_text(XSIZE/2 - 100, YSIZE/4+150, "(1) Continue"); 
  gfx_text(XSIZE/2 - 100, YSIZE/4+200 , "(2) New Game"); 
  gfx_text(XSIZE/2 - 100, YSIZE/4+250 , "(3) Exit Program"); 
}

void reset_pause_menu(){ // Clear space that was the pause menu
  gfx_color(0,0,0);
  gfx_fill_rectangle(XSIZE/4, YSIZE/4, XSIZE/2+1, YSIZE/2+1);
}

void draw_score(){ // Display the scor
  char score_str[20];
  int cx;
  gfx_color(0,0,0);
  gfx_color(255,255,255);
  gfx_changefont("9x15" );
  gfx_text(XSIZE/2 - 75,  20, "Score:  "); 
  cx = snprintf(score_str, 20, "%d", score);
  gfx_text(XSIZE/2 + 25, 20, score_str); 
}

int title_screen(){ // Title Screen Display
  int x = 1;
  char c;
  gfx_clear();
  gfx_color(130,130,255);
 // gfx_changefont("lucidasans-bolditalic-24");
  gfx_text(XSIZE/2 - 125, YSIZE- 2*(YSIZE/3) - 25, "MISSILE COMMAND");
 // gfx_changefont("lucidasans-14");
  gfx_color(130,255,130);
  gfx_text(XSIZE/2 - 125, YSIZE- (YSIZE/3), "Press Left Mouse Button to start");
 // gfx_changefont("lucidasans-12");
  gfx_color(255,255,255);
  gfx_text(XSIZE/2 - 100, YSIZE - 2*(YSIZE/3) +70, "Points: ");
  gfx_text(XSIZE/2 - 100, YSIZE - 2*(YSIZE/3) +95, "25 x Missiles Stopped");
  gfx_text(XSIZE/2 - 100, YSIZE - 2*(YSIZE/3) +120, "100 x Cities Remaining per Level");
  gfx_color(255, 130, 130);
  gfx_text(XSIZE/2 - 40, YSIZE- (YSIZE/3)+50, "Controls:");
  gfx_text(XSIZE/2 - 110, YSIZE- (YSIZE/3)+75, "Left Mouse Button = Fire Bomb");
  gfx_text(XSIZE/2 - 85, YSIZE- (YSIZE/3)+100, "Esc = pause/resume");

  while(x){  // Checks for input from title screen (exits on escape, starts on click)
    c = gfx_wait();
    if(c == 1){
      x = 0;
    }
    else if(c == 27){
        return 0;
    }
  }
  return 1;
}

void draw_base(int level) { // Draw the ground with color being a function of level
   
   int x = level%5;
   switch (x){
      case 0:
         gfx_color(255, 0, 0);
         break;

      case 1:
         gfx_color(178, 102, 255);
         break;
  
      case 2:
         gfx_color(0, 204, 0);
         break;

      case 3:
         gfx_color(0, 128, 255);
         break;

      case 4:
         gfx_color(255, 128, 0);
         break;

      case 5:
         gfx_color(153, 0, 153);
         break;
   }

   gfx_fill_rectangle(0, 550, 800, 50);
   gfx_fill_rectangle(0, 500, 50, 100);
   gfx_fill_rectangle(750, 500, 50, 100);
   gfx_fill_rectangle(325, 535, 150, 15);
   gfx_fill_rectangle(350, 525, 100, 30);
   gfx_fill_rectangle(375, 515, 50, 45);
 
}

void draw_city(int x, int b) { // Draw one city at given x as a starting coordinate
   gfx_color (b, b, b);
   gfx_fill_rectangle(x+2, 550-40, 10, 40);
   gfx_fill_rectangle(x-8, 550-20, 10, 20);
   gfx_fill_rectangle(x+10, 550-30, 10, 30);
   gfx_fill_rectangle(x-15, 550-35, 10, 35);
   gfx_fill_rectangle(x-20, 550-10, 10, 10);
   gfx_fill_rectangle(x+17, 550-15, 10, 15);

}

void draw_cities(int city_mid[6]) { // Draws all the cities
   int a = 0;
   for(a = 0; a < 6; a++) {
      draw_city(city_mid[a], 160);
   }
}

void blot_cities(int cities[6], int city_mid[6]) {  // Draws black square over dead cities 

   int a = 0;

   for (a = 0; a < 6; a++) {
      if (cities[a] == 0){
         draw_city(city_mid[a], 0);
      }

   }
}

void check_missile_pos(int cities[6], Missile_t stock[MAX_MISSILES]) { // Checks if missile is low enough to destroy a city
   int i;
   for(i = 0; i < MAX_MISSILES; i++){
      if (stock[i].current_pos_y >= 500) {
         check_city_status(cities, stock, i);  // If it is, runs the program to check if it is in the right region
      }
   }
}

void check_city_status(int cities[6], Missile_t stock[MAX_MISSILES], int i) { // Check if city is killed & set that it is dead
    if((stock[i].current_pos_x >= 85)&&(stock[i].current_pos_x <= 132)){
      stock[i].status = 0;
      gfx_color(0,0,0);
      gfx_line(stock[i].current_pos_x, stock[i].current_pos_y, stock[i].start_x, stock[i].start_y);
      cities[0] = 0;
    }
    else if((stock[i].current_pos_x >= 166)&&(stock[i].current_pos_x <= 213)){
      stock[i].status = 0;
      gfx_color(0,0,0);
      gfx_line(stock[i].current_pos_x, stock[i].current_pos_y, stock[i].start_x, stock[i].start_y);
      cities[1] = 0;
    }
    else if((stock[i].current_pos_x >= 245)&&(stock[i].current_pos_x <= 292)){
      stock[i].status = 0;
      gfx_color(0,0,0);
      gfx_line(stock[i].current_pos_x, stock[i].current_pos_y, stock[i].start_x, stock[i].start_y);
      cities[2] = 0;
    }
    else if((stock[i].current_pos_x >= 510)&&(stock[i].current_pos_x <= 557)){
      stock[i].status = 0;
      gfx_color(0,0,0);
      gfx_line(stock[i].current_pos_x, stock[i].current_pos_y, stock[i].start_x, stock[i].start_y);
      cities[3] = 0;
    }
    else if((stock[i].current_pos_x >= 592)&&(stock[i].current_pos_x <= 639)){
      stock[i].status = 0;
      gfx_color(0,0,0);
      gfx_line(stock[i].current_pos_x, stock[i].current_pos_y, stock[i].start_x, stock[i].start_y);
      cities[4] = 0;
    }
    else if((stock[i].current_pos_x >= 675)&&(stock[i].current_pos_x <= 722)){
      stock[i].status = 0;
      gfx_color(0,0,0);
      gfx_line(stock[i].current_pos_x, stock[i].current_pos_y, stock[i].start_x, stock[i].start_y);
      cities[5] = 0;
    }
    else if((stock[i].current_pos_x >= 325)&&(stock[i].current_pos_x<= 475)) {
        stock[i].status = 0;
        gfx_color(0,0,0);
        gfx_line(stock[i].current_pos_x, stock[i].current_pos_y, stock[i].start_x, stock[i].start_y);
    }
}
    
int cities_remaining(int cities[6]){ // Counts cities left  (for seeing how many cities are remining for quit case)
  int i;
  int count = 0;
  for(i = 0; i < 6; i++){
    if(cities[i] == 1){  // If city is alive increment one
      count++;
    }
  }
  return count;
}

int count_missiles(Missile_t stock[MAX_MISSILES]){ // Count how many missiles are remaining
  int i = 0, count = 0;
  for(i = 0; i < MAX_MISSILES; i++){
    if(stock[i].status == 1){  // If it sees a missile alive still, increment it
      count++;
    }
  }
  return count;
}

void level_screen(int level){ // Draws display between levels showing score and what level you will approach 
  if(level == 0){ // only clears on first level to be more visually interesting
    gfx_clear();
  }
  int x = 1;
  char c;
  char level_str[20];
  int cx;
  gfx_color(255,255,255);
  gfx_changefont("9x15bold");
  gfx_text(XSIZE/2 - 100, YSIZE/4+50, "Approaching Level "); 
  cx = snprintf(level_str, 20, "%d", level);  // Stores the value of each level as a string for printing
  gfx_text(XSIZE/2 + 75, YSIZE/4+50, level_str);
  gfx_changefont("9x15" );
  gfx_text(XSIZE/2 - 170, YSIZE/4+100, "Press left mouse button to continue"); 

  char score_str[20];

  gfx_color(0,0,0);
  gfx_color(255,255,255);
  gfx_changefont("9x15" );
  gfx_text(XSIZE/2 - 75,  20, "Score:  "); 
  cx = snprintf(score_str, 20, "%d", score);  // Stores the score as a string for printing
  gfx_text(XSIZE/2 + 25, 20, score_str); 
  
  while(x){
    c = gfx_wait();
    if(c == 1){
      x = 0;
    }
  }
}
