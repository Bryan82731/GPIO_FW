#include <stdio.h>

main(int argc, char **argv)
{
  char temp[512*1024];
  FILE *in0,*in1,*in2, *out;
  int i = 0;
  int numread;

  for(i= 0; i< 512*1024; i++)
  temp[i] = 0xff;
  in0 = fopen(argv[1], "r");
  in1 = fopen(argv[2], "r");
  in2 = fopen(argv[3], "r");
  out = fopen(argv[4], "w");

  fread(temp, 1, 64*1024, in0);
  fread(temp+64*1024, 1, 384*1024, in1);
  fread(temp+448*1024, 1, 64*1024, in2);
  fwrite(temp, 1, 512*1024, out);

  fclose(in0);
  fclose(in1);
  fclose(in2);
  fclose(out);

}
