/*----------------------------------------------------------------------------
Circular Buffer Implementation (Multiple Producer-Consumer)
*----------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "cmsis_os.h"
#include "uart.h"
//#define FULL
#define EMPTY

void Producer_Thread (void const *argument); 
void Consumer1_Thread (void const *argument);
void Consumer2_Thread (void const *argument);
void Cashier_Thread (void const *argument);



#ifdef FULL
/*----------------------------------------------------------------------------
	Test for full buffer condition
*----------------------------------------------------------------------------*/
osThreadDef(Producer_Thread, osPriorityHigh, 1, 0);
osThreadDef(Consumer1_Thread, osPriorityNormal, 1, 0);
osThreadDef(Consumer2_Thread, osPriorityNormal, 1, 0);
osThreadDef(Cashier_Thread, osPriorityNormal, 1, 0);
#elif defined(EMPTY)
/*----------------------------------------------------------------------------
	Test for empty buffer condition
*----------------------------------------------------------------------------*/
osThreadDef(Producer_Thread, osPriorityNormal, 1, 0);
osThreadDef(Consumer1_Thread, osPriorityHigh, 1, 0);
osThreadDef(Consumer2_Thread, osPriorityHigh, 1, 0);
osThreadDef(Cashier_Thread, osPriorityNormal, 1, 0);
#else
// Thread Definition
osThreadDef(Producer_Thread, osPriorityNormal, 1, 0); 
osThreadDef(Consumer1_Thread, osPriorityNormal, 1, 0);
osThreadDef(Consumer2_Thread, osPriorityNormal, 1, 0);
osThreadDef(Cashier_Thread, osPriorityNormal, 1, 0);
#endif
// Thread ID
osThreadId T_x1;
osThreadId T_x2;
osThreadId T_x3;
osThreadId T_x4;

// Message ID and Definition
osMessageQId Q_LED;
osMessageQDef (Q_LED,0x16,unsigned char);
osEvent  result;

osMutexId x_mutex; // Mutex ID
osMutexDef(x_mutex); // Mutex definition
osMutexId y_mutex; // Mutex ID
osMutexDef(y_mutex); // Mutex definition

osSemaphoreId item_semaphore; // Semaphore ID
osSemaphoreDef(item_semaphore); // Semaphore definition
osSemaphoreId space_semaphore; // Semaphore ID
osSemaphoreDef(space_semaphore); // Semaphore definition

long int x=0;
long int i=0;
long int j=0;
long int k=0;
long int c=0;
unsigned int sym = 0x3A;

// Structure of the circular buffer (contain 8 spaces)
const unsigned int N = 8;
unsigned char buffer[N];
unsigned int insertPtr = 0;
unsigned int removePtr = 0;

// Input SET HERE
#define INPUT_SIZE 14
unsigned char INPUT[INPUT_SIZE] = "OUR_FIRST_TRY";
// Output Receive
unsigned char OUTPUT[INPUT_SIZE];

/*----------------------------------------------------------------------------
Test for item get from queue same with item put into queue
*----------------------------------------------------------------------------*/
void check_in_out(unsigned char *in,unsigned char *out)
{
	int check_flag = 1;
	SendChar('I');
	SendChar('N');
	SendChar('P');
	SendChar('U');
	SendChar('T');
	SendChar(':');
	for (i = 0; i < INPUT_SIZE ; i++){
		SendChar(in[i]);
	}
	SendChar('\n');
	
	SendChar('O');
	SendChar('U');
	SendChar('T');
	SendChar('P');
	SendChar('U');
	SendChar('T');
	SendChar(':');
	for (i = 0; i < INPUT_SIZE ; i++){
		SendChar(out[i]);
	}
	SendChar('\n');
	
	for (i = 0; i < INPUT_SIZE; i++)
	{
		if (in[i] != out[i])
		{
			check_flag = 0;
			break;
		}
	}
	SendChar('I');
	SendChar('N');
	SendChar('&');
	SendChar('O');
	SendChar('U');
	SendChar('T');
	SendChar(':');
	if (check_flag == 0)
	{
		SendChar('D');
		SendChar('I');
		SendChar('F');
		SendChar('F');
		SendChar('\n');
	}
	else
	{
		SendChar('S');
		SendChar('A');
		SendChar('M');
		SendChar('E');
		SendChar('\n');
	}
	
}

/*----------------------------------------------------------------------------
Producer insert item to circular buffer
*----------------------------------------------------------------------------*/
void put(unsigned char an_item){
	osSemaphoreWait(space_semaphore, osWaitForever);
	osMutexWait(x_mutex, osWaitForever);
	buffer[insertPtr] = an_item;
	insertPtr = (insertPtr + 1) % N;
	SendChar('P');
	SendChar(sym);
	SendChar(an_item);
	SendChar('\n');
	osMutexRelease(x_mutex);
	osSemaphoreRelease(item_semaphore);
}

/*----------------------------------------------------------------------------
Consumer get item from circular buffer
*----------------------------------------------------------------------------*/
unsigned char get(){
	unsigned int rr = 0x00;
	osSemaphoreWait(item_semaphore, osWaitForever);
	osMutexWait(x_mutex, osWaitForever);
	rr = buffer[removePtr];
	removePtr = (removePtr + 1) % N;
	osMutexRelease(x_mutex);
	osSemaphoreRelease(space_semaphore);
	return rr;
}

/*----------------------------------------------------------------------------
Producer Thread (put item)
*----------------------------------------------------------------------------*/
void Producer_Thread (void const *argument) 
{
	for(; i<INPUT_SIZE; i++){
		put(INPUT[i]);
	}
}

/*----------------------------------------------------------------------------
Consumer 1 Thread (get item)
*----------------------------------------------------------------------------*/
void Consumer1_Thread (void const *argument) 
{
	unsigned int data1 = 0x00;
	for(; j<INPUT_SIZE/2 ; j++){
		osMutexWait(y_mutex, osWaitForever);
		data1 = get();
		SendChar('G');
		SendChar('1');
		SendChar(sym);
		SendChar(data1);
		SendChar('\n');
		osMessagePut(Q_LED,data1,osWaitForever);// Place a value in the message queue
		osMutexRelease(y_mutex);
	}
}

/*----------------------------------------------------------------------------
Consumer 2 Thread (get item)
*----------------------------------------------------------------------------*/
void Consumer2_Thread (void const *argument) 
{
	unsigned int data2 = 0x00;
	for(; k<INPUT_SIZE/2; k++){
		osMutexWait(y_mutex, osWaitForever);
		data2 = get();
		SendChar('G');
		SendChar('2');
		SendChar(sym);
		SendChar(data2);
		SendChar('\n');
		osMessagePut(Q_LED,data2,osWaitForever);// Place a value in the message queue
		osMutexRelease(y_mutex);
	}
}

/*----------------------------------------------------------------------------
Cashier Thread (get transfered value from message queue)
*----------------------------------------------------------------------------*/
void Cashier_Thread(void const *argument)
{
	for(;c<INPUT_SIZE; c++){
		osMutexWait(y_mutex, osWaitForever);
		result = osMessageGet(Q_LED,osWaitForever);	// wait for a message to arrive
		OUTPUT[c] = result.value.v;
		SendChar('C');
		SendChar(sym);
		SendChar(result.value.v);
		SendChar('\n');
		osMutexRelease(y_mutex);
	}
	check_in_out(INPUT,OUTPUT);
}

/*----------------------------------------------------------------------------
main function
*----------------------------------------------------------------------------*/
int main (void) 
{
	osKernelInitialize ();// initialize CMSIS-RTOS
	USART1_Init();
	item_semaphore = osSemaphoreCreate(osSemaphore(item_semaphore), 0);
	space_semaphore = osSemaphoreCreate(osSemaphore(space_semaphore), N-1);
	x_mutex = osMutexCreate(osMutex(x_mutex));	
	y_mutex = osMutexCreate(osMutex(y_mutex));	
	
	Q_LED = osMessageCreate(osMessageQ(Q_LED),NULL);// create the message queue
	
	T_x1 = osThreadCreate(osThread(Producer_Thread), NULL);
	T_x2 = osThreadCreate(osThread(Consumer1_Thread), NULL);
	T_x3 = osThreadCreate(osThread(Consumer2_Thread), NULL);
	T_x4 = osThreadCreate(osThread(Cashier_Thread), NULL);
	
 
	osKernelStart ();// start thread execution 
	
}

