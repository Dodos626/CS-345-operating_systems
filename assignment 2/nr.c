
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>

sem_t semaphore_west_side;

sem_t semaphore_east_side;

sem_t semaphore_side;

pthread_mutex_t safelock;

int cars_on_road = 0;

/*0 is for goint to west 1 is for going to east 2 if it's free*/
int cars_on_road_way;
int cars_waiting = 0;

void *routine_west(void *args)
{
  printf("Car %d: Waiting to pass to East\n", *(int *)args);

  /*if 3 cars are on the road wait*/
  pthread_mutex_lock(&safelock);
  cars_waiting++;
  pthread_mutex_unlock(&safelock);
  
  sem_wait(&semaphore_west_side);
  
  pthread_mutex_lock(&safelock);
  cars_waiting--;
  cars_on_road++;
  pthread_mutex_unlock(&safelock);
  printf("Car %d: is starting to pass \n",*(int *)args);
  sleep(1);

  pthread_mutex_lock(&safelock);
  cars_on_road--;
  printf("Car %d: Reached to East, Cars on the road:%d\n", *(int *)args, cars_on_road);

  /*if you are the last car make it open for everyone*/
  if (cars_on_road == 0 && cars_waiting==0)
  {
    cars_on_road_way = 2;
    sem_post(&semaphore_side);
    
  }
  pthread_mutex_unlock(&safelock);
  sem_post(&semaphore_west_side);

  
  free(args);
}

void *routine_east(void *args)
{
  
  printf("Car %d: Waiting to pass to West\n", *(int *)args);
  /*if 3 cars are on the road wait*/
  pthread_mutex_lock(&safelock);
  cars_waiting++;
  pthread_mutex_unlock(&safelock);
  
  sem_wait(&semaphore_east_side);
  
  pthread_mutex_lock(&safelock);
  cars_waiting--;
  cars_on_road++;
  pthread_mutex_unlock(&safelock);
  printf("Car %d: is starting to pass \n",*(int *)args);
  sleep(1);

  pthread_mutex_lock(&safelock);
  cars_on_road--;
  printf("Car %d: Reached to West, Cars on the road:%d\n", *(int *)args, cars_on_road);

  /*if you are the last car make it open for everyone*/
  if (cars_on_road == 0 && cars_waiting==0)
  {
    cars_on_road_way = 2;
    sem_post(&semaphore_side);
    
  }
  pthread_mutex_unlock(&safelock);

  sem_post(&semaphore_east_side);

  
  free(args);
}

int main(int arg, char **argv)
{
  /*checking args*/
  if (arg != 3 || strcmp(argv[1], "-c"))
  {
    printf("error in arguments given\n");
    exit(1);
  }
  /*creating mutex*/
  pthread_mutex_init(&safelock, NULL);
  

  /*making the random number*/
  srand(time(NULL));
  int random_int = rand() % 2;
  cars_on_road_way = 2;

  /*creating the thread number*/
  int THREAD_NUM = atoi(argv[2]);
  pthread_t th[THREAD_NUM];

  /*creating the semaphores*/
  sem_init(&semaphore_west_side, 0, 3);
  sem_init(&semaphore_east_side, 0, 3);
  sem_init(&semaphore_side, 0, 1);

  int i;
  for (i = 0; i < THREAD_NUM; i++)
  {
    int *a = malloc(sizeof(int));
    *a = i;

    /*depending on the random number we go east to west or reverse*/
    switch (random_int)
    {
    case 1:

      if (cars_on_road_way == 2 || cars_on_road_way == 0)
      {
        sem_wait(&semaphore_side);
        cars_on_road_way = 1 ;
      }
      
      if (pthread_create(&th[i], NULL, &routine_west, a) != 0)
      {
        perror("Failed to create thread");
      }
      break;

    default:
      if (cars_on_road_way == 2 || cars_on_road_way == 1)
      {
        sem_wait(&semaphore_side);
        cars_on_road_way = 0 ;
      }
      
      if (pthread_create(&th[i], NULL, &routine_east, a) != 0)
      {
        perror("Failed to create thread");
      }
      break;
    }

    random_int = rand() % 2;
  }

  /*error checking for threads*/
  for (i = 0; i < THREAD_NUM; i++)
  {
    if (pthread_join(th[i], NULL) != 0)
    {
      perror("Failed to join thread");
    }
  }

  /*destroying semaphores*/
  sem_destroy(&semaphore_west_side);
  sem_destroy(&semaphore_east_side);
  sem_destroy(&semaphore_side);

  /*destroying mutexs*/
  pthread_mutex_destroy(&safelock);
  
}