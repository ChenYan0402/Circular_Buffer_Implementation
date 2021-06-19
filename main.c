/*----------------------------------------------------------------------------
	Circular Buffer Implementation
*----------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "cmsis_os.h"
#include "uart.h"

void Producer_Thread (void const *argument);
void Consumer_Thread (void const *argument);
/*----------------------------------------------------------------------------
	Test for full buffer condition
*----------------------------------------------------------------------------*/
//osThreadDef(Producer_Thread, osPriorityHigh, 1, 0);
//osThreadDef(Consumer_Thread, osPriorityNormal, 1, 0);

/*----------------------------------------------------------------------------
	Test for empty buffer condition
*----------------------------------------------------------------------------*/
//osThreadDef(Producer_Thread, osPriorityNormal, 1, 0);
//osThreadDef(Consumer_Thread, osPriorityHigh, 1, 0);

osThreadDef(Producer_Thread, osPriorityNormal, 1, 0);
osThreadDef(Consumer_Thread, osPriorityNormal, 1, 0);

osThreadId T_x1; // Thread ID
osThreadId T_x2; // Thread ID
	
osMutexId x_mutex; // Mutex ID
osMutexDef(x_mutex); // Mutex definition
osSemaphoreId item_semaphore; // Semaphore ID
osSemaphoreDef(item_semaphore); // Semaphore definition
osSemaphoreId space_semaphore; // Semaphore ID
osSemaphoreDef(space_semaphore); // Semaphore definition



// Input SET HERE
unsigned char INPUT[14] = "OUR_FIRST_TRY";
#define INPUT_SIZE 14
// Output Receive
unsigned char OUTPUT[INPUT_SIZE];

long int x=0;
long int i=0;
long int j=0;
unsigned int sym = 0x3A;

// Structure of the circular buffer (contain 8 spaces)
const unsigned int N = 8; 
unsigned char buffer[N]; 
unsigned int insertPtr = 0; //increase insertPtr when item is put into buffer
unsigned int removePtr = 0; // increase removedPtr when item is get from buffer

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
	osSemaphoreWait(space_semaphore, osWaitForever); // wait for empty space
	osMutexWait(x_mutex, osWaitForever); // guarantee the atomic (protect critical section)
	buffer[insertPtr] = an_item; // insert item
	insertPtr = (insertPtr + 1) % N; // %N is used mainly to provide circular structure (7->0)
	//SendChar('P');
	//SendChar(sym);
	//SendChar(an_item);
	//SendChar('\n');
	osMutexRelease(x_mutex);
	osSemaphoreRelease(item_semaphore);
}

/*----------------------------------------------------------------------------
Consumer get item from circular buffer
*----------------------------------------------------------------------------*/
unsigned char get(){
	unsigned int item = 0x00; // initialise variable for storing the item get from buffer
	osSemaphoreWait(item_semaphore, osWaitForever); // wait for item
	osMutexWait(x_mutex, osWaitForever); // guarantee the atomic (protect critical section)
	item = buffer[removePtr]; // get item and store to variable "item"
	removePtr = (removePtr + 1) % N; // %N is used mainly to provide circular structure (7->0)
	osMutexRelease(x_mutex);
	osSemaphoreRelease(space_semaphore);
	return item;
}

int loopcount = INPUT_SIZE;

/*----------------------------------------------------------------------------
Producer Thread (item insert from A-Z)
*----------------------------------------------------------------------------*/
void Producer_Thread (void const *argument) 
{
	for(i=0; i<loopcount; i++){
		put(INPUT[i]);
	}
}
/*----------------------------------------------------------------------------
Consumer Thread (get item from A-Z)
*----------------------------------------------------------------------------*/
void Consumer_Thread (void const *argument) 
{
	for(j=0; j<loopcount; j++){
		unsigned int data = 0x00;
		long int check = 0;
		check = j%8;
		
		data = get();
		OUTPUT[j] = data ;
		SendChar('G');
		SendChar(sym);
		SendChar(data);
		SendChar('\n');	
		
		if (data==buffer[check]) // test for value get from queue same with item put into queue
		{
			SendChar('o');
			SendChar('k');
			SendChar('\n');
		}
	}
	check_in_out(INPUT,OUTPUT);
}
/*----------------------------------------------------------------------------
main function
*----------------------------------------------------------------------------*/
int main (void) 
{
	osKernelInitialize ();                    // initialize CMSIS-RTOS
	USART1_Init();
	item_semaphore = osSemaphoreCreate(osSemaphore(item_semaphore), 0);
	space_semaphore = osSemaphoreCreate(osSemaphore(space_semaphore), N-1); 
	x_mutex = osMutexCreate(osMutex(x_mutex));	
	
	T_x1 = osThreadCreate(osThread(Producer_Thread), NULL); // producer
	T_x2 = osThreadCreate(osThread(Consumer_Thread), NULL); // consumer
 
	osKernelStart ();                         // start thread execution 
		
}


