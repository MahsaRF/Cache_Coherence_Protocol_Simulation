/*******************************************************
                          cache.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "cache.h"
using namespace std;
bool Bus::BUSREADX(int id, ulong Address)
{
  bool exist = false;
  if(protocol==0)
  {
    for(int i=0;i<num_caches;i++){

      if(i==id)
        continue;

      cacheLine * line=cache_list[i]->findLine(Address);
      if(line==NULL)
        continue;

      if(line->Flags==DIRTY){
        /*cc[id]++;*/
        M_I[i]++;
      }

      if(line->Flags==SHARED)
      S_I[i]++;

      line->Flags=INVALID;
    }
    exist = false;
  }


  if(protocol==1)
  {
    for(int i=0;i<num_caches;i++)
    {

      if(i==id)
        continue;

      cacheLine * line=cache_list[i]->findLine(Address);
      if(line==NULL)
        continue;

      if(line->Flags == DIRTY){
         M_I[i]++;
	 exist=true;
      }
      if(line->Flags == SHARED){
        S_I[i]++;
	exist=true;
      }

      if(line->Flags == EXC){
	E_I[i]++;
	exist=true;
      }

     if(line->Flags==SHARED)
       exist=true;

     line->Flags=INVALID;

    }
  }
  return exist ;
}

bool Bus::BUSUPDATE(int id, ulong Address)
{
  bool exist = false;
  if(protocol==2)
  {
    for(int i=0 ; i < num_caches ; i++)
    {

      if(i == id)
        continue;

      cacheLine * line=cache_list[i]->findLine(Address);
      if(line == NULL)
        continue;

      if(line->Flags == SHAREDC){
	exist = true;
      }
      else if(line->Flags == SHAREDM){
	line->Flags=SHAREDC;
        Sm_Sc[i]++;
	exist = true;
      }
  
    else if(line->Flags == EXC){
 	line->Flags=SHAREDC;
        exist = true;
      }

      else if(line->Flags == DIRTY){
        line->Flags=SHAREDC;    
        exist = true;
      }

    }
  }
  return exist;
}

bool Bus::BUSREAD(int id, ulong Address)
{

  if(protocol == 0)
  {
    for(int i=0 ; i < num_caches ; i++){

      if(i==id)
        continue;

    cacheLine * line=cache_list[i]->findLine(Address);
    if(line==NULL)
      continue;

    if(line->Flags==DIRTY)
    {
       M_S[i]++;
    }

    line->Flags=SHARED;
    
   }
   return 1;
  }

  if(protocol==1)
  {
    bool found=false;
    for(int i=0;i<num_caches;i++)
    {

      if(i==id)
        continue;

      cacheLine * line=cache_list[i]->findLine(Address);
      if(line==NULL)
        continue;

     if(line->Flags == DIRTY){
	M_S[i]++;
	found=true;
     }

     if(line->Flags==INVALID)
	I_S[i]++;

     if(line->Flags==EXC){
	E_S[i]++;
	found=true;
     }

    if(line->Flags==SHARED){
	found = true;
    }

    line->Flags=SHARED;

   }
   return found;
  }


  if(protocol==2)
  {
    bool found=false;
    for(int i=0;i<num_caches;i++)
    {
      if(i==id)
        continue;

      cacheLine * line=cache_list[i]->findLine(Address);
      if(line==NULL)
        continue;

      if(line->Flags==DIRTY){
	line->Flags=SHAREDM;
	M_Sm[i]++;
	found=true;
	FLUSHES[i]++;
     }
    else if(line->Flags == EXC){
	line->Flags=SHAREDC;
	E_Sc[i]++;
	found=true;
    }
    else if(line->Flags == SHAREDM){
	
        FLUSHES[i]++; 
        found=true;
    }

    else if(line->Flags == SHAREDC){
       found=true;
    }
       
   }
  return found;
  }
return true;
}

Cache::Cache(int s,int a,int b,int _protocol,int _id )
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   writeMisses = writeBacks = currentCycle = 0;
   extramemo = 0;
   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);   
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));   
   log2Blk    = (ulong)(log2(b));   
  
   //*******************//
   //initialize your counters here//
   //*******************//
 
   tagMask =0;
   for(i=0;i<log2Sets;i++)
   {
		tagMask <<= 1;
        tagMask |= 1;
   }
   
   /**create a two dimentional cache, sized as cache[sets][assoc]**/ 
   cache = new cacheLine*[sets];
   for(i=0; i<sets; i++)
   {
      cache[i] = new cacheLine[assoc];
      for(j=0; j<assoc; j++) 
      {
	   cache[i][j].invalidate();
      }
   }
   protocol= _protocol;      
   id = _id; 	
}
/**you might add other parameters to Access()
 * since this function is an entry point 
 * to the memory hierarchy (i.e. caches)**/
