#include <stdio.h>
#include <pthread.h>

#include <vector>

const int PRODUCT_SIZE = 100;
const int PRODUCTION_TIMES = 2;

std::vector<int> product;
pthread_mutex_t product_mutex;

void *consumption(void *args){
  for(int i = 0; i < PRODUCTION_TIMES; i++){
    pthread_mutex_lock(&product_mutex);
    for(int j = 0; j < PRODUCT_SIZE; j++){
      printf("Consuming: %d from production: %d\n",i,product[j]);
    }
    product.clear();
    pthread_mutex_unlock(&product_mutex);
  }
}

void *production(void *args){
  for(int i = 0; i < PRODUCTION_TIMES; i++){
    pthread_mutex_lock(&product_mutex);
    for(int j = 0; j < PRODUCT_SIZE; j++){
      product.push_back(j);
      printf("Production turn: %d produced: %d\n",i,product[j]);
    }
    pthread_mutex_unlock(&product_mutex);
  }
}

int main(void){
  pthread_t consumer, producer;
  pthread_mutex_init(&product_mutex,NULL);
  pthread_create(&producer,NULL,production,NULL);
  pthread_create(&consumer,NULL,consumption,NULL);
  pthread_join(producer,NULL);
  pthread_join(consumer,NULL);
  pthread_mutex_destroy(&product_mutex);
  return 0;
}
