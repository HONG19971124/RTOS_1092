/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/

#include "includes.h"
#include "stdio.h"
/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 1024       /* Size of each task's stacks (# of WORDs)            */

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        Task1_Stk[TASK_STK_SIZE];
OS_STK        Task2_Stk[TASK_STK_SIZE];
INT8U         err;
OS_EVENT      *ResourceMutex1;
OS_EVENT      *ResourceMutex2;
/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  Task1();                       /* Function prototypes of tasks                  */
        void  Task2();
static  void  TaskStartCreateTasks(void);
static  void  MutexCreate(void);
        void  Print(void);
        void   MutexPrint(void);


/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void  main (void)
{
    task_log_index = 0;
    Mutex_log_index= 0;
    Mutex_store_index=0;
    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */


    TaskStartCreateTasks();

    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    TaskStartCreateTasks();
    MutexCreate();
    OS_EXIT_CRITICAL();
    OSTimeSet(0);
    OSStart();                                             /* Start multitasking                       */
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
/*
********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks (void)
{
        OSTaskCreate(Task1, (void *)0, &Task1_Stk[TASK_STK_SIZE], 3);
        OSTaskCreate(Task2, (void *)0, &Task2_Stk[TASK_STK_SIZE], 4);
}

static void MutexCreate(void)
{
       ResourceMutex1 = OSMutexCreate(1,&err);
       ResourceMutex2 = OSMutexCreate(2,&err);
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/
void  Task1()
{
  int ready;
  int C = 11;
  int resource = 0;
  int status = 0;
  OSTCBCur->compTime = C;
  for(;;){
      ready = OSTime;
      if(ready < 5){
            ready = 5 - ready;
            OSTimeDly(ready);
      }
      if(task_log[2][0] == 23){/*OSTime Complete & End*/
            printf("%-10d%-16s%3d%9d\n",task_log[2][0],"Complete",task_log[2][2],task_log[2][3]);
            printf("-----------------END-----------------");
            OSTaskSuspend(3);
      }
      while(OSTCBCur->compTime > 0){
        if(status != OSTCBCur->compTime){
             if(resource == 2){
                OSMutexPend(ResourceMutex2,0,&err);
             }
             else if(resource == 5){
                OSMutexPend(ResourceMutex1,0,&err);
             }
             else if(resource == 8){
                OSMutexPost(ResourceMutex1);

             }
             resource = resource + 1;
             status = OSTCBCur->compTime;
         }
      }



      OSMutexPost(ResourceMutex2);
      printf("%-10d%-16s%3d%9d\n",task_log[1][0],"Complete",task_log[1][2],task_log[1][3]);/* Task 2 to task 1 complete print*/
      MutexPrint();
      OSTimeDly(1);
    }
}






void  Task2()
{
  int C = 12;
  int resource = 0;
  int status = 0;
  OSTCBCur->compTime = C;

  for(;;){
      while(OSTCBCur->compTime > 0){
         if(status != OSTCBCur->compTime){
            if(resource == 2){
               OSMutexPend(ResourceMutex1,0,&err);
            }
            else if(resource == 8){
               OSMutexPend(ResourceMutex2,0,&err);
            }
            else if(resource == 10){
               OSMutexPost(ResourceMutex2);
            }
            resource = resource + 1;
            status = OSTCBCur->compTime;
         }
      }
      OSMutexPost(ResourceMutex1);
      MutexPrint();
      OSTaskSuspend(4);
    }
}


void MutexPrint(void)
{
    int i;
    int j;
    if(Mutex_log_index > 0){
        for( i = 0; i<Mutex_log_index; i++){
            if(Mutex_log[i][4] == 1){
                printf("%-10d lock    R%d Prio %d changes to %d\n",Mutex_log[i][0],Mutex_log[i][1],Mutex_log[i][2],Mutex_log[i][3]);    /*Lock print*/
                Mutex_log[i][4] = 0;
            }
            else if(Mutex_log[i][4] == 2){
                printf("%-10d unlock  R%d Prio %d changes to %d\n",Mutex_log[i][0],Mutex_log[i][1],Mutex_log[i][2],Mutex_log[i][3]);    /*unlock print*/
                Mutex_log[i][4] = 0;
            }
        }
    }
}


void Print(void)
{
  int i;
  int j;
  if(task_log_index > 0){
    for( i = 0; i<task_log_index; i++){
      if(task_log[i][1] == 1){
        printf("%-10d%-16s%3d%9d\n",task_log[i][0],"Preempt",task_log[i][2],task_log[i][3]);
        for( j = 0; j<5; j++){
          task_log[i][j]=0;
        }
      }  else if(task_log[i][1] == 2){
          printf("%-10d%-16s%3d%9d\n",task_log[i][0],"Complete",task_log[i][2],task_log[i][3]);
        for( j = 0; j<5; j++){
            task_log[i][j]=0;
        }
      }
    }
  }
}