void Cache::Access(ulong addr,uchar op)
{
	currentCycle++;/*per cache global counter to maintain LRU order 
			among cache ways, updated on every cache access*/
        	
	if(op == 'w') {writes++;BUS->writes++;}
	else          {reads++;BUS->reads++;}
	
	cacheLine * line = findLine(addr);
	if(line == NULL)/*miss*/
	{
          bool exc=false;
          if(op=='w')
          {
          	if(protocol == 0){/* -- write -- miss  -- MSI*/
          		BUS->I_M[id]++;
          		BUS->BUSREADX(id,addr);
        		// BUS->cc[id]++;
          	}
         
          	if(protocol==1){/* -- write -- miss  -- MESI*/
			BUS->I_M[id]++;
          		if(BUS->BUSREADX(id,addr))
         		BUS->cc[id]++;
           
           	}

          	if(protocol==2){ /* -- write -- miss  -- Dragon*/
/*----------------------------------------------------------------*/ 
                        bool v = BUS->BUSREAD(id,addr);

//			bool v=BUS->BUSUPDATE(id,addr);
                	cacheLine *newline = fillLine(addr);

			if (v){
				BUS->BUSUPDATE(id,addr);
                        	newline->setFlags(SHAREDM);
			}
			else{ 
				newline->setFlags(DIRTY);
			}
/*-------------------------------------------------------------*/
           	}

          }
          else{
          	if(protocol==0){/*read -- miss  -- MSI*/
          		BUS->I_S[id]++;
          		BUS->BUSREAD(id,addr);
          	}
          	if(protocol==1){/*read -- miss  -- MESI*/
	  		bool v=BUS->BUSREAD(id,addr);
          		if(v){
           			BUS->cc[id]++;
           			BUS->I_S[id]++;
	   		}
          		else{
				exc=true;
           			BUS->I_E[id]++;
           		}
          	}

	  	if(protocol == 2){	/*read -- miss  -- Dragon*/
          		bool v=BUS->BUSREAD(id,addr);
                        cacheLine *newline = fillLine(addr);
          		if(v){
				newline->setFlags(SHAREDC);
         			//  BUS->cc[id]++;
           		}
          		else{
                                newline->setFlags(EXC);
           		}
          	}         

          }
	  if(op == 'w') {
		writeMisses++;
		BUS->writes_miss++;
	  }
	  else {
		readMisses++;
		BUS->reads_miss++;
	  }
          /*------------Update cache line state -----------------*/
          if (protocol==0 || protocol==1){
	  	cacheLine *newline = fillLine(addr);
   	 	if(op == 'w') 
			newline->setFlags(DIRTY);
          	else{
			if(!exc)
                		newline->setFlags(SHARED); 
                	else
                		newline->setFlags(EXC);   
	  	}
	 }
	 /*-------------------------------------------------------*/
	}

	else
	{
		/**since it's a hit, update LRU and update dirty flag**/
		updateLRU(line);
		if(op == 'w') { 
                	if(protocol==0){  /*hit -- write -- MSI*/
                  		if(line->Flags==SHARED){
                   			BUS->S_M[id]++;
                   			BUS->BUSREADX(id,addr);
					extramemo++;
                  		}   
		 		line->setFlags(DIRTY);
                 	}

                  	if(protocol == 1){/*hit -- write -- MESI*/
                  		if(line->Flags == SHARED){
                   			BUS->S_M[id]++;
                   			BUS->BUSREADX(id,addr);
                  		} 
                  
                  		if(line->Flags==EXC)
                  		{
                   			BUS->E_M[id]++;
                  		}
                  
                   		line->setFlags(DIRTY);
                   	}
                  	if(protocol == 2){ /* hit -- write -- Dragon*/
/*-------------------------------------------------------------------*/
                  		bool exist=false;
                  		if(line->Flags==SHAREDM){
                   			exist=BUS->BUSUPDATE(id,addr);
					if (exist){
						line->setFlags(SHAREDM);
					}
					else{
                                                BUS->Sm_M[id]++;
                                                line->setFlags(DIRTY);
					}	
                  		}
                  		else if(line->Flags==SHAREDC){
                   			exist=BUS->BUSUPDATE(id,addr);
					if (exist){
                                                BUS->Sc_Sm[id]++;
                                                line->setFlags(SHAREDM);
                                        }
                                        else{
                                                BUS->Sc_M[id]++;
                                                line->setFlags(DIRTY);
                                        }
                  		}
                  		else if(line->Flags==EXC){
                   			BUS->E_M[id]++;
                                        line->setFlags(DIRTY);
                  		}
                                else if(line->Flags==DIRTY){
                   			line->setFlags(DIRTY);
				}
/*---------------------------------------------------------------------*/
                   	}

		}                
	}
}

cacheLine * Cache::findLine(ulong addr)
{
                
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
	if(cache[i][j].isValid())
	        if(cache[i][j].getTag() == tag)
		{
		     pos = j; break; 
		}
   if(pos == assoc)
	return NULL;
   else
	return &(cache[i][pos]); 
}

/*look up line*/
cacheLine * Cache::findLine2(ulong addr)
{
   ulong i, j, tag, pos;

   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);

   for(j=0; j<assoc; j++)
                if(cache[i][j].getTag() == tag)
                {
                     pos = j; break;
                }
   if(pos == assoc)
        return NULL;
   else
        return &(cache[i][pos]);
}




/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
  line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) return &(cache[i][j]);     
   }   
   for(j=0;j<assoc;j++)
   {
	 if(cache[i][j].getSeq() <= min) { victim = j; min = cache[i][j].getSeq();}
   } 
   assert(victim != assoc);
   
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   if(victim->getFlags() == DIRTY || victim->getFlags()==SHAREDM) 
     writeBack(addr);

   tag = calcTag(addr);   
   victim->setTag(tag);
      victim->setFlags(SHARED);    
   if(protocol==1 || protocol==2)
      victim->setFlags(EXC);
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}

void Cache::printStats()
{ 
	printf("===== Simulation results      =====\n");
	/****print out the rest of statistics here.****/
	/****follow the ouput file format**************/
}
