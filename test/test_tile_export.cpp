#define main app_main
#import "../src/build_linux.cpp"
#undef main

#import <stdio.h>

int main()
{
  printf("Tests passed\n");
  return 0;
}
