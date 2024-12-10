// G8RTOS_Scheduler.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for scheduler functions

#include "../G8RTOS_Scheduler.h"

/************************************Includes***************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "../G8RTOS_CriticalSection.h"

#include <inc/hw_memmap.h>
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"

/********************************Private Variables**********************************/

// Thread Control Blocks - array to hold information for each thread
static tcb_t threadControlBlocks[MAX_THREADS];

// Thread Stacks - array of arrays for individual stacks of each thread
static uint32_t threadStacks[MAX_THREADS][STACKSIZE];

// Periodic Event Threads - array to hold pertinent information for each thread
static ptcb_t pthreadControlBlocks[MAX_PTHREADS];

// Current Number of Threads currently in the scheduler
static uint32_t NumberOfThreads;

// Current Number of Periodic Threads currently in the scheduler
static uint32_t NumberOfPThreads;

static uint32_t threadCounter = 0;

/*******************************Private Functions***********************************/

// Occurs every 1 ms.
static void InitSysTick(void) {
    // hint: use SysCtlClockGet() to get the clock speed without having to hardcode it!
    // Set systick period to overflow every 1 ms.
	SysTickPeriodSet(SysCtlClockGet() / 1000);
    // Set systick interrupt handler
	SysTickIntRegister(SysTick_Handler);
    // Set pendsv handler
	IntRegister(FAULT_PENDSV, PendSV_Handler);
    // Enable systick interrupt
	SysTickIntEnable();
    // Enable systick
	SysTickEnable();
}


/********************************Public Variables***********************************/

uint32_t SystemTime;

tcb_t* CurrentlyRunningThread;



/********************************Public Functions***********************************/

// SysTick_Handler
// Increments system time, sets PendSV flag to start scheduler.
// Return: void
void SysTick_Handler() {
    SystemTime++;

    // Traverse the linked-list to find which threads should be awake.
    tcb_t *ptr = &threadControlBlocks[0];
    do {
    	ptr = ptr->nextTCB; // skips at least one
    	if (ptr->asleep) {
    		if (ptr->sleepCount == 0) {
    			ptr->asleep = false;
    		} else {
    			ptr->sleepCount--;
    		}
    	}
    } while (ptr != &threadControlBlocks[0]); // look at all possible threads

    // Traverse the periodic linked list to run which functions need to be run.
    if (NumberOfPThreads > 0) {
    	ptcb_t *Pptr = &pthreadControlBlocks[0];
    	do {
    		Pptr = Pptr->nextPTCB; // skips at least one
    		if (Pptr->executeTime <= SystemTime) {
    			Pptr->handler();
    			Pptr->executeTime = SystemTime + Pptr->period;
    		}
    	} while (Pptr != &pthreadControlBlocks[0]);
    }

    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
}

void silent_thread() {
	while(1) {}
}

// G8RTOS_Init
// Initializes the RTOS by initializing system time.
// Return: void
void G8RTOS_Init() {
    uint32_t newVTORTable = 0x20000000;
    uint32_t* newTable = (uint32_t*)newVTORTable;
    uint32_t* oldTable = (uint32_t*) 0;

    for (int i = 0; i < 155; i++) {
        newTable[i] = oldTable[i];
    }

    HWREG(NVIC_VTABLE) = newVTORTable;

    SystemTime = 0;
    NumberOfThreads = 0;
    NumberOfPThreads = 0;

    G8RTOS_AddThread(silent_thread, 255, "silent");
}

// G8RTOS_Launch
// Launches the RTOS.
// Return: error codes, 0 if none
int32_t G8RTOS_Launch() {
    // Initialize system tick
	InitSysTick();
    // Set currently running thread to the first control block
	CurrentlyRunningThread = &threadControlBlocks[0];
       // Pendsv
	IntPrioritySet(FAULT_PENDSV, 0xE0);
       // Systick
	IntPrioritySet(FAULT_SYSTICK, 0xE0);

    // Call G8RTOS_Start()
	G8RTOS_Start();
    return 0;
}

// G8RTOS_Scheduler
// Chooses next thread in the TCB. This time uses priority scheduling.
// Return: void
void G8RTOS_Scheduler() {
    // Using priority, determine the most eligible thread to run that
    // is not blocked or asleep. Set current thread to this thread's TCB.
	uint32_t max = 256; // max
	tcb_t *pt;
	tcb_t *bestPt;
	if (!CurrentlyRunningThread->isAlive) CurrentlyRunningThread = CurrentlyRunningThread->nextTCB;
	pt = CurrentlyRunningThread; // search for highest thread not blocked or sleeping
	do {
		pt = pt->nextTCB; // skips at least one
		if ((pt->priority < max) && ((pt->blocked) == 0) && (!(pt->asleep))) {
			max = pt->priority;
			bestPt = pt;
		}
	} while (CurrentlyRunningThread != pt); // look at all possible threads
	CurrentlyRunningThread = bestPt;
}

void SetInitialStack(int i){
	threadControlBlocks[i].stackPointer = &threadStacks[i][STACKSIZE-16]; // thread stack pointer
	threadStacks[i][STACKSIZE-1] = 0x01000000; // Thumb bit
	threadStacks[i][STACKSIZE-3] = 0x14141414; // R14
	threadStacks[i][STACKSIZE-4] = 0x12121212; // R12
	threadStacks[i][STACKSIZE-5] = 0x03030303; // R3
	threadStacks[i][STACKSIZE-6] = 0x02020202; // R2
	threadStacks[i][STACKSIZE-7] = 0x01010101; // R1
	threadStacks[i][STACKSIZE-8] = 0x00000000; // R0
	threadStacks[i][STACKSIZE-9] = 0x11111111; // R11
	threadStacks[i][STACKSIZE-10] = 0x10101010; // R10
	threadStacks[i][STACKSIZE-11] = 0x09090909; // R9
	threadStacks[i][STACKSIZE-12] = 0x08080808; // R8
	threadStacks[i][STACKSIZE-13] = 0x07070707; // R7
	threadStacks[i][STACKSIZE-14] = 0x06060606; // R6
	threadStacks[i][STACKSIZE-15] = 0x05050505; // R5
	threadStacks[i][STACKSIZE-16] = 0x04040404; // R4
}

// G8RTOS_AddThread
// Adds a thread. This is now in a critical section to support dynamic threads.
// It also now should initalize priority and account for live or dead threads.
// Param void* "threadToAdd": pointer to thread function address
// Param uint8_t "priority": priority from 0, 255.
// Param char* "name": character array containing the thread name.
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_AddThread(void (*threadToAdd)(void), uint8_t priority, char *name) {
    // Your code here
	IBit_State = StartCriticalSection();
	if (NumberOfThreads > MAX_THREADS) {
		EndCriticalSection(IBit_State);
		return THREAD_LIMIT_REACHED;
	}
	uint8_t index;
	tcb_t* newThread = NULL;
	tcb_t* nextThread;
	tcb_t* previousThread;
	if (NumberOfThreads == 0) {
		index = 0;
		newThread = nextThread = previousThread = &threadControlBlocks[0];
	} else {
		for (index = 0; index < MAX_THREADS; index++) {
			if (threadControlBlocks[index].stackPointer != NULL && !threadControlBlocks[index].isAlive) {
				newThread = &threadControlBlocks[index];
				break;
			}
		}
		if (newThread == NULL) {
			index = NumberOfThreads;
			newThread = &threadControlBlocks[index];

			nextThread = &threadControlBlocks[0];
			previousThread = &threadControlBlocks[index-1];
		} else {
			uint8_t nextThreadIndex = index + 1;
			while (nextThreadIndex < MAX_THREADS && !threadControlBlocks[nextThreadIndex].isAlive) nextThreadIndex++;

			if (nextThreadIndex == MAX_THREADS) {
				nextThread = &threadControlBlocks[0];
			} else {
				nextThread = &threadControlBlocks[nextThreadIndex];
			}

			uint8_t previousThreadIndex = index - 1;
			while (previousThreadIndex != 0 && !threadControlBlocks[previousThreadIndex].isAlive) previousThreadIndex--;

			previousThread = &threadControlBlocks[previousThreadIndex];
		}
	}
	newThread->nextTCB = nextThread;
	newThread->previousTCB = previousThread;

	previousThread->nextTCB = newThread;
	nextThread->previousTCB = newThread;

	SetInitialStack(index);
	threadStacks[index][STACKSIZE-2] = (int32_t)(threadToAdd);

	newThread->isAlive = true;
	newThread->asleep = false;
	newThread->ThreadID = threadCounter;
	newThread->priority = priority;
	strncpy(newThread->threadName, name, sizeof(newThread->threadName));

	NumberOfThreads++;
	threadCounter++;
	EndCriticalSection(IBit_State);
    // This should be in a critical section!
}

// G8RTOS_Add_APeriodicEvent


// Param void* "AthreadToAdd": pointer to thread function address
// Param int32_t "IRQn": Interrupt request number that references the vector table. [0..155].
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_Add_APeriodicEvent(void (*AthreadToAdd)(void), uint8_t priority, int32_t IRQn) {
	IBit_State = StartCriticalSection();
	if (IRQn < 0 || IRQn > 155) {
		EndCriticalSection(IBit_State);
		return IRQn_INVALID;
	}
	if (priority > 6) {
		EndCriticalSection(IBit_State);
		return HWI_PRIORITY_INVALID;
	}

	IntRegister(IRQn, AthreadToAdd);
	IntPrioritySet(IRQn, priority);
	IntEnable(IRQn);
	EndCriticalSection(IBit_State);
	return NO_ERROR;

    // Disable interrupts
    // Check if IRQn is valid
    // Check if priority is valid
    // Set corresponding index in interrupt vector table to handler.
    // Set priority.
    // Enable the interrupt.
    // End the critical section.
}

