/** \file
 * \brief Example code for Simple Open EtherCAT master
 *
 * Usage : red_test [ifname1] [ifname2] [cycletime]
 * ifname is NIC interface, f.e. eth0
 * cycletime in us, f.e. 500
 *
 * This is a redundancy test.
 *
 * (c)Arthur Ketels 2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <inttypes.h>

#include "ethercat.h"

#define NSEC_PER_SEC 1000000000
#define EC_TIMEOUTMON 500
#define CHK_TIMEOUT 30

struct sched_param schedp;
char IOmap[4096];
pthread_t thread1, thread2;
struct timeval tv, t1, t2;
long long int dorun = 0;
int deltat, tmax = 0;
int64 toff, gl_delta;
int DCdiff;
int os;
uint8 ob;
uint16 ob2;
uint8 *digout = 0;
int expectedWKC;
boolean needlf;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;
int quiet = 0;


void redtest(char *ifname, char *ifname2)
{
	int cnt, i, j, k, oloop, iloop, chk, chk1;

	printf("Starting Redundant test\n");

	chk = CHK_TIMEOUT;
	do
	{
		/* initialise SOEM, bind socket to ifname */
		if (ec_init_redundant(ifname, ifname2))
		{
			break;
		}
		printf("No socket connection on %s and %s, Retry %d.    \r", ifname, ifname2, chk); fflush(stdout);
		osal_usleep(1000000);
	}
	while (chk--);
	printf("\n");

	if (chk <= 0) {
		printf("No socket connection on %s or %s\nExit.\n", ifname, ifname2);
		return;
	} else {
		printf("ec_init_redundant on %s and %s succeeded.\n", ifname, ifname2);
		chk1 = CHK_TIMEOUT;
		do
		{
			/* find and auto-config slaves */
			if ( ec_config(FALSE, &IOmap) > 0 )
			{
				printf("%d slaves found and configured.\n",ec_slavecount);
				/* wait for all slaves to reach SAFE_OP state */
				ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE);

				/* configure DC options for every DC capable slave found in the list */
				ec_configdc();

				/* read indevidual slave state and store in ec_slave[] */
				ec_readstate();
				for(cnt = 1; cnt <= ec_slavecount ; cnt++)
				{
					printf("Slave:%d Name:%s Output size:%3dbits Input size:%3dbits State:%2d delay:%d.%d\n",
							cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits,
							ec_slave[cnt].state, (int)ec_slave[cnt].pdelay, ec_slave[cnt].hasdc);
					printf("         Out:%p,%4d In:%p,%4d\n",
							ec_slave[cnt].outputs, ec_slave[cnt].Obytes, ec_slave[cnt].inputs, ec_slave[cnt].Ibytes);
					/* check for EL2004 or EL2008 */
					if( !digout && ((ec_slave[cnt].eep_id == 0x0af83052) || (ec_slave[cnt].eep_id == 0x07d83052)))
					{
						digout = ec_slave[cnt].outputs;
					}
				}
				expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
				printf("Calculated workcounter %d\n", expectedWKC);

				printf("Request operational state for all slaves\n");
				ec_slave[0].state = EC_STATE_OPERATIONAL;

				/* request OP state for all slaves */
				ec_writestate(0);

				/* activate cyclic process data */
				dorun = 1;

				/* wait for all slaves to reach OP state */
				ec_statecheck(0, EC_STATE_OPERATIONAL,  5 * EC_TIMEOUTSTATE);
				oloop = ec_slave[0].Obytes;
				if ((oloop == 0) && (ec_slave[0].Obits > 0)) oloop = 1;
				if (oloop > 8) oloop = 8;
				iloop = ec_slave[0].Ibytes;
				if ((iloop == 0) && (ec_slave[0].Ibits > 0)) iloop = 1;
				if (iloop > 8) iloop = 8;

				/* Clean input/output buffer */
				printf("Clean buffer, oloop : %d , iloop: %d\n", oloop, iloop);
				for(k = 1; k <= ec_slavecount ; k++) {
					printf("Slave %d [%s] found. (%u,%u,%u,%u)\n", k, ec_slave[k].name, ec_slave[k].Obytes, ec_slave[k].Obits, ec_slave[k].Ibytes, ec_slave[k].Ibits);
					if (ec_slave[k].Obytes > 0) memset(ec_slave[k].outputs, 0, oloop);
					if (ec_slave[k].Ibytes > 0) memset(ec_slave[k].inputs, 0, iloop);
				}

				if (ec_slave[0].state == EC_STATE_OPERATIONAL )
				{
					printf("Operational state reached for all slaves.\n");
					inOP = TRUE;
					/* acyclic loop */
					while(wkc > 0 && inOP)
					{
						if (!quiet) {
							for(k = 1; k <= ec_slavecount ; k++) {
								printf(" (%5lld, %3d) [Slave %d 0x%04x] O:", dorun, wkc, k, ec_slave[k].configadr);
								for(j = oloop - 1 ; j >= 0; j--)
								{
									if (ec_slave[k].Obytes > 0) printf("%2.2x", *(ec_slave[k].outputs + j));
								}
								printf(" I:");
								for(j = iloop - 1 ; j >= 0; j--)
								{
									if (ec_slave[k].Ibytes > 0) printf("%2.2x", *(ec_slave[k].inputs + j));
								}
								printf(" T:%12"PRId64", dt:%12"PRId64"\n", ec_DCtime, gl_delta);
								needlf = TRUE;
								fflush(stdout);
							}
						}
						osal_usleep(20000);
					}
					dorun = 0;
					inOP = FALSE;
				}
				else
				{
					printf("Not all slaves reached operational state.\n");
					ec_readstate();
					for(i = 1; i<=ec_slavecount ; i++)
					{
						if(ec_slave[i].state != EC_STATE_OPERATIONAL)
						{
								printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",
									i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
						}
					}
				}
				printf("Request safe operational state for all slaves\n");
				ec_slave[0].state = EC_STATE_SAFE_OP;
				/* request SAFE_OP state for all slaves */
				ec_writestate(0);
			}
			else
			{
				printf("No slaves found!, Retry %d.    \r", chk1); fflush(stdout);
				osal_usleep(1000000);
			}

		} while (chk1--);
		printf("\n");
		printf("End redundant test, close socket\n");
		/* stop SOEM, close socket */
		ec_close();
	}
}

/* add ns to timespec */
void add_timespec(struct timespec *ts, int64 addtime)
{
	int64 sec, nsec;

	nsec = addtime % NSEC_PER_SEC;
	sec = (addtime - nsec) / NSEC_PER_SEC;
	ts->tv_sec += sec;
	ts->tv_nsec += nsec;
	if ( ts->tv_nsec >= NSEC_PER_SEC )
	{
		nsec = ts->tv_nsec % NSEC_PER_SEC;
		ts->tv_sec += (ts->tv_nsec - nsec) / NSEC_PER_SEC;
		ts->tv_nsec = nsec;
	}
}

/* PI calculation to get linux time synced to DC time */
void ec_sync(int64 reftime, int64 cycletime , int64 *offsettime)
{
	static int64 integral = 0;
	int64 delta;
	/* set linux sync point 50us later than DC sync, just as example */
	delta = (reftime - 50000) % cycletime;
	if(delta> (cycletime / 2)) { delta= delta - cycletime; }
	if(delta>0){ integral++; }
	if(delta<0){ integral--; }
	*offsettime = -(delta / 100) - (integral / 20);
	gl_delta = delta;
}

/* RT EtherCAT thread */
OSAL_THREAD_FUNC_RT ecatthread(void *ptr)
{
	struct timespec   ts, tleft;
	int ht, k;
	int64 cycletime;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	ht = (ts.tv_nsec / 1000000) + 1; /* round to nearest ms */
	ts.tv_nsec = ht * 1000000;
	if (ts.tv_nsec >= NSEC_PER_SEC) {
		ts.tv_sec++;
		ts.tv_nsec -= NSEC_PER_SEC;
	}
	cycletime = *(int*)ptr * 1000; /* cycletime in ns */
	toff = 0;
	dorun = 0;
	ec_send_processdata();
	while(1)
	{
		/* calculate next cycle start */
		add_timespec(&ts, cycletime + toff);
		/* wait to cycle start */
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, &tleft);
		if (dorun>0)
		{
			wkc = ec_receive_processdata(EC_TIMEOUTRET);
			if(wkc >= expectedWKC)
			{
				for(k = 1; k <= ec_slavecount ; k++) {
					if (ec_slave[k].Ibytes > 0) {
						if (*((uint64*)ec_slave[k].inputs) == 0) {
							/* Set to 1 to tell slaves start test */
							*((uint64*)ec_slave[k].outputs) = 1;
						} else {
							/* Copy data in input buffer to output buffer */
							memcpy(ec_slave[k].outputs, ec_slave[k].inputs, ec_slave[k].Obytes);
						}
					}
				}
			}
			dorun++;
			/* if we have some digital output, cycle */
			if( digout ) *digout = (uint8) ((dorun / 16) & 0xff);

			if (ec_slave[0].hasdc)
			{
				/* calulate toff to get linux time and DC synced */
				ec_sync(ec_DCtime, cycletime, &toff);
			}
			ec_send_processdata();
		}
	}
}

OSAL_THREAD_FUNC ecatcheck( void *ptr )
{
	int slave;

	(void) ptr;

	while(1)
	{
		if( inOP && ((wkc < expectedWKC) || ec_group[currentgroup].docheckstate))
		{
			if (needlf)
			{
				needlf = FALSE;
				printf("\n");
			}
			/* one ore more slaves are not responding */
			ec_group[currentgroup].docheckstate = FALSE;
			ec_readstate();
			for (slave = 1; slave <= ec_slavecount; slave++)
			{
				if ((ec_slave[slave].group == currentgroup) && (ec_slave[slave].state != EC_STATE_OPERATIONAL))
				{
					ec_group[currentgroup].docheckstate = TRUE;
					if (ec_slave[slave].state == (EC_STATE_SAFE_OP + EC_STATE_ERROR))
					{
						printf("ERROR : slave %d is in SAFE_OP + ERROR, attempting ack.\n", slave);
						ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);
						ec_writestate(slave);
					}
					else if(ec_slave[slave].state == EC_STATE_SAFE_OP)
					{
						printf("WARNING : slave %d is in SAFE_OP, change to OPERATIONAL.\n", slave);
						ec_slave[slave].state = EC_STATE_OPERATIONAL;
						ec_writestate(slave);
					}
					else if(ec_slave[slave].state > EC_STATE_NONE)
					{
						if (ec_reconfig_slave(slave, EC_TIMEOUTMON))
						{
							ec_slave[slave].islost = FALSE;
							printf("MESSAGE : slave %d reconfigured\n",slave);
						}
					}
					else if(!ec_slave[slave].islost)
					{
						/* re-check state */
						ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
						if (ec_slave[slave].state == EC_STATE_NONE)
						{
							ec_slave[slave].islost = TRUE;
							printf("ERROR : slave %d lost\n",slave);
							inOP = FALSE;
						}
					}
				}
				if (ec_slave[slave].islost)
				{
					if(ec_slave[slave].state == EC_STATE_NONE)
					{
						if (ec_recover_slave(slave, EC_TIMEOUTMON))
						{
							ec_slave[slave].islost = FALSE;
							printf("MESSAGE : slave %d recovered\n",slave);
						}
					}
					else
					{
						ec_slave[slave].islost = FALSE;
						printf("MESSAGE : slave %d found\n",slave);
					}
				}
			}
			if(!ec_group[currentgroup].docheckstate)
				printf("OK : all slaves resumed OPERATIONAL.\n");
		}
		osal_usleep(10000);
	}
}

#define stack64k (64 * 1024)

int main(int argc, char *argv[])
{
	int ctime = 10000;

	printf("SOEM (Simple Open EtherCAT Master)\nRedundancy test\nVersion %s\n", APP_VERSION);

	if ((argc > 1) && (argv[1][0] == '-') && (argv[1][1] == 'q'))
	{
		quiet = 1;
		argc--;
		argv++;
	}

	if (argc > 3)
	{
		dorun = 0;
		ctime = atoi(argv[3]);

		/* create RT thread */
		osal_thread_create_rt(&thread1, stack64k * 2, &ecatthread, (void*) &ctime);

		/* create thread to handle slave error handling in OP */
		osal_thread_create(&thread2, stack64k * 4, &ecatcheck, NULL);

		/* start acyclic part */
		while (1) {
			redtest(argv[1], argv[2]);
			printf("Redundancy test exit, re-load it.\n"); fflush(stdout);
		}
	}
	else
	{
		printf("Usage: red_test [-q] [ifname1] [ifname2] [cy_time]\n");
		printf("  -q     : Quiet mode\n");
		printf("  ifname1 : Primary Dev name, f.e. eth0\n");
		printf("  ifname2 : Secondary Dev name, f.e. eth1\n");
		printf("  cy_time : Cycle time in us\n");

		printf ("\nAvailable adapters (ifname):\n");
		ec_adaptert * adapter = NULL;
		adapter = ec_find_adapters ();
		while (adapter != NULL)
		{
			printf ("    - %s  (%s)\n", adapter->name, adapter->desc);
			adapter = adapter->next;
		}
		ec_free_adapters(adapter);
	}

	printf("End program\n");

	return (0);
}
