#include "cache_set_plru.h"
#include "log.h"

// Tree LRU for 4 and 8 way caches

CacheSetPLRU::CacheSetPLRU(
      CacheBase::cache_t cache_type,
      UInt32 associativity, UInt32 blocksize) :
   CacheSet(cache_type, associativity, blocksize)
{
   LOG_ASSERT_ERROR(associativity == 4 || associativity == 8 || associativity == 16,
      "PLRU not implemted for associativity %d (only 4, 8, 16)", associativity);
   for(int i = 0; i < 16; ++i)
      b[i] = 0;
}

CacheSetPLRU::~CacheSetPLRU()
{
}

UInt32
CacheSetPLRU::getReplacementIndex(CacheCntlr *cntlr)
{
   // Invalidations may mess up the LRU bits

   for (UInt32 i = 0; i < m_associativity; i++)
   {
      if (!m_cache_block_info_array[i]->isValid())
      {
         updateReplacementIndex(i);
         return i;
      }
   }

   UInt32 retValue = -1;
   if (m_associativity == 4)
   {
      if (b[0] == 0)
      {
         if (b[1] == 0) retValue = 0;
         else           retValue = 1;   // b1==1
      }
      else
      {
         if (b[2] == 0) retValue = 2;
         else           retValue = 3;   // b2==1
      }
   }
   else if (m_associativity == 8)
   {
      if (b[0] == 0)
      {
         if (b[1] == 0)
         {
            if (b[2] == 0) retValue= 0;
            else           retValue= 1;  // b2==1
         }
         else
         {                            // b1==1
            if (b[3] == 0) retValue = 2;
            else           retValue = 3;  // b3==1
         }
      }
      else
      {                               // b0==1
         if (b[4] == 0)
         {
            if (b[5] == 0) retValue = 4;
            else           retValue = 5;  // b5==1
         }
         else
         {                            // b4==1
            if (b[6] == 0) retValue = 6;
            else           retValue = 7;  // b6==1
         }
      }
   }
   else if (m_associativity == 16)
   {
      if (b[0] == 0)
      {
		  if (b[1] == 0)
		  {
			 if (b[2] == 0)
			 {
				if (b[3] == 0) retValue= 0;
				else           retValue= 1;  // b2==1
			 }
			 else
			 {                            // b1==1
				if (b[4] == 0) retValue = 2;
				else           retValue = 3;  // b3==1
			 }
		  }
		  else
		  {                               // b0==1
			 if (b[5] == 0)
			 {
				if (b[6] == 0) retValue = 4;
				else           retValue = 5;  // b5==1
			 }
			 else
			 {                            // b4==1
				if (b[7] == 0) retValue = 6;
				else           retValue = 7;  // b6==1
			 }
		  }
		}
		else{
		  if (b[8] == 0)
		  {
			 if (b[9] == 0)
			 {
				if (b[10] == 0) retValue= 8;
				else           retValue= 9;  // b2==1
			 }
			 else
			 {                            // b1==1
				if (b[11] == 0) retValue = 10;
				else           retValue = 11;  // b3==1
			 }
		  }
		  else
		  {                               // b0==1
			 if (b[12] == 0)
			 {
				if (b[13] == 0) retValue = 12;
				else           retValue = 13;  // b5==1
			 }
			 else
			 {                            // b4==1
				if (b[14] == 0) retValue = 14;
				else           retValue = 15;  // b6==1
			 }
		  }
		}
	}
   else
   {
      LOG_PRINT_ERROR("PLRU doesn't support associativity %d", m_associativity);
   }


   LOG_ASSERT_ERROR(isValidReplacement(retValue), "PLRU selected an invalid replacement candidate" );
   updateReplacementIndex(retValue);
   return retValue;

}

void
CacheSetPLRU::updateReplacementIndex(UInt32 accessed_index)
{
   if (m_associativity == 4)
   {
      if      (accessed_index==0) { b[0]=1;b[1]=1;     }
      else if (accessed_index==1) { b[0]=1;b[1]=0;     }
      else if (accessed_index==2) { b[0]=0;       b[2]=1;}
      else if (accessed_index==3) { b[0]=0;       b[2]=0;}
   }
   else if (m_associativity == 8)
   {
      if      (accessed_index==0) { b[0]=1;b[1]=1;b[2]=1;                            }
      else if (accessed_index==1) { b[0]=1;b[1]=1;b[2]=0;                            }
      else if (accessed_index==2) { b[0]=1;b[1]=0;       b[3]=1;                     }
      else if (accessed_index==3) { b[0]=1;b[1]=0;       b[3]=0;                     }
      else if (accessed_index==4) { b[0]=0;                     b[4]=1;b[5]=1;       }
      else if (accessed_index==5) { b[0]=0;                     b[4]=1;b[5]=0;       }
      else if (accessed_index==6) { b[0]=0;                     b[4]=0;       b[6]=1;}
      else if (accessed_index==7) { b[0]=0;                     b[4]=0;       b[6]=0;}
   }
   else if (m_associativity == 16) // added by Qi
   {
      if      (accessed_index==0) { b[0]=1;b[1]=1;b[2]=1;b[3]=1;                           }
      else if (accessed_index==1) { b[0]=1;b[1]=1;b[2]=1;b[3]=0;                            }
      else if (accessed_index==2) { b[0]=1;b[1]=1;b[2]=0;       b[4]=1;                     }
      else if (accessed_index==3) { b[0]=1;b[1]=1;b[2]=0;       b[4]=0;                     }
      else if (accessed_index==4) { b[0]=1;b[1]=1;                     b[5]=1;b[6]=1;       }
      else if (accessed_index==5) { b[0]=1;b[1]=1;                     b[5]=1;b[6]=0;       }
      else if (accessed_index==6) { b[0]=1;b[1]=1;                     b[5]=0;       b[7]=1;}
      else if (accessed_index==7) { b[0]=1;b[1]=0;                     b[5]=0;       b[7]=0;}
      else if (accessed_index==8) { b[0]=0; b[8]=1;b[9]=1;b[10]=1;                            }
      else if (accessed_index==9) { b[0]=0; b[8]=1;b[9]=1;b[10]=0;                            }
      else if (accessed_index==10) { b[0]=0; b[8]=1;b[9]=0;       b[11]=1;                     }
      else if (accessed_index==11) { b[0]=0; b[8]=1;b[9]=0;       b[11]=0;                     }
      else if (accessed_index==12) { b[0]=0; b[8]=0;                     b[12]=1;b[13]=1;       }
      else if (accessed_index==13) { b[0]=0; b[8]=0;                     b[12]=1;b[13]=0;       }
      else if (accessed_index==14) { b[0]=0; b[8]=0;                     b[12]=0;       b[14]=1;}
      else if (accessed_index==15) { b[0]=0; b[8]=0;                     b[12]=0;       b[14]=0;}
   }
   else
   {
      LOG_PRINT_ERROR("PLRU doesn't support associativity %d", m_associativity);
   }
}
