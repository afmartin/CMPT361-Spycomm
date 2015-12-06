/*
######################################################################
CMPT 361 - Assignment 3
Group 4: Nick, John, Alex, Kevin
December 5th, 2015
Filename: screen.c
Description: Contains functions relevant to maintaining the user
interface for server.c
######################################################################
*/

#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "screen.h"

#define TEST 1
#define MAX_FILE_NAME_LENGTH 12

void drawBox(Box box){
  
  
  int y = box.row; 
  int x = box.column;
  
  mvhline(y, x, 0, WIDTH);
  mvhline(y+HEIGHT, x, 0, WIDTH);
  mvvline(y, x, 0, HEIGHT);
  mvvline(y, x+WIDTH, 0, HEIGHT);
  mvaddch(y, x, ACS_ULCORNER);
  mvaddch(y+HEIGHT, x, ACS_LLCORNER);
  mvaddch(y, x+WIDTH, ACS_URCORNER);
  mvaddch(y+HEIGHT, x+WIDTH, ACS_LRCORNER);
  refresh();
}
 
void connectedToDisplay(Box box, char * clientAddr, char * fileName){
   mvprintw(box.row+1, box.column+2, "Connected to: %s", clientAddr);
   if (MAX_FILE_NAME_LENGTH > strlen(fileName)) {
   	 mvprintw(box.row+2, box.column+2, "Downloading file: '%s'", fileName);
   } else {
   	 mvprintw(box.row+2, box.column+2, "Downloading file: '%.*s...'", MAX_FILE_NAME_LENGTH - 3, fileName);
   }

   refresh();
}

void progressBar(Box * box, long iterations){
   
  if (box->progress == 0){
    box->position = box->column + 1;
    box->progress = 0;
    box->percentage = 0.0;
    box->state = 1;
  }
  box->progress ++;
  if (iterations < MAXBAR){
    box->percentage = 100.0;
    attron(A_STANDOUT);
    mvprintw(box->row+4, box->position, "%-*c", MAXBAR, ' ');
    attroff(A_STANDOUT);
    mvprintw(box->row+5, box->column+1, "%.1f%%", box->percentage);
    
  }
  else if (box->progress % ((iterations / MAXBAR)) == 0 && 
	   box->progress <= ((iterations / MAXBAR)) * MAXBAR &&
	   box->percentage < 100.0){
    attron(A_STANDOUT);
    mvaddch(box->row+4, box->position++, ' ');
    attroff(A_STANDOUT);
    box->percentage += 100.0/MAXBAR;
    mvprintw(box->row+5, box->column+1, "%.1f%%", box->percentage);
    move(0,0);
    refresh();
  }
  
  if (box->progress == iterations || box->percentage == 100.0){
    box->progress = 0;
    box->percentage = 0.0;
    box->state = 0;
  }
  
}		   
    
void clearBox(Box box){
  
  for (int i = box.row+1; i < box.row+HEIGHT; i++){
    move (i, box.column+1);
    for (int ii = box.column+1; ii < box.column+WIDTH; ii++){
      addch(' ');
    }
  }
  refresh();
}

void displayWaitingForConnection(Box boxes[]){
  
  for (int i = 0; i < MAXBOXES; i++){
    if (boxes[i].state == 0){
      clearBox(boxes[i]);
      mvprintw(boxes[i].row+2, boxes[i].column+2, "Awaiting a connection");
      drawBox(boxes[i]);
    }
  }
}

void displayWaiting(Box box) {
  clearBox(box);
  mvprintw(box.row+2, box.column+2, "Awaiting a connection");
  drawBox(box);
}

void displayHandshakeInfo(Box box, char * clientAddr){
  
  mvprintw(box.row+2, box.column+2, "Initiating handshake with:");
  mvprintw(box.row+3, box.column+5, "%s", clientAddr); 
}

