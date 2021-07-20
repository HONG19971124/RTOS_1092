# RTOS-1092
###
### *Run Tool : DosBox*  
### *Implement OS : uCOS-II*  
#### LAB Implement :  
   - [x] RM scheduling
   - [x] EDF scheduling
   - [x] CPP resource
###
###
 *Code :(EDF)*  
 ```C
 static void OS_SchedNew(void)
{
    INT8U prio = OS_LOWEST_PRIO;
    INT32U deadline_r = 1000000;
    OS_TCB* cur = OSTCBList;
    while(cur->OSTCBNext){
        if(cur->OSTCBDly == 0 && cur->deadline != NULL && cur->deadline <deadline_r) {
            prio = cur->OSTCBPrio;
            deadline_r = cur->deadline;
        }
        cur = cur->OSTCBNext;
    }
    OSPrioHighRdy = prio;
}
 ```
 CPP main.c :[main.c(2 task)](https://github.com/HONG19971124/RTOS-1092/blob/CPP-resoure/cpp/SOFTWARE/uCOS-II/EX1_x86L/BC45/SOURCE/TEST.C)  
            :[main.c(3 task)](https://github.com/HONG19971124/RTOS-1092/blob/CPP-resoure/cpp2/SOFTWARE/uCOS-II/EX1_x86L/BC45/SOURCE/TEST.C)  
 CPP implement : [OS_MUTEX.c](https://github.com/HONG19971124/RTOS-1092/blob/CPP-resoure/cpp/SOFTWARE/uCOS-II/SOURCE/OS_MUTEX.C)   
 *Code :(CPP)*  
 ```C
 void  OSMutexPend (OS_EVENT *pevent, INT16U timeout, INT8U *err)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    INT8U      cpp;
    INT8U      mprio;                                      /* Mutex owner priority                     */
    BOOLEAN    rdy;                                        /* Flag indicating task was ready           */
    OS_TCB    *ptcb;

    if (OSIntNesting > 0) {                                /* See if called from ISR ...               */
        *err = OS_ERR_PEND_ISR;                            /* ... can't PEND from an ISR               */
        return;
    }
#if OS_ARG_CHK_EN > 0
    if (pevent == (OS_EVENT *)0) {                         /* Validate 'pevent'                        */
        *err = OS_ERR_PEVENT_NULL;
        return;
    }
    if (pevent->OSEventType != OS_EVENT_TYPE_MUTEX) {      /* Validate event block type                */
        *err = OS_ERR_EVENT_TYPE;
        return;
    }
#endif
    OS_ENTER_CRITICAL();								   /* Is Mutex available?                      */
    if ((INT8U)(pevent->OSEventCnt & OS_MUTEX_KEEP_LOWER_8) == OS_MUTEX_AVAILABLE) {
        pevent->OSEventCnt &= OS_MUTEX_KEEP_UPPER_8;       /* Yes, Acquire the resource                */
        pevent->OSEventCnt |= OSTCBCur->OSTCBPrio;         /*      Save priority of owning task        */
        pevent->OSEventPtr  = (void *)OSTCBCur;            /*      Point to owning task's OS_TCB       */
        OS_EXIT_CRITICAL();
        cpp   = (INT8U)(pevent->OSEventCnt >> 8);                     /* No, Get CPP from mutex            */
        mprio = (INT8U)(pevent->OSEventCnt & OS_MUTEX_KEEP_LOWER_8);  /*     Get priority of mutex owner   */
        ptcb  = (OS_TCB *)(pevent->OSEventPtr);                       /*     Point to TCB of mutex owner   */

        Mutex_log[Mutex_log_index][0] = OSTime;
        Mutex_log[Mutex_log_index][1] = cpp;                          /*what Resource?                      */
        Mutex_log[Mutex_log_index][2] = OSTCBCur->OSTCBPrio;

        if(cpp > ptcb->OSTCBPrio){
          Mutex_store[Mutex_store_index][0] = cpp;                    /*Org cpp                             */
          cpp = ptcb->OSTCBPrio;
          pevent->OSEventCnt  = (cpp << 8) | OSTCBCur->OSTCBPrio;
          Mutex_store[Mutex_store_index][1] = 1;                      /*Use Store index*/
        }

        if ((OSRdyTbl[ptcb->OSTCBY] &= ~ptcb->OSTCBBitX) == 0x00) {
             OSRdyGrp &= ~ptcb->OSTCBBitY;
        }
        ptcb->OSTCBPrio         = cpp;                                /* Change owner task prio to CPP     */
        ptcb->OSTCBY            = ptcb->OSTCBPrio >> 3;
        ptcb->OSTCBBitY         = OSMapTbl[ptcb->OSTCBY];
        ptcb->OSTCBX            = ptcb->OSTCBPrio & 0x07;
        ptcb->OSTCBBitX         = OSMapTbl[ptcb->OSTCBX];

        OSRdyGrp               |= ptcb->OSTCBBitY;                    /* ... make it ready at new priority. */
        OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX;
        Mutex_log[Mutex_log_index][3] = OSTCBCur->OSTCBPrio;
        Mutex_log[Mutex_log_index][4] = 1;                            /*Lock index                          */
        OSTCBPrioTbl[cpp]       = (OS_TCB *)ptcb;
        Mutex_log_index=Mutex_log_index+1;
    }
}
 ```
 ```C
 INT8U  OSMutexPost (OS_EVENT *pevent)
{
#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr;
#endif
    INT8U      cpp;
    INT8U      prio;
    INT8U      org;
    int        store;

    if (OSIntNesting > 0) {                           /* See if called from ISR ...                    */
        return (OS_ERR_POST_ISR);                     /* ... can't POST mutex from an ISR              */
    }
#if OS_ARG_CHK_EN > 0
    if (pevent == (OS_EVENT *)0) {                    /* Validate 'pevent'                             */
        return (OS_ERR_PEVENT_NULL);
    }
    if (pevent->OSEventType != OS_EVENT_TYPE_MUTEX) { /* Validate event block type                     */
        return (OS_ERR_EVENT_TYPE);
    }
#endif
    OS_ENTER_CRITICAL();
    cpp  = (INT8U)(pevent->OSEventCnt >> 8);          /* Get priority inheritance priority of mutex    */
    prio = (INT8U)(pevent->OSEventCnt & OS_MUTEX_KEEP_LOWER_8);  /* Get owner's original priority      */
    if (OSTCBCur->OSTCBPrio != cpp &&
        OSTCBCur->OSTCBPrio != prio) {                /* See if posting task owns the MUTEX            */
        OS_EXIT_CRITICAL();
        return (OS_ERR_NOT_MUTEX_OWNER);
    }
    if (OSTCBCur->OSTCBPrio == cpp) {                 /* Did we have to raise current task's priority? */
                                                      /* Yes, Return to original priority              */
                                                      /*      Remove owner from ready list at 'cpp'    */
        if ((OSRdyTbl[OSTCBCur->OSTCBY] &= ~OSTCBCur->OSTCBBitX) == 0) {
            OSRdyGrp &= ~OSTCBCur->OSTCBBitY;
        }

        Mutex_log[Mutex_log_index][0]=OSTime;

        if(Mutex_store[Mutex_store_index][1] == 1){
          store = Mutex_store[Mutex_store_index][0];
          Mutex_log[Mutex_log_index][1] = store;
        }
        else
        {
          Mutex_log[Mutex_log_index][1]=cpp;
        }

        Mutex_log[Mutex_log_index][2]=OSTCBCur->OSTCBPrio;

        OSTCBCur->OSTCBPrio         = prio;
        OSTCBCur->OSTCBY            = prio >> 3;
        OSTCBCur->OSTCBBitY         = OSMapTbl[OSTCBCur->OSTCBY];
        OSTCBCur->OSTCBX            = prio & 0x07;
        OSTCBCur->OSTCBBitX         = OSMapTbl[OSTCBCur->OSTCBX];
        OSRdyGrp                   |= OSTCBCur->OSTCBBitY;
        OSRdyTbl[OSTCBCur->OSTCBY] |= OSTCBCur->OSTCBBitX;
        OSTCBPrioTbl[prio]          = (OS_TCB *)OSTCBCur;

        Mutex_log[Mutex_log_index][3]=OSTCBCur->OSTCBPrio;
        Mutex_log[Mutex_log_index][4]=2;
        Mutex_log_index=Mutex_log_index+1;            /*unlock index                                   */
    }
    OSTCBPrioTbl[cpp] = (OS_TCB *)1;                  /* Reserve table entry                           */
    if (pevent->OSEventGrp != 0x00) {                 /* Any task waiting for the mutex?               */
                                                      /* Yes, Make HPT waiting for mutex ready         */
        prio                = OS_EventTaskRdy(pevent, (void *)0, OS_STAT_MUTEX);
        pevent->OSEventCnt &= OS_MUTEX_KEEP_UPPER_8;  /*      Save priority of mutex's new owner       */
        pevent->OSEventCnt |= prio;
        pevent->OSEventPtr  = OSTCBPrioTbl[prio];     /*      Link to mutex owner's OS_TCB             */
        OS_EXIT_CRITICAL();
        //OS_Sched();                                   /*      Find highest priority task ready to run  */
        return (OS_NO_ERR);
    }

    /*Restore cpp lost*/
    if(Mutex_store[Mutex_store_index][1] == 1){
       org = (INT8U)Mutex_store[Mutex_store_index][0];
       pevent->OSEventCnt = (org << 8) | OS_MUTEX_AVAILABLE;
       Mutex_store_index = Mutex_store_index+1;
    }
    pevent->OSEventCnt |= OS_MUTEX_AVAILABLE;         /* No,  Mutex is now available                   */
    pevent->OSEventPtr  = (void *)0;
    OS_EXIT_CRITICAL();
    return (OS_NO_ERR);
}
 ```
 ### Result :  (EDF)
 ![EDF 2task](https://github.com/HONG19971124/RTOS-1092/blob/EDF-scheduling/EDF_1result.png)
 ![EDF 3task](https://github.com/HONG19971124/RTOS-1092/blob/EDF-scheduling/EDF_2result.png)
 ### Result : (CPP)
 ![CPP 2task](https://github.com/HONG19971124/RTOS-1092/blob/CPP-resoure/CPP_result1.png)
 ![CPP 3task](https://github.com/HONG19971124/RTOS-1092/blob/CPP-resoure/CPP_resule2.png)
 
