#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){
  char* line = NULL;
  size_t size;

  size_t num_tables = 0;
  printf("\n");
  while (getline(&line, &size, stdin) != -1){
    if (!strncmp("Making a persistent hash table.", line, 31)){
      num_tables += 1;
    } else {
      continue;
    }
    printf("\e[0K\r# num tables: %u          ",
           num_tables);
  }
  printf("\n");
}
