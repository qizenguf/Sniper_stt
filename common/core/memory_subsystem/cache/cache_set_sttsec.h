#ifndef CACHE_SET_STTSEC_H
#define CACHE_SET_STTSEC_H

#include "cache_set.h"

class CacheSetInfoSTTSEC : public CacheSetInfo
{
   public:
      CacheSetInfoSTTSEC(String name, String cfgname, core_id_t core_id, UInt32 associativity, UInt8 num_attempts);
      virtual ~CacheSetInfoSTTSEC();
      void increment(UInt32 index)
      {
         LOG_ASSERT_ERROR(index < m_associativity, "Index(%d) >= Associativity(%d)", index, m_associativity);
         ++m_access[index];
      }
      void incrementAttempt(UInt8 attempt)
      {
         if (m_attempts)
            ++m_attempts[attempt];
         else
            LOG_ASSERT_ERROR(attempt == 0, "No place to store attempt# histogram but attempt != 0");
      }
   private:
      const UInt32 m_associativity;
      UInt64* m_access;
      UInt64* m_attempts;
};

class CacheSetSTTSEC : public CacheSet
{
   public:
      CacheSetSTTSEC(CacheBase::cache_t cache_type,
            UInt32 associativity, UInt32 blocksize, UInt32 sectorsize, CacheSetInfoSTTSEC* set_info, UInt8 num_attempts);
      virtual ~CacheSetSTTSEC();

      virtual UInt32 getReplacementIndex(CacheCntlr *cntlr);
      void updateReplacementIndex(UInt32 accessed_index);

   protected:
      const UInt8 m_num_attempts;
      UInt8* m_lru_bits;
      UInt32 m_sectorsize;
      UInt32 m_mru_cutoff;
      UInt32 m_nearby_thres;
      CacheSetInfoSTTSEC* m_set_info;
      void moveToMRU(UInt32 accessed_index);
};

#endif /* CACHE_SET_STTSEC_H */
