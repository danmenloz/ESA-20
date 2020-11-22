#include "shield.h"

#define N_COPIES  (2)

static int set_current[N_COPIES+1]; // original value saved at index 0

void Write_Set_Current(int new_value){
	g_set_current = new_value;
	for (int i=1; i<N_COPIES+1; i++)
		set_current[i] = new_value;
}

int Read_Set_Current(void){
	//Boyer–Moore majority vote algorithm
	set_current[0] = g_set_current; //store original value at index 0
	int m = 0; // majority value
	int c = 0; // vote counter
	
	for (int i=0; i<N_COPIES+1; i++){
		if(c==0){
			m = set_current[i];
			c+=1;
		}
		else{
			if(m==set_current[i])
				c+=1;
			else
				c-=1;
		}
	}
	// update variable and return majority value
	g_set_current = m;
	return m;
	
}
