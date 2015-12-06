/*                                                                       
                                                                          
##########################################################################
######                                                                    
CMPT 361 - Assignment 3                                                   
                                                                          
Group 4: Nick, John, Alex, Kevin                                          
November 6th, 2015                                                        
Filename: screen.h                                                        
Description: Functions for implementing the UI in the spycommd program    
#########################################################################\
######                                                                     */
#include <ncurses.h>
#define MAXBOXES 8
#define HEIGHT 7
#define WIDTH 36
#define MAXBAR 34
#define COLUMN1 0
#define COLUMN2 WIDTH+2

typedef struct _displayBox{
  
  int boxNo;
  int row;
  int column;
  int progress;
  int position;
  float percentage;
  int state;

} Box; 

/**                                                                       
 * drawBox                                                            
 *                                                                        
 * draws a box to the screen to represent a connection with the client
 *                                                                        
 * Args:                                                                  
 * struct Box box - the box struct to be drawn                            
 */

void drawBox(Box box);

/**
 * connectedToDisplay
 *
 * displays information about the connected client as well as the name of 
 * the file currently being sent
 *
 * Args:
 * struct Box box - box to display information
 * char * clientAddr - address of the client connected
 * char * fileName - name of file being transferred
 */

void connectedToDisplay(Box box, char * clientAddr, char * fileName);

/**
 * progressBar
 *
 * displays the progress of a file being downloaded
 *
 * Args:
 * struct Box box - which box to display the progress bar in
 * long iterations - the ammount of packets received before file is done 
 * done being transferred
 */
void progressBar(Box * box, long iterations);

/**
 * clearBox
 *
 * clears information out of a box
 * 
 * Args:
 * struct Box box - which box to clear
 */
void clearBox(Box box);

/**
 * displayWaiting
 *
 * displays that the box is waiting for a connection
 * 
 * Args:
 * struct Box box - which box to display in
 */
void displayWaiting(Box box);
