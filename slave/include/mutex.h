#ifndef __MUTEX_H__
#define __MUTEX_H__

struct MutexLock
{
   SemaphoreHandle_t &_sem;

   inline MutexLock(SemaphoreHandle_t &sem) : _sem(sem)
   {
      xSemaphoreTake(_sem, portMAX_DELAY);
   }

   inline ~MutexLock()
   {
      xSemaphoreGive(_sem);
   }
};

struct MutexLockRecursive
{
   SemaphoreHandle_t &_sem;

   inline MutexLockRecursive(SemaphoreHandle_t &sem) : _sem(sem)
   {
      xSemaphoreTakeRecursive(_sem, portMAX_DELAY);
   }

   inline ~MutexLockRecursive()
   {
      xSemaphoreGiveRecursive(_sem);
   }
};

#endif