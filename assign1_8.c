#include "types.h"
#include "stat.h"
#include "user.h"

#define MSGSIZE 8

int tot_digits(int num){
	int count = 0;
	while(num != 0){
		num /= 10;
		count++;
	}
	return count;
}

void int_to_string(char *result, int sum, int dig, char signal){
	int x = '0';
	for(int i=0; i<dig; i++){
		int las_dig = sum%10;
		result[dig - 1 - i] = las_dig + x;			
		sum = sum/10;
	}

	for(int i=dig; i<MSGSIZE-2; i++){
		result[i] = 'x';
	}

    result[MSGSIZE-2] = signal;  //to indicate what kind of message is sent
}

int string_to_int(char *result, int *sig){
	//int tot = sizeof(result)/sizeof(result[0]);
	int ans = 0;
	for(int i=0; i<MSGSIZE-2; i++){
		if(result[i] >= '0' && result[i] <= '9') ans = 10*ans + (result[i] - '0'); 
	}
    int v = result[MSGSIZE-2] - '0';
    *sig = v;
    //printf(1, "sig %d", v);
	return ans;
}

void print_variance(float xx)
{
	int beg=(int)(xx);
	int fin=(int)(xx*100)-beg*100;
	printf(1, "Variance of array for the file arr is %d.%d", beg, fin);
}

int
main(int argc, char *argv[])
{	
	
	if(argc< 2){
		printf(1,"Need type and input filename\n");
		exit();
	}
	char *filename;
	filename=argv[2];
	int type = atoi(argv[1]);
	//printf(1,"Type is %d and filename is %s\n",type, filename);
	
	int tot_sum = 0;	
    float mean = 0.0;
    float variance = 0.0;


	int size=1000;
	short arr[size];
	
	
	char c;
	int fd = open(filename, 0);
	for(int i=0; i<size; i++){
		read(fd, &c, 1);
		arr[i]=c-'0';
		read(fd, &c, 1);
	}	
  	close(fd);
	

	// for(int i=0; i<size; i++){
	// 	arr[i] = (i%125)%9;
	// }

  	// this is to supress warning
  	//printf(1,"first elem %d\n", arr[0]);
  
  	//----FILL THE CODE HERE for unicast sum
	
	int leader_pid = getpid();
	int n1 = fork();
	int n2 = fork();
	int n3 = fork();

	if(n1 > 0 && n2 > 0 && n3 > 0){

		int sum = 0;
		for(int i=0; i<125; i++){
			sum += arr[i];
		}
		tot_sum += sum;
        mean += sum;
		//printf(1, "sum is %d\n", sum);	

		char *msg = (char *)malloc(MSGSIZE);
        int *r_ind = (int *)malloc(8 * sizeof(int));

        r_ind[7] = -10; // invalid pid deliberately
        int ind = 0;

		for(int i=0; i<14; i++){
			
			int stat=-1;
			while(stat==-1){
				//printf(1, "in loop");
				stat = recv(msg);
			}
            int p = 0;
            //printf(1, "%d\n", p);
			int val = string_to_int(msg, &p);
            //printf(1, "%d\n", p);
            

            if(p == 'a' - '0'){
                //printf(1, "value %d\n", val);
                int partial_sum = val;
                tot_sum += partial_sum;
                mean += partial_sum;
            }
            else if(p == 'b' - '0'){
                //printf(1, "pid value %d\n", val);
                r_ind[ind] = val;
                ind++;
            }
			//printf(1,"2 CHILD: msg marecv is: %d \n", partial_sum);
			
		}
        
        mean /= 1000; 

        int i;
        for(i = 0; i < 4; i++) msg[i] = *((char *)&mean + i);
        
        send_multi(leader_pid, r_ind, msg);

        
  		for(i = 0; i < 125; i++){
  			float d = (mean - (float)arr[i]);
  			variance +=  d * d;
  		}

        for(int i=0; i<7; i++){
            int stat=-1;
			while(stat==-1){
				stat = recv(msg);
			}
            variance += *((float *)msg);
        }

        variance /= (float)1000;
        //printf(1, "var is %d\n", (int)variance);
        free(r_ind);
        free(msg);
		
		wait();
		wait();
		wait();
	}
	else if(n1 > 0 && n2 > 0 && n3 == 0){
		
        int sum = 0;
		for(int i=125; i<250; i++){
			sum += arr[i];
		}
		
		char *msg = (char *)malloc(MSGSIZE);

        int process_id = getpid();
		int dig1 = tot_digits(sum);
		int_to_string(msg, sum, dig1, 'a');
		//printf(1,"1 PARENT: msg sent is: %s \n", msg);
		send(process_id,leader_pid, msg);	

        int dig2 = tot_digits(process_id);
        int_to_string(msg, process_id, dig2, 'b');
        send(process_id, leader_pid, msg);

        int stat=-1;
		while(stat==-1){
			stat = recv(msg);
		}
        float avg_global = *((float *)msg);

        int i;
        float part_var = 0.0;
  		for(i = 125; i < 250; i++){
  			float d = (avg_global - (float)arr[i]);
  			part_var +=  d * d;
  		}

        for(i = 0; i < 4; i++) msg[i] = *((char*)&part_var + i);
        send(process_id, leader_pid, msg);

		free(msg);
		
		wait();
		wait();
		exit();
		
	}
	else if(n1 == 0 &&  n2 > 0 && n3 > 0){
		
        int sum = 0;
		for(int i=250; i<375; i++){
			sum += arr[i];
		}
		
		char *msg = (char *)malloc(MSGSIZE);

        int process_id = getpid();
		int dig1 = tot_digits(sum);
		int_to_string(msg, sum, dig1, 'a');
		//printf(1,"1 PARENT: msg sent is: %s \n", msg);
		send(process_id,leader_pid, msg);	

        int dig2 = tot_digits(process_id);
        int_to_string(msg, process_id, dig2, 'b');
        send(process_id, leader_pid, msg);

        int stat=-1;
		while(stat==-1){
			stat = recv(msg);
		}
        float avg_global = *((float *)msg);

        int i;
        float part_var = 0.0;
  		for(i = 250; i < 375; i++){
  			float d = (avg_global - (float)arr[i]);
  			part_var +=  d * d;
  		}

        for(i = 0; i < 4; i++) msg[i] = *((char*)&part_var + i);
        send(process_id, leader_pid, msg);

		free(msg);

		wait();
		wait();
		exit();
	}
	else if(n1 == 0 &&  n2 > 0 && n3 == 0){
		
        int sum = 0;
		for(int i=375; i<500; i++){
			sum += arr[i];
		}
		
		char *msg = (char *)malloc(MSGSIZE);

        int process_id = getpid();
		int dig1 = tot_digits(sum);
		int_to_string(msg, sum, dig1, 'a');
		//printf(1,"1 PARENT: msg sent is: %s \n", msg);
		send(process_id,leader_pid, msg);	

        int dig2 = tot_digits(process_id);
        int_to_string(msg, process_id, dig2, 'b');
        send(process_id, leader_pid, msg);

        int stat=-1;
		while(stat==-1){
			stat = recv(msg);
		}
        float avg_global = *((float *)msg);

        int i;
        float part_var = 0.0;
  		for(i = 375; i < 500; i++){
  			float d = (avg_global - (float)arr[i]);
  			part_var +=  d * d;
  		}

        for(i = 0; i < 4; i++) msg[i] = *((char*)&part_var + i);
        send(process_id, leader_pid, msg);

		free(msg);

		wait();
		exit();
	}
	else if(n1 > 0 && n2 == 0 && n3  > 0){
		
        int sum = 0;
		for(int i=500; i<625; i++){
			sum += arr[i];
		}

		char *msg = (char *)malloc(MSGSIZE);

        int process_id = getpid();
		int dig1 = tot_digits(sum);
		int_to_string(msg, sum, dig1, 'a');
		//printf(1,"1 PARENT: msg sent is: %s \n", msg);
		send(process_id,leader_pid, msg);	

        int dig2 = tot_digits(process_id);
        int_to_string(msg, process_id, dig2, 'b');
        send(process_id, leader_pid, msg);

        int stat=-1;
		while(stat==-1){
			stat = recv(msg);
		}
        float avg_global = *((float *)msg);

        int i;
        float part_var = 0.0;
  		for(i = 500; i < 625; i++){
  			float d = (avg_global - (float)arr[i]);
  			part_var +=  d * d;
  		}

        for(i = 0; i < 4; i++) msg[i] = *((char*)&part_var + i);
        send(process_id, leader_pid, msg);

		free(msg);

		wait();
		exit();	
	}
	else if(n1 > 0 && n2 == 0 && n3  == 0){
		
        int sum = 0;
		for(int i=625; i<750; i++){
			sum += arr[i];
		}
		
		char *msg = (char *)malloc(MSGSIZE);

        int process_id = getpid();
		int dig1 = tot_digits(sum);
		int_to_string(msg, sum, dig1, 'a');
		//printf(1,"1 PARENT: msg sent is: %s \n", msg);
		send(process_id,leader_pid, msg);	

        int dig2 = tot_digits(process_id);
        int_to_string(msg, process_id, dig2, 'b');
        send(process_id, leader_pid, msg);

        int stat=-1;
		while(stat==-1){
			stat = recv(msg);
		}
        float avg_global = *((float *)msg);

        int i;
        float part_var = 0.0;
  		for(i = 625; i < 750; i++){
  			float d = (avg_global - (float)arr[i]);
  			part_var +=  d * d;
  		}

        for(i = 0; i < 4; i++) msg[i] = *((char*)&part_var + i);
        send(process_id, leader_pid, msg);

		free(msg);

		wait();
		exit();
	}
	else if(n1 == 0 && n2 == 0 && n3 > 0){
		int sum = 0;
		for(int i=750; i<875; i++){
			sum += arr[i];
		}
		

		char *msg = (char *)malloc(MSGSIZE);

        int process_id = getpid();
		int dig1 = tot_digits(sum);
		int_to_string(msg, sum, dig1, 'a');
		//printf(1,"1 PARENT: msg sent is: %s \n", msg);
		send(process_id,leader_pid, msg);	

        int dig2 = tot_digits(process_id);
        int_to_string(msg, process_id, dig2, 'b');
        send(process_id, leader_pid, msg);

        int stat=-1;
		while(stat==-1){
			stat = recv(msg);
		}
        float avg_global = *((float *)msg);

        int i;
        float part_var = 0.0;
  		for(i = 750; i < 875; i++){
  			float d = (avg_global - (float)arr[i]);
  			part_var +=  d * d;
  		}

        for(i = 0; i < 4; i++) msg[i] = *((char*)&part_var + i);
        send(process_id, leader_pid, msg);

		free(msg);

		wait();
		exit();
		
	}
	else{
		int sum = 0;
		for(int i=875; i<1000; i++){
			sum += arr[i];
		}
	
		char *msg = (char *)malloc(MSGSIZE);

        int process_id = getpid();
		int dig1 = tot_digits(sum);
		int_to_string(msg, sum, dig1, 'a');
		//printf(1,"1 PARENT: msg sent is: %s \n", msg);
		send(process_id,leader_pid, msg);	

        int dig2 = tot_digits(process_id);
        int_to_string(msg, process_id, dig2, 'b');
        send(process_id, leader_pid, msg);

        int stat=-1;
		while(stat==-1){
			stat = recv(msg);
		}
        float avg_global = *((float *)msg);

        int i;
        float part_var = 0.0;
  		for(i = 875; i < 1000; i++){
  			float d = (avg_global - (float)arr[i]);
  			part_var +=  d * d;
  		}

        for(i = 0; i < 4; i++) msg[i] = *((char*)&part_var + i);
        send(process_id, leader_pid, msg);

		free(msg);

		//wait();
		exit();
	}

	
	// printf(1, "Sum of array is %d\n", tot_sum);
    // printf(1, "Variance is %d\n", (int)variance);
	// exit();

  	//------------------
  	if(type==0){ //unicast sum
		printf(1,"Sum of array for file %s is %d\n", filename,tot_sum);
	}
	else{ //mulicast variance
		//printf(1,"Variance of array for file %s is %d\n", filename, (int)variance);
		print_variance(variance);
	}
	exit();
	
}