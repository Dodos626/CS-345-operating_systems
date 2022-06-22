#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>



sem_t cars_sem;
sem_t ferry_sem;

pthread_mutex_t safelock;

int total_cars = 0;
int cars_on_ferry = 0;
int cars_waiting = 0;

int car_names[20];
int car_names_index = 0;

void *cars_routine(void *args)
{
  // random time of waiting before arriving at the dock
  total_cars++;
  
  int x = rand() % 10;
  sleep(x);
  pthread_mutex_lock(&safelock);
  cars_waiting++;
  pthread_mutex_unlock(&safelock);
  printf("Car %d, waiting to board to the ferry\n", *(int *)args);
  

  // checks if there is space on board
  sem_wait(&cars_sem);
  
  

  pthread_mutex_lock(&safelock);
  cars_waiting--;
  cars_on_ferry++;
  car_names[car_names_index]=(int)(*(int *)args);
  car_names_index++;
  // if ship got full or if there are no more cars start the ship
  if (cars_on_ferry == 20 || total_cars - cars_on_ferry == 0 || cars_waiting == 0)
  {
    
    
    sem_post(&ferry_sem);

    for (size_t i = 0; i < 20 - cars_on_ferry; i++)
    {
      sem_wait(&cars_sem);
    }
    
    
  }
  printf("\033[0;32mCar %d, is boarding to the ferry\n\033[0m", *(int *)args);

  pthread_mutex_unlock(&safelock);
}

void *ferry_routine(void *args)
{
  while (1)
  {

    printf("\033[0;31mThe ferry is waiting\n\033[0m");
    for (size_t i = 0; i < 20; i++)
    {
      sem_post(&cars_sem);
    }

    // wait to be woken up
    sem_wait(&ferry_sem);
    
    printf("\033[0;31mThe ferry finished to board cars\n\033[0m");
    sleep(1);
    printf("\033[0;31mThe ferry started the trip\n\033[0m");
    sleep(1);
    printf("\033[0;31mThe ferry reached it's destination with %d cars on board\n\033[0m", cars_on_ferry);
    pthread_mutex_lock(&safelock);
    for (size_t i = 0; i < car_names_index; i++)
    {
      printf("Car %d, reached the destination and is leaving the ship\n",car_names[i]);
      car_names[car_names_index]=0;
    }
    car_names_index=0;
    pthread_mutex_unlock(&safelock);
    sleep(1);
    printf("\033[0;31mThe ferry is returing\n\033[0m");
    pthread_mutex_lock(&safelock);

    // Consume
    total_cars = total_cars - cars_on_ferry;
    cars_on_ferry = 0;

    pthread_mutex_unlock(&safelock);

    // no more cars to take
    if (total_cars == 0)
    {
      break;
    }
  }
}

int main(int argc, char *argv[])
{
  /*checking args*/
  if (argc != 3 || strcmp(argv[1], "-c"))
  {
    printf("error in arguments given\n");
    exit(1);
  }
  /*creating the thread number*/
  int THREAD_NUM = atoi(argv[2]);

  srand(time(NULL));
  pthread_t th[THREAD_NUM];
  pthread_mutex_init(&safelock, NULL);
  sem_init(&cars_sem, 0, 0);
  sem_init(&ferry_sem, 0, 0);
  int i;

  for (i = 0; i < THREAD_NUM+1; i++)
  {
    int *a = malloc(sizeof(int));
    *a = i-1;
    if (i == 0)
    {
      if (pthread_create(&th[i], NULL, &ferry_routine, NULL) != 0)
      {
        perror("Failed to create thread");
      }
    }else if (pthread_create(&th[i], NULL, &cars_routine, a) != 0)
    {
      perror("Failed to create thread");
    }
  }

  for (i = 0; i < THREAD_NUM+1; i++)
  {
    if (pthread_join(th[i], NULL) != 0)
    {
      perror("Failed to join thread");
    }
  }
  sem_destroy(&cars_sem);
  sem_destroy(&ferry_sem);
  pthread_mutex_destroy(&safelock);
  return 0;
}