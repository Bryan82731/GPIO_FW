#include <stdio.h>
#include <string.h>


int main(int argc, char **argv)
{
   char temp[11*4*1024];
   FILE *fp1 = fopen(argv[1],"r");
   FILE *fp2 = fopen(argv[2],"r+");

   memset(temp, 0, 11*4*1024);
   fseek(fp1, 0x2000 ,SEEK_SET);
   fread(temp, 1, 11*4*1024, fp1);
   fseek(fp2, 0x2000 ,SEEK_SET);
   fwrite(temp, 1, 11*4*1024, fp2);

   
   fclose(fp1);
   fclose(fp2);

   return 0; 
}
