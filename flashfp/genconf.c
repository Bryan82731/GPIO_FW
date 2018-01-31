#include <stdio.h>
#include <string.h>


int main(int argc, char **argv)
{
   char temp[64*1024];
   FILE *fp1 = fopen(argv[1],"r");
   FILE *fp2 = fopen(argv[2],"r");
   FILE *fp3 = fopen(argv[3],"r");
   FILE *fp4 = fopen(argv[4],"r");
   FILE *fp5 = fopen(argv[5],"w");

   memset(temp, 0, 64*1024);
   fread(temp, 1, 12*1024, fp1);
   fread(temp+ 24*1024, 1, 20*1024, fp2);
   fread(temp+ 44*1024, 1, 4*1024, fp4);
   fread(temp+ 60*1024,1, 4*1024,  fp3);
   fwrite(temp, 1, 64*1024, fp5);

   
   fclose(fp1);
   fclose(fp2);
   fclose(fp3);
   fclose(fp4);
   fclose(fp5);

   return 0; 
}
