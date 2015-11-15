#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <ncurses/ncurses.h>

#define BOX1_START 1
#define BOX2_START 8
#define BOX3_START 16
#define BOX4_START 24

#define ITERATIONS 64
#define BOX1_END 7
#define BOX2_END 15
#define BOX3_END 23
#define BOX4_END 31

#define BOX_WIDTHS 68

#define PROGRESS_BAR_START 2
#define PROGRESS_LEN 64
#define DONE mvprintw(0, 0, "done")
#define DONES printf("done")

typedef struct _displayBox{
  
  int y1;
  int y2;
  int progress;
  int position;
  float percentage;
  
  //int state;
  
} displayBox;
  

/*
  following code snippet taken from 
  http://stackoverflow.com/questions/22399406/drawing-boxes-using-ncurses
  November 14, 2015
*/
void rectangle(int y1, int x1, int y2, int x2){
  
  mvhline(y1, x1, 0, x2-x1);
  mvhline(y2, x1, 0, x2-x1);
  mvvline(y1, x1, 0, y2-y1);
  mvvline(y1, x2, 0, y2-y1);
  mvaddch(y1, x1, ACS_ULCORNER);
  mvaddch(y2, x1, ACS_LLCORNER);
  mvaddch(y1, x2, ACS_URCORNER);
  mvaddch(y2, x2, ACS_LRCORNER);
}

void displayServerSpecs(){}

/* void displayHandshake(displayBox box, char * clientAddr){ */
  
/*   mvprintw(box.y1+2, 5, "Initializing handshake to %s", clientAddr); */
/* } */

void connectedToDisplay(displayBox * box, char * clientAddr,
			char * fileName){
  
  mvprintw(box->y1+1, 2, "Connected to %s", clientAddr);
  mvprintw(box->y1+2, 2, "Downloading file:  '%s'", fileName);
  /* move(box->y1+4, PROGRESS_BAR_START); */
  /* for (int i = 0; i < PROGRESS_LEN; i++) addch(' '); */
  refresh();
  
}

void progressBar(displayBox * box, int fileSize){
  
  if (box->progress == 0){
    box->position = PROGRESS_BAR_START;
    box->progress = 0;
    box->percentage = 0;
  }
  box->progress += 1;
  // checks how many iterations are needed before the progress bar jumps
  if (box->progress % (fileSize / PROGRESS_LEN) == 0){
    attron(A_STANDOUT);
    mvaddch(box->y1+4, box->position++, ' '); 
    attroff(A_STANDOUT);
    box->percentage += 100.0/64.0;
    mvprintw(box->y1+5, 2, "%.1f%%", box->percentage);
    move(0,0);
    refresh();
  }
}

void clearBox(displayBox box){
  
  for (int i = box.y1; i < box.y2; i++){
    move(i, 1);
    clrtoeol();
    //refresh();
  }
  rectangle(box.y1, 1, box.y2, BOX_WIDTHS);
  refresh();
}

int main(void){
  
  displayBox box, box2;
  box.y1 = BOX1_START;
  box.y2 = BOX1_END;
  box.progress = 0;
  box2.y1 = BOX2_START;
  box2.y2 = BOX2_END;
  box2.progress= 0;
  DONES;
  initscr();
  noecho();
  rectangle(box.y1, 1, box.y2, BOX_WIDTHS);
  rectangle(box2.y1, 1, box.y2, BOX_WIDTHS);
  //refresh();
  clearBox(box2);
  connectedToDisplay(&box, "192.158.0.92", "file.c");
  connectedToDisplay(&box2, "192.168.32.1", "compress.c");
  for (int i = 0; i < 765000078; i++){
    progressBar(&box, 765000078);
    progressBar(&box2, 765000078);
  }
  //rectangle(8, 1, 15, 73);
  //rectangle(16, 1, 23, 73);
  //rectangle(24, 1, 31, 73);
  //refresh();
  getch();
  endwin();
  return 0;
}
