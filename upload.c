#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>
 
int main(void)
{
   char addr[] = "http://127.0.0.1:8080";
   char data[] = "./White Noise.mp3";
   return up(addr, data);
}

int up(char* addr, char* data)
{
  system("curl -X POST -F file=@\"./White Noise.mp3\" http://127.0.0.1:8080");
  return 0;
}