/** \file
 * \brief Example code for Simple Open EtherCAT master
 *
 * Usage : simple_test [ifname1]
 * ifname is NIC interface, f.e. eth0
 *
 * This is a minimal test.
 *
 * (c)Arthur Ketels 2010 - 2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "ethercat.h"

#define EC_TIMEOUTMON 500

#define CHK_TIMEOUT 60

char IOmap[4096];
OSAL_THREAD_HANDLE thread1;
int expectedWKC;
boolean needlf;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;
boolean forceByteAlignment = FALSE;

void simpletest(char *ifname, int cyclic_num)
{
	int i, j, k, oloop, iloop, chk, chk1;
	needlf = FALSE;
	inOP = FALSE;

	printf("Starting simple test\n");

	/* initialise SOEM, bind socket to ifname */
	chk = CHK_TIMEOUT;
	do
	{
		if (ec_init(ifname)) {
			break;
		}
		printf("No socket connection on %s, Retry %d.    \r", ifname, chk); fflush(stdout);
		osal_usleep(1000000);
	}
	while (chk--);
	printf("\n");

	if (chk <= 0) {
		printf("No socket connection on %s\nExit.\n", ifname);
		return;
	}
	printf("ec_init on %s succeeded.\n", ifname);

	chk1 = CHK_TIMEOUT;
	do
	{
		/* find and auto-config slaves */
		if ( ec_config_init(FALSE) > 0 )
		{
			printf("%d slaves found and configured.\n", ec_slavecount);

			/* wait for all slaves to reach SAFE_OP state */
			if (forceByteAlignment)
			{
				ec_config_map_aligned(&IOmap);
			}
			else
			{
				ec_config_map(&IOmap);
			}

			ec_configdc();

			printf("Slaves mapped, state to SAFE_OP.\n");
			if (ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE) != EC_STATE_SAFE_OP) {
				printf("State error!, Retry %d.\n", chk1);
				osal_usleep(1000000);
				continue;
			}

			oloop = ec_slave[0].Obytes;
			if ((oloop == 0) && (ec_slave[0].Obits > 0)) oloop = 1;
			if (oloop > 8) oloop = 8;
			iloop = ec_slave[0].Ibytes;
			if ((iloop == 0) && (ec_slave[0].Ibits > 0)) iloop = 1;
			if (iloop > 8) iloop = 8;

			printf("oloop : %d , iloop: %d\n", oloop, iloop);
			for(k = 1; k <= ec_slavecount ; k++) {
				printf("Slave %d [%s] found.\n", k, ec_slave[k].name);
				memset(ec_slave[k].outputs, 0, oloop);
				memset(ec_slave[k].inputs, 0, iloop);
			}

			printf("segments : %d : %d %d %d %d\n",ec_group[0].nsegments ,ec_group[0].IOsegment[0],ec_group[0].IOsegment[1],ec_group[0].IOsegment[2],ec_group[0].IOsegment[3]);

			printf("Request operational state for all slaves\n");
			expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
			printf("Calculated workcounter %d\n", expectedWKC);
			ec_slave[0].state = EC_STATE_OPERATIONAL;
			/* send one valid process data to make outputs in slaves happy*/
			ec_send_processdata();
			ec_receive_processdata(EC_TIMEOUTRET);
			/* request OP state for all slaves */
			ec_writestate(0);
			chk = CHK_TIMEOUT;
			/* wait for all slaves to reach OP state */
			do
			{
				printf("Wait for all slaves to reach OP state, Retry %d.    \r", chk); fflush(stdout);
				ec_send_processdata();
				ec_receive_processdata(EC_TIMEOUTRET);
				ec_statecheck(0, EC_STATE_OPERATIONAL, 1000000);
			}
			while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));
			printf("\n");

			if (ec_slave[0].state == EC_STATE_OPERATIONAL )
			{
				printf("Operational state reached for all slaves.\n");
				inOP = TRUE;
				/* cyclic loop */
				i = 1;
				while (cyclic_num < 0 ? 1 : i <= cyclic_num)
				{
					ec_send_processdata();
					wkc = ec_receive_processdata(EC_TIMEOUTRET);

					if(wkc >= expectedWKC)
					{
						printf("Processdata cycle %4d, WKC %d :\n", i, wkc);

						for(k = 1; k <= ec_slavecount ; k++) {
							/* Copy data in input buffer to output buffer */
							memcpy(ec_slave[k].outputs, ec_slave[k].inputs, oloop);

							printf(" [Slave %d 0x%04x] O:", k, ec_slave[k].configadr);
							for(j = 0 ; j < oloop; j++)
							{
								printf(" %2.2x", *(ec_slave[k].outputs + j));
							}
							printf(" I:");
							for(j = 0 ; j < iloop; j++)
							{
								printf(" %2.2x", *(ec_slave[k].inputs + j));
							}
							printf(" T:%"PRId64"\n",ec_DCtime);
							needlf = TRUE;
						}
					}
					osal_usleep(5000);
					i++;
				}
				inOP = FALSE;
			}
			else
			{
				printf("Not all slaves reached operational state.\n");
				ec_readstate();
				for(i = 1; i <= ec_slavecount ; i++)
				{
					if(ec_slave[i].state != EC_STATE_OPERATIONAL)
					{
						printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",
							i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
					}
				}
			}
			printf("\nRequest init state for all slaves\n");
			ec_slave[0].state = EC_STATE_INIT;
			/* request INIT state for all slaves */
			ec_writestate(0);
			break;
		}
		else
		{
			printf("No slaves found!, Retry %d.    \r", chk1); fflush(stdout);
			osal_usleep(1000000);
		}
	} while (chk1--);
	printf("\n");
	printf("End simple test, close socket\n");
	/* stop SOEM, close socket */
	ec_close();
}

OSAL_THREAD_FUNC ecatcheck( void *ptr )
{
	int slave;
	(void)ptr;                  /* Not used */

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

int main(int argc, char *argv[])
{
	int cyclic_num = -1;
	printf("SOEM (Simple Open EtherCAT Master)\nSimple test\n");

	if (argc > 1)
	{
		/* create thread to handle slave error handling in OP */
		// pthread_create( &thread1, NULL, (void *) &ecatcheck, (void*) &ctime);
		osal_thread_create(&thread1, 128000, &ecatcheck, (void*) &ctime);
		/* start cyclic part */
		if (argc > 2) {
			cyclic_num = atoi(argv[2]);
		}
		simpletest(argv[1], cyclic_num);
	}
	else
	{
		ec_adaptert * adapter = NULL;
		printf("Usage: simple_test [ifname] [count]\n");
		printf("  ifname = Ethernet interface (eth0 for example)\n");
		printf("  count = Number of test (-1:Nonstop)\n");

		printf ("\nAvailable adapters:\n");
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
