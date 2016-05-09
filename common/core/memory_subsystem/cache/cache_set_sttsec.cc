#include "cache_set_sttsec.h"
#include "log.h"
#include "stats.h"
#include "simulator.h"
#include "config.h"
#include "config.hpp"

// Implements LRU replacement, optionally augmented with Query-Based Selection [Jaleel et al., MICRO'10]

CacheSetSTTSEC::CacheSetSTTSEC(
      CacheBase::cache_t cache_type,
      UInt32 associativity, UInt32 blocksize, UInt32 sector_size, CacheSetInfoSTTSEC* set_info, UInt8 num_attempts)
   : CacheSet(cache_type, associativity, blocksize)
   , m_num_attempts(num_attempts)
   , m_sectorsize(sector_size)
   , m_mru_cutoff(0)
   , m_set_info(set_info)   
{  
   m_mru_cutoff = Sim()->getCfg()->getInt("perf_model/STT/mru_cutoff"); // front mru blocks are protected fron eviction
   m_nearby_thres = Sim()->getCfg()->getInt("perf_model/STT/nearby_thres"); // if too many nearrby blocks, eviction happened within sector regarless of mru protection.
   m_lru_bits = new UInt8[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
      m_lru_bits[i] = i;
   //m_mru_cutoff = Sim()->getCfg()->getInt("perf_model/STT/mru_cutoff");
}

CacheSetSTTSEC::~CacheSetSTTSEC()
{
   delete [] m_lru_bits;
}

UInt32
CacheSetSTTSEC::getReplacementIndex(CacheCntlr *cntlr)
{
   // First try to find an invalid block
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      if (!m_cache_block_info_array[i]->isValid())
      {
         // Mark our newly-inserted line as most-recently used
         moveToMRU(i);
         return i;
      }
   }
      
   // Make m_num_attemps attempts at evicting the block at LRU position
      UInt32 index = 0;
      UInt8 max_bits = 0; // sector search
      UInt32 index_2 = 0;
      UInt8 max_bits_2 = 0; // normal lru
      UInt8 num_of_nearby = 0;
      for (UInt32 i = 0; i < m_associativity; i++)
      {
		 if(m_cache_block_info_array[i]->getTag()/m_sectorsize == accessing_tag/m_sectorsize) { // find blocks from same sector
			if (m_lru_bits[i] > max_bits && isValidReplacement(i))
			{
				index = i;
				max_bits = m_lru_bits[i];
			}
			num_of_nearby++;
		}
		if (m_lru_bits[i] > max_bits_2 && isValidReplacement(i))
		{
				index_2 = i;
				max_bits_2 = m_lru_bits[i];
		}
      }
      LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");

    

	  if(max_bits >= m_mru_cutoff){
		  moveToMRU(index);
		  return index;
	  }
      else if( num_of_nearby >= m_nearby_thres){
		  moveToMRU(index);
		  return index;
	  }else{
         // Mark our newly-inserted line as most-recently used
         moveToMRU(index_2);
         //m_set_info->incrementAttempt(attempt);
         return index_2;
      }

   LOG_PRINT_ERROR("Should not reach here");
}

void
CacheSetSTTSEC::updateReplacementIndex(UInt32 accessed_index)
{
   m_set_info->increment(m_lru_bits[accessed_index]);
   moveToMRU(accessed_index);
}

void
CacheSetSTTSEC::moveToMRU(UInt32 accessed_index)
{
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      if (m_lru_bits[i] < m_lru_bits[accessed_index])
         m_lru_bits[i] ++;
   }
   m_lru_bits[accessed_index] = 0;
}

CacheSetInfoSTTSEC::CacheSetInfoSTTSEC(String name, String cfgname, core_id_t core_id, UInt32 associativity, UInt8 num_attempts)
   : m_associativity(associativity)
   , m_attempts(NULL)
{
   m_access = new UInt64[m_associativity];
   for(UInt32 i = 0; i < m_associativity; ++i)
   {
      m_access[i] = 0;
      registerStatsMetric(name, core_id, String("access-mru-")+itostr(i), &m_access[i]);
   }

   if (num_attempts > 1)
   {
      m_attempts = new UInt64[num_attempts];
      for(UInt32 i = 0; i < num_attempts; ++i)
      {
         m_attempts[i] = 0;
         registerStatsMetric(name, core_id, String("qbs-attempt-")+itostr(i), &m_attempts[i]);
      }
   }
};

CacheSetInfoSTTSEC::~CacheSetInfoSTTSEC()
{
   delete [] m_access;
   if (m_attempts)
      delete [] m_attempts;
}