// G8RTOS_Add_PeriodicEvent
// Adds periodic threads to G8RTOS Scheduler
// Function will initialize a periodic event struct to represent event.
// The struct will be added to a linked list of periodic events
// Param void* "PThreadToAdd": void-void function for P thread handler
// Param uint32_t "period": period of P thread to add
// Param uint32_t "execution": When to execute the periodic thread
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_Add_PeriodicEvent(void (*PThreadToAdd)(void), uint32_t period, uint32_t execution) {
    // your code
	IBit_State = StartCriticalSection();
	if (NumberOfPThreads > MAX_THREADS) {
		EndCriticalSection(IBit_State);
		return THREAD_LIMIT_REACHED;
	} else {
		if (NumberOfPThreads == 0) {
			pthreadControlBlocks[0].handler = PThreadToAdd;
			pthreadControlBlocks[0].period = period;
			pthreadControlBlocks[0].executeTime = execution;
			pthreadControlBlocks[0].currentTime = SystemTime;
			pthreadControlBlocks[0].nextPTCB = &pthreadControlBlocks[0];
			pthreadControlBlocks[0].previousPTCB = &pthreadControlBlocks[0];
		} else {
			pthreadControlBlocks[NumberOfPThreads].handler = PThreadToAdd;
			pthreadControlBlocks[NumberOfPThreads].period = period;
			pthreadControlBlocks[NumberOfPThreads].executeTime = execution;
			pthreadControlBlocks[NumberOfPThreads].currentTime = SystemTime;
			pthreadControlBlocks[NumberOfPThreads].nextPTCB = &pthreadControlBlocks[0];
			pthreadControlBlocks[NumberOfPThreads].previousPTCB = &pthreadControlBlocks[NumberOfPThreads-1];

			pthreadControlBlocks[NumberOfPThreads-1].nextPTCB = &pthreadControlBlocks[NumberOfPThreads];
			pthreadControlBlocks[0].previousPTCB = &pthreadControlBlocks[NumberOfPThreads];
		}
		NumberOfPThreads++;
	}
	EndCriticalSection(IBit_State);

    // Make sure that the number of PThreads is not greater than max PThreads.
        // Check if there is no PThread. Initialize and set the first PThread.
        // Subsequent PThreads should be added, inserted similarly to a doubly-linked linked list
            // last PTCB should point to first, last PTCB should point to last.
        // Set function
        // Set period
        // Set execute time
        // Increment number of PThreads

    return NO_ERROR;
}

// G8RTOS_KillThread
// Param uint32_t "threadID": ID of thread to kill
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_KillThread(threadID_t threadID) {
    // Start critical section
	IBit_State = StartCriticalSection();
    // Check if there is only one thread, return if so
	if (NumberOfThreads == 1) {
		EndCriticalSection(IBit_State);
		return CANNOT_KILL_LAST_THREAD;
	}
    // Traverse linked list, find thread to kill
	tcb_t *pt = &threadControlBlocks[0];
	do {
	    pt = pt->nextTCB; // skips at least one
	    if (pt->ThreadID == threadID) {
	    	pt->isAlive = false;
	    	pt->blocked = 0;
	    	pt->previousTCB->nextTCB = pt->nextTCB;
	    	pt->nextTCB->previousTCB = pt->previousTCB;
	    	NumberOfThreads--;
	    	if (pt == CurrentlyRunningThread) {
	    		EndCriticalSection(IBit_State);
	    		HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
	    	}
	    	EndCriticalSection(IBit_State);
	    	return NO_ERROR;
	    }
	} while (pt != &threadControlBlocks[0]);
        // Update the next tcb and prev tcb pointers if found
            // mark as not alive, release the semaphore it is blocked on
        // Otherwise, thread does not exist.
	EndCriticalSection(IBit_State);
	return THREAD_DOES_NOT_EXIST;
}

// G8RTOS_KillSelf
// Kills currently running thread.
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_KillSelf() {
    // your code
	IBit_State = StartCriticalSection();
	if (NumberOfThreads == 1) {
		EndCriticalSection(IBit_State);
		return CANNOT_KILL_LAST_THREAD;
	}
	CurrentlyRunningThread->isAlive = false;
	CurrentlyRunningThread->blocked = 0;
	CurrentlyRunningThread->previousTCB->nextTCB = CurrentlyRunningThread->nextTCB;
	CurrentlyRunningThread->nextTCB->previousTCB = CurrentlyRunningThread->previousTCB;
	NumberOfThreads--;
	EndCriticalSection(IBit_State);

	HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;

    // Check if there is only one thread
    // Else, mark this thread as not alive.

}

// sleep
// Puts current thread to sleep
// Param uint32_t "durationMS": how many systicks to sleep for
void sleep(uint32_t durationMS) {
    // Update time to sleep to
	CurrentlyRunningThread->sleepCount = durationMS;
    // Set thread as asleep
	CurrentlyRunningThread->asleep = true;

	HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
}

// G8RTOS_GetThreadID
// Gets current thread ID.
// Return: threadID_t
threadID_t G8RTOS_GetThreadID(void) {
    return CurrentlyRunningThread->ThreadID;        //Returns the thread ID
}

// G8RTOS_GetNumberOfThreads
// Gets number of threads.
// Return: uint32_t
uint32_t G8RTOS_GetNumberOfThreads(void) {
    return NumberOfThreads;         //Returns the number of threads
}
