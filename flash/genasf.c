#include <stdio.h>
#include <string.h>


int main(int argc, char **argv)
{
   char temp[20*1024];
   FILE *fp1 = fopen(argv[1],"r");
   FILE *fp2 = fopen(argv[2],"r");
   FILE *fp3 = fopen(argv[3],"w");

   memset(temp, 0, 20*1024);
   fread(temp, 1, 20*1024, fp1);
   fread(temp+8*1024, 1, 4*1024, fp2);
   fwrite(temp, 1, 20*1024, fp3);

   
   fclose(fp1);
   fclose(fp2);
   fclose(fp3);

   return 0; 
}
