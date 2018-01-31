#include <stdio.h>

main(int argc, char **argv)
{
  char temp[512*1024];
  FILE *in0, *out;
  int i = 0;
  int numread;
  int fsize;

  for(i= 0; i< 512*1024; i++)
  temp[i] = 0xff;
  in0 = fopen(argv[1], "r");
  out = fopen(argv[2], "w");

  fseek(in0, 0, SEEK_END);
  fsize = ftell(in0);
  rewind(in0);

  *(int *)temp = fsize;
  fread(temp+4, 1, 512*1024-4, in0);
  fwrite(temp, 1, fsize+4, out);

  fclose(in0);
  fclose(out);

}
