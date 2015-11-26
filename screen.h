
#include <ncurses/ncurses.h>
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


Box * initBoxes();
void drawBox(Box box);
void connectedToDisplay(Box box, char * clientAddr, char * fileName);
void progressBar(Box * box, long fileSize);
void clearBox(Box box);
void endBox(Box * box);
