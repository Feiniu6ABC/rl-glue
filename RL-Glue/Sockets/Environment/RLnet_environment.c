#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <RLcommon.h>
#include <RLnet/RLnet.h>

#ifdef NETWORK_DEBUG
#define RLNET_DEBUG(x) x
#else
#define RLNET_DEBUG(x)
#endif

/* Declare the task spec and length */
unsigned int theTaskSpecLength;
char* theTaskSpecBuffer;

/* Declare observation action and reward */
Action theAction;
Observation theObservation;
Reward theReward;

/* Provide forward declaration of environment interface */
extern Task_specification env_init();
extern Observation env_start();
extern Reward_observation env_step(Action a);
extern void env_cleanup();
extern void env_set_state(State_key sk);
extern void env_set_random_seed(Random_seed_key rsk);
extern State_key env_get_state();
extern Random_seed_key env_get_random_seed();

const char* kUnknownMessage = "Unknown Message: %s\n";
const char* kUsage = "Usage: Environment <ip-address> <port>\n";

void print_reward(Reward theReward)
{
  fprintf(stderr, "ENV RECV: theReward %f\n", theReward);
}

void print_action_header(Action theAction)
{
  fprintf(stderr, "ENV SENT: theAction.numInts = %d\n", theAction.numInts);
  fprintf(stderr, "ENV SENT: theAction.numDoubles = %d\n", theAction.numDoubles);
}

void print_observation_header(Observation theObservation)
{
  fprintf(stderr, "ENV RECV: theObservation.numInts = %d\n", theObservation.numInts);
  fprintf(stderr, "ENV RECV: theObservation.numDoubles = %d\n", theObservation.numDoubles);
}

void on_env_init(rlSocket theConnection)
{
  /* Task_specification env_init(); */  
  theTaskSpec = env_init();
  theTaskSpecLength = strlen(theTaskSpec) + 1;

  rlSendMessageHeader(theConnection, &theTaskSpecLength);
  rlSendMessageBody(theConnection, theTaskSpec, theTaskSpecLength);
}

void on_env_start(rlSocket theConnection)
{
  /* Observation env_start(); */

  theObservation = env_start();
  rlSendObservation(theConnection, theObservation);
}

void on_env_step(rlSocket theConnection)
{
  /* Reward_observation env_step(Action a); */
  
  /* need to deal with allocation / dealloction of the action! */
  rlRecvAction(theConnection, &theAction);  
  Reward_observation ro = env_step(theAction);
}

void on_env_cleanup(rlSocket theConnection)
{
  /* void env_cleanup(); */
}

void on_env_set_state(rlSocket theConnection)
{
  /* void env_set_state(State_key sk); */
}

void on_env_set_random_seed(rlSocket theConnection)
{
  /* void env_set_random_seed(Random_seed_key rsk); */
}

void on_env_get_state(rlSocket theConnection)
{
  /* State_key env_get_state(); */
}

void on_env_get_random_seed(rlSocket theConnection)
{
  /* Random_seed_key env_get_random_seed(); */
}

void run_env(rlSocket theConnection)
{
  char theMessage[8] = {0};

  do
  { 
    rlRecvData(theConnection, theMessage, 8);
    RLNET_DEBUG( fprintf(stderr, "ENV RECV: %s\n", theMessage); )
	      
    if (strncmp(theMessage, "init", 8) == 0)
    {
      on_env_init(theConnection);
    }
    else if (strncmp(theMessage, "start", 8) == 0)
    {
      on_env_start(theConnection);
    }
    else if (strncmp(theMessage, "step", 8) == 0)
    {
      on_env_step(theConnection);
    }
    else if (strncmp(theMessage, "cleanup", 8) == 0)
    {
      on_env_cleanup(theConnection);
    }
    else if ( strncmp(theMessage, "setsk", 8) == 0)
    {
      on_env_set_state(theConnection);
    }
    else if ( strncmp(theMessage, "setrsk", 8) == 0)
    {
      on_env_set_random_seed(theConnection);
    }
    else if ( strncmp(theMessage, "getsk", 8) == 0)
    {
      on_env_get_state(theConnection);
    }
    else if ( strncmp(theMessage, "getrsk", 8) == 0)
    {
      on_env_get_random_seed(theConnection);
    }
    else
    {
      fprintf(stderr, kUnknownMessage, theMessage);
      break;
    }
  } while (strncmp(theMessage, "cleanup", 8) != 0);
}

int main(int argc, char** argv)
{
  rlSocket theConnection;
  short thePort = 0;

  if (argc != 3) 
  {
    fprintf(stderr, kUsage);
    return 1;
  }

  sscanf(argv[2], "%hd", &thePort);

  theConnection = rlOpen(thePort);
  assert(rlIsValidSocket(theConnection));

  assert(rlConnect(theConnection, argv[1]) >= 0);
  run_env(theConnection);
  assert(rlClose(theConnection) >= 0);

  return 0;
}