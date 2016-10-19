#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){
  char* line = NULL;
  size_t size;

  size_t num_svs = 0;
  size_t num_stems = 0;
  printf("\n");
  while (getline(&line, &size, stdin) != -1){
    if (!strncmp("Making shadow value", line, 19)){
      num_svs += 1;
    } else if (!strncmp("Cleaning up shadow value", line, 24)){
      num_svs -= 1;
    } else if (!strncmp("Making stem node", line, 16)){
      num_stems += 1;
    } else if (!strncmp("Cleaning up stem node", line, 20)){
      num_stems -= 1;
    } else {
      continue;
    }
    printf("\e[0K\r# shadow values: %u      \t; # stems: %u      ",
           num_svs, num_stems);
  }
  printf("\n");
}
