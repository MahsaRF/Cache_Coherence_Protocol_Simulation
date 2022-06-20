/*******************************************************
                          cache.h
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>

typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;

/****add new states, based on the protocol****/
enum{
	INVALID = 0,
	VALID,
	DIRTY,
        SHARED,
        EXC,
	SHAREDM,
	SHAREDC
};

class cacheLine 
{
public:
   ulong tag;
   ulong Flags;   // 0:invalid, 1:valid, 2:dirty 
   ulong seq; 
 
public:
   cacheLine()            { tag = 0; Flags = 0; }
   ulong getTag()         { return tag; }
   ulong getFlags()			{ return Flags;}
   ulong getSeq()         { return seq; }
   void setSeq(ulong Seq)			{ seq = Seq;}
   void setFlags(ulong flags)			{  Flags = flags;}
   void setTag(ulong a)   { tag = a; }
   void invalidate()      { tag = 0; Flags = INVALID; }//useful function
   bool isValid()         { return ((Flags) != INVALID); }
};


class Bus;

class Cache
{
public:
   ulong size, lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
   ulong reads,readMisses,writes,writeMisses,writeBacks, extramemo;

   //******///
   //add coherence counters here///
   //******///

   cacheLine **cache;
   ulong calcTag(ulong addr)     { return (addr >> (log2Blk) );}
   ulong calcIndex(ulong addr)  { return ((addr >> log2Blk) & tagMask);}
   ulong calcAddr4Tag(ulong tag)   { return (tag << (log2Blk));}
   
public:
    ulong currentCycle;  
    int id;
    Bus * BUS;
    int protocol;     
    Cache(int,int,int,int,int);
   ~Cache() { delete cache;}
   
   cacheLine *findLineToReplace(ulong addr);
   cacheLine *fillLine(ulong addr);
   cacheLine * findLine(ulong addr);
   cacheLine * findLine2(ulong addr);
   cacheLine * getLRU(ulong);
   
   ulong getRM(){return readMisses;} ulong getWM(){return writeMisses;} 
   ulong getReads(){return reads;}ulong getWrites(){return writes;}
   ulong getWB(){return writeBacks;}
   
   void writeBack(ulong)   {writeBacks++;}
   void Access(ulong,uchar);
   void printStats();
   void updateLRU(cacheLine *);

   //******///
  //add other functions to handle bus transactions///
   //******///

};

class Bus{
public:
  Cache **cache_list;
  int *S_I;
  int *M_I;
  int *I_S;
  int *I_M;
  int *S_M;
  int *M_S;
  int *I_E;
  int *E_I;
  int *E_S;
  int *E_M;
  int *cc;
  int *FLUSHES;
//
  int * E_Sc;
  int * Sm_Sc;
  int * M_Sm;
  int * Sm_M;
  int * Sc_Sm;
  int * Sc_M;
//
  int protocol;
  int reads;
  int reads_miss;
  int writes;
  int writes_miss;
  int writes_wb;
  int num_caches;

  bool BUSREADX(int, ulong);
  bool BUSREAD(int id, ulong Address);
  bool BUSUPDATE(int id, ulong Address);
  Bus (int _num_caches, int cache_size,int cache_assoc,int blk_size,int _protocol){
    num_caches = _num_caches;
    protocol = _protocol;

    cache_list= new Cache*[num_caches];
    for(int i=0;i<num_caches;i++){
	cache_list[i]=new Cache(cache_size,cache_assoc,blk_size,protocol,i);
	cache_list[i]->BUS=this; /* &BUS;*/
    }
    num_caches = _num_caches;
    M_S=new int[num_caches];
    M_I=new int[num_caches];
    S_M=new int[num_caches];
    S_I=new int[num_caches];
    I_S=new int[num_caches];
    I_M=new int[num_caches];
    E_M=new int[num_caches];
    E_S=new int[num_caches];
    E_I=new int[num_caches];
    I_E=new int[num_caches];
    cc=new int[num_caches];
    FLUSHES=new int[num_caches];
    E_Sc=new int[num_caches];
    Sm_Sc=new int[num_caches];
    M_Sm=new int[num_caches];
    Sm_M = new int[num_caches];
    Sc_Sm = new int[num_caches];
    Sc_M = new int[num_caches];
     
    for(int i=0 ; i< num_caches ; i++){    
        M_S[i]=0;
        M_I[i]=0;
        S_M[i]=0;
        S_I[i]=0;
        I_S[i]=0;
        I_M[i]=0;
	E_M[i]=0;
	E_S[i]=0;
	E_I[i]=0;
	I_E[i]=0;
        cc[i]=0;
	FLUSHES[i]=0;
	E_Sc[i]=0;
   	Sm_Sc[i]=0;
   	M_Sm[i]=0;
   	Sm_M[i]=0;
   	Sc_Sm[i]=0;
   	Sc_M[i]=0;
     }
  }
  ~Bus(){
    delete M_S;
    delete M_I;
    delete S_M;
    delete S_I;
    delete I_S;
    delete I_M;
    delete E_M;
    delete E_S;
    delete E_I;
    delete I_E;
    delete cc;
    delete FLUSHES;
    delete E_Sc;
    delete Sm_Sc;
    delete M_Sm;
    delete Sm_M;
    delete Sc_Sm;
    delete Sc_M;
    for(int i=0 ; i < num_caches ; i++)
      delete cache_list[i];
    delete [] cache_list;
   }	
};

#endif
