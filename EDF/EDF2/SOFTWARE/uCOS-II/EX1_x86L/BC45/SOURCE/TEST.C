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

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        Task1_Stk[TASK_STK_SIZE];
OS_STK        Task2_Stk[TASK_STK_SIZE];
OS_STK        Task3_Stk[TASK_STK_SIZE];
/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  Task1();                       /* Function prototypes of tasks                  */
        void  Task2();
        void  Task3();
static  void  TaskStartCreateTasks(void);
        void  Print(void);


/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void  main (void)
{
    task_log_index = 0;
    printf("[Time]  [Event]   [From]  [To]\r\n");
    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */


    TaskStartCreateTasks();

    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OSTimeSet(0);
    OS_EXIT_CRITICAL();
    OSStart();                                             /* Start multitasking                       */
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
/*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks (void)
{
        OSTaskCreate(Task1, (void *)0, &Task1_Stk[TASK_STK_SIZE], 1,1,4);       /* Modify Init comptime deadline in TCB*/
        OSTaskCreate(Task2, (void *)0, &Task2_Stk[TASK_STK_SIZE], 2,2,5);
        OSTaskCreate(Task3, (void *)0, &Task3_Stk[TASK_STK_SIZE], 3,2,10);
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

void  Task1()
{
    int C = 1;
    int P = 4;      /*    period    */
    int start;
    int end;
    int toDelay;
    start = 0;
    for (;;) {
        Print();
        while(OSTCBCur->compTime > 0);
        OS_ENTER_CRITICAL();
        end = OSTimeGet();
        toDelay = P-(end-start);
        start = start+ P;
        OSTCBCur->compTime=C;
        OSTCBCur->deadline += P;
        OS_EXIT_CRITICAL();
        OSTimeDly(toDelay);
    }
}

void  Task2()
{
  int C = 2;
  int P = 5;
  int start;
  int end;
  int toDelay;
  start = 0;
  for(;;){
      Print();
      while(OSTCBCur->compTime > 0);
      OS_ENTER_CRITICAL();
      end = OSTimeGet();
      toDelay = P-(end-start);
      start = start+P;
      OSTCBCur->compTime = C;
      OSTCBCur->deadline += P;
      OS_EXIT_CRITICAL();
      OSTimeDly(toDelay);
    }
}


void  Task3()
{
  int C = 2;
  int P = 10;
  int start;
  int end;
  int toDelay;
  start = 0;
  for(;;){
      Print();
      while(OSTCBCur->compTime > 0);
      OS_ENTER_CRITICAL();
      end = OSTimeGet();
      toDelay = P-(end-start);
      start = start+P;
      OSTCBCur->compTime = C;
      OSTCBCur->deadline += P;
      OS_EXIT_CRITICAL();
      OSTimeDly(toDelay);
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

