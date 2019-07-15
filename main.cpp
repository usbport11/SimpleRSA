#include <iostream>
#include <math.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <sys/stat.h>
#include <cmath>
#include "InfInt.h"

using namespace std;

typedef unsigned int KEY[2];

//unsigned int SmallSimpleDigits[3] = {17, 257, 65537};
unsigned int SimpleDigits[10] = {19, 23, 29, 31, 37, 41, 43, 47, 53, 59};

InfInt InfIntPow(InfInt myint1, InfInt myint2)
{
  InfInt Res = "1";
  int Count = myint2.toInt();
  cout<<Count<<endl;
  for(int i=0; i<Count; i++)
  {
    Res *= myint1;
  }
  return Res;
}

int gcd(int a, int b)
{
    if(b==0) return a;
    return gcd(b, a % b);
}

void gcdext(int a, int b, int *d, int *x, int *y)
{
  int s;
  if(b == 0)
  {
    *d = a;
    *x = 1;
    *y = 0;
    return;
  }
  gcdext(b, a%b, d, x, y);
  s = *y;
  *y = *x - (a / b) * (*y);
  *x = s;
}

bool GenerateKeys(KEY &k_public, KEY &k_private)
{
 unsigned int p = SimpleDigits[rand() % 10];
 unsigned int q = SimpleDigits[rand() % 10];
 unsigned int n = p * q;
 unsigned int f = (p - 1) * (q - 1);
 //search simple digit that sub on f with no rest
 unsigned int e = 3;
 //calculate d
 int g,x,y;
 unsigned int d;
 gcdext(e, f, &g, &x, &y);
 d = f + x;
 //generate keys
 k_public[0] = e; k_public[1] = n;
 k_private[0] = d; k_private[1] = n;
 //cout
 cout<<"p = "<<p<<endl;
 cout<<"q = "<<q<<endl;
 cout<<"n = "<<n<<endl;
 cout<<"f = "<<f<<endl;
 cout<<"e = "<<e<<endl;
 cout<<"d = "<<d<<endl;

 return true;
}

bool Encryption(char* SourceFile, char* NewFile, KEY k_public)
{
 struct stat FileStat;
 stat(SourceFile, &FileStat);
 int BytesCount = FileStat.st_size;
 char *Buffer = new char[BytesCount];
 memset(Buffer, 0, BytesCount);
 int Handle = open(SourceFile, O_RDWR | O_BINARY);
 read(Handle, Buffer, BytesCount);
 close(Handle);

 //translate key to big number
 char bufferA[10];
 unsigned int sz = 0;
 InfInt sym, p1, p2, encr;
 memset(bufferA, 0, 10);
 itoa(k_public[0], bufferA, 10);
 p1 = bufferA;
 memset(bufferA, 0, 10);
 itoa(k_public[1], bufferA, 10);
 p2 = bufferA;

 //encoding
 Handle = open(NewFile, O_RDWR | O_BINARY | O_TRUNC);
 cout<<"Encoding and writing..."<<endl;
 for(int i=0; i<BytesCount; i++)
 {
   //convert symbol
   memset(bufferA, 0, 10);
   itoa((unsigned int)Buffer[i], bufferA, 10);
   sym = bufferA;
   //calculate coded str
   encr = InfIntPow(sym, p1);
   encr = encr % p2; 
   //write header
   sz = encr.size() - 2;
   memset(bufferA, 0, 10);
   itoa((unsigned int)sz, bufferA, 10);
   write(Handle, (void*)bufferA, strlen(bufferA));
   //write separator |
   write(Handle, (void*)"|", 1);
   //write coded str
   write(Handle, (void*)encr.toString().c_str(), encr.size()-2);
 }
 close(Handle);

 if(Buffer) delete [] Buffer;

 return true;
}

bool Decryption(char* SourceFile, char* NewFile, KEY k_private)
{
 //write on fly. allocate memory not need here. i think    
 int HandleSrc = open(SourceFile, O_RDWR | O_BINARY);
 int HandleDec = open(NewFile, O_RDWR | O_BINARY | O_TRUNC);

 InfInt decr;
 InfInt p1, p2;

 string Cnt;
 char sym[1];
 char buffer[1];
 char bufferA[2048];
 unsigned int size;

 memset(bufferA, 0, 2048);
 itoa(k_private[0], bufferA, 10);
 p1 = bufferA;
 memset(bufferA, 0, 2048);
 itoa(k_private[1], bufferA, 10);
 p2 = bufferA;

 cout<<"Decoding and writing..."<<endl;
 while(!eof(HandleSrc))
 {
   read(HandleSrc, buffer , 1);
   if(buffer[0] == ''|'')
   {
     size = atoi(Cnt.c_str());
     memset(bufferA, 0, 2048);
     read(HandleSrc, bufferA , size);
     decr = bufferA;
     decr = InfIntPow(decr, p1);
     decr = decr % p2;
     memset(bufferA, 0, 2048);
     sym[0] = atoi(decr.toString().c_str());
     cout<<size<<" "<<sym[0]<<endl;
     write(HandleDec, sym, 1);
     Cnt.clear();
     continue;
   }
   Cnt.push_back(buffer[0]);
 }

 close(HandleSrc);
 close(HandleDec);

 return true;
}

int main(int argc, char *argv[])
{  
 KEY PublicKey, PrivateKey;
 if(GenerateKeys(PublicKey, PrivateKey))
 cout<<"["<<PublicKey[0]<<", "<<PublicKey[1]<<"] ["<<PrivateKey[0]<<", "<<PrivateKey[1]<<"]"<<endl;
 //Encryption("source.txt", "encrypt.txt", PublicKey);
 Decryption("encrypt.txt", "decrypt.txt", PrivateKey);
 cout<<"Finish!"<<endl;

 system("PAUSE");
 return EXIT_SUCCESS;
}
