

#define NYT '\0'

typedef struct Tree{
  
  int weight;
  int isLeaf;
  struct Tree * left;
  struct Tree * right;
  struct Tree * parent;

} ;

int initTree();
int updateTree();
int findEqualWeight();
int traverse(); // for testing 
int encodeChar();
int findByCode();
int swap();
int increaseWeight();
