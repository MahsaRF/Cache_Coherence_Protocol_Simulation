/*******************************************************
                          main.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fstream>
#include <iomanip>
using namespace std;

#include "cache.h"
const char protocol_name[3][8] = {"MSI", "MESI", "Dragon"};
int main(int argc, char *argv[])
{
	
   ifstream fin;
   FILE * pFile;
      
   if(argv[1] == NULL){
	 cout<<"input format: ";
	 cout<<"./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n";
	 exit(0);
   }
   if(argc < 7){
  	cout << "input format: ";
  	cout << "./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n";
	return 0;
  }

  int cache_size = atoi(argv[1]);
  int cache_assoc= atoi(argv[2]);
  int blk_size   = atoi(argv[3]);
  int num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
  int protocol   = atoi(argv[5]);	 /*0:MSI, 1:MESI, 2:Dragon*/
  char *fname = argv[6];
  char ResultFile[128] = {0};
  char Line[128];
  unsigned int Address;
  int cpu_num;
  int ctr=0;
  char RW;
  int write_back = 0;
  double miss_rate = 0.0;
  int memory_transaction = 0;

  if (protocol < 0 || protocol > 2){
        cout << "invalid protocol ";
        cout << "acceptable values are 0:MSI, 1:MESI, 2:Dragon\n";
        return 0;
  }
  Bus BUS(num_processors,cache_size,cache_assoc,blk_size,protocol);
  pFile = fopen (fname,"r");
  if(pFile == 0)
  {   
    cout << "Trace file problem\n";
    exit(0);
  }
  while(fgets(Line,30,pFile)!=NULL){
    ctr++;
    sscanf(Line,"%d %c  %8x",&cpu_num,&RW,&Address);
    BUS.cache_list[cpu_num]->Access(Address,RW);
  }
  /* print tht output to file*/

  /*1. header */
  fstream out;
  sprintf(ResultFile, "%s_%d_%d.txt", protocol_name[protocol], cache_size, blk_size);
  out.open (ResultFile, fstream::out | fstream::trunc);

  out << "===== 506 Personal information =====\n";
  out << "Mahsa Rezaei Firuzkuhi\n";
  out << "200067566\n";
  out << "Section ECE001\n";

  out << "===== 506 SMP Simulator Configuration =====\n";
  out << "L1_SIZE:                        "<< cache_size << "\n";
  out << "L1_ASSOC:                       "<< cache_assoc << "\n";
  out << "L1_BLOCKSIZE:                   "<< blk_size << "\n";
  out << "NUMBER OF PROCESSORS:           "<< num_processors << "\n";
  out << "COHERENCE PROTOCOL:             "<< protocol_name[protocol] << "\n";
  out << "TRACE FILE:                     "<< fname << "\n";

  /*2. cache  */
  for(int i = 0 ; i< num_processors ; i++){
	
      miss_rate = 100 * (0.0 + BUS.cache_list[i]->readMisses  + BUS.cache_list[i]->writeMisses) / 
				(BUS.cache_list[i]->reads + BUS.cache_list[i]->writes);

      if (protocol == 0 || protocol == 1)
	write_back = BUS.cache_list[i]->writeBacks + BUS.M_I[i] + BUS.M_S[i];
      else
	write_back = BUS.cache_list[i]->writeBacks + BUS.FLUSHES[i];

      if (protocol == 0)
	memory_transaction = BUS.cache_list[i]->readMisses + BUS.cache_list[i]->writeMisses
					+ write_back + BUS.cache_list[i]->extramemo;		
      else if (protocol == 1 || protocol == 2)
	memory_transaction = BUS.cache_list[i]->readMisses + BUS.cache_list[i]->writeMisses + write_back - BUS.cc[i];

      out << "============ Simulation results (Cache " << i << ") ============\n";
      out << "01. number of reads:                            " << BUS.cache_list[i]->reads << "\n";
      out << "02. number of read misses:                      " << BUS.cache_list[i]->readMisses << "\n";
      out << "03. number of writes:                           " << BUS.cache_list[i]->writes << "\n";
      out << "04. number of write misses:                     " << BUS.cache_list[i]->writeMisses << "\n";
      out << "05. total miss rate:                     	" << std::setprecision(2)<< fixed<<miss_rate << "%\n";
      out << "06. number of writebacks:                       " << write_back<< "\n";
      out << "07. number of cache-to-cache transfers:         " << BUS.cc[i] << "\n";
      out << "08. number of memory transactions:              " << memory_transaction<< "\n";

      if (protocol == 0 || protocol == 1){
      	out << "09. number of interventions:                    " << BUS.E_S[i] + BUS.M_S[i] << "\n";
      	out << "10. number of invalidations:                    " << BUS.M_I[i] + BUS.S_I[i] << "\n";
      }else{
	out << "09. number of interventions:                    " << BUS.E_Sc[i] + BUS.M_Sm[i]<< "\n";
        out << "10. number of invalidations:                    " << 0 << "\n";

      }
	
      if (protocol == 0 || protocol == 1)
	out << "11. number of flushes:                          " << BUS.M_I[i]+BUS.M_S[i] << "\n";

      else if(protocol==2)
        out << "11. number of flushes:                          " << BUS.FLUSHES[i] << "\n";
  }

  fclose(pFile);
  out.close();	
}
