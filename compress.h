
typedef struct _TreeNode{
  
  char symbol;
  struct _TreeNode * left;
  struct _TreeNode * right;

} node;

typedef struct letterFrequencies{
  
  char symbol;
  int freq;
  struct letterFrequencies * next;

} frequencyTable;



frequencyTable *  findFrequencies(char * string);
void traverseFreqTable(frequencyTable ** table);

node * initTree(frequencyTable ** table);
int findEqualWeight();
int traverse(); // for testing 
int encodeChar();
int findByCode();
int swap();
int increaseWeight();
