#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include <vector>

const int READ_WRITE_TIMES = 2;
const int CONTENT = 2;
const int WRITERS = 2;
const int READERS = 5;

struct Content{
  int author, content;
  Content(){}
  Content(int _author,int _content){
    author = _author, content = _content;
  }
};

std::vector<Content> content;
pthread_cond_t waiting_writer = PTHREAD_COND_INITIALIZER;

bool finish_writers = false;
int active_readers = 0, done_readers = 0;
pthread_mutex_t active_readers_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t waiting_content = PTHREAD_COND_INITIALIZER;
void *reading_procedure(void *args){
  int reader_id = *((int *)args);
  for(int t = 0; t < READ_WRITE_TIMES; t++){
    usleep(1000 * (random() % READERS + WRITERS));
    pthread_mutex_lock(&active_readers_mutex);
    while(!content.size()){
      pthread_cond_wait(&waiting_content,&active_readers_mutex);
    }
    active_readers++;
    pthread_mutex_unlock(&active_readers_mutex);
    for(int i = 0; i < (int)content.size(); i++){
      printf("Turn: %d\n",t);
      printf("Who read: %d\n",reader_id);
      printf("Value read: %d\n",content[i].content);
      printf("Number of readers: %d\n\n",active_readers);
    }
    pthread_mutex_lock(&active_readers_mutex);
    active_readers--;
    if(!active_readers){
      pthread_cond_signal(&waiting_writer);
    }
    if(t+1 == READ_WRITE_TIMES){
      done_readers++;
      //maybe there are still some writers waiting.
      if(done_readers == READERS){
        finish_writers = true;
        pthread_cond_broadcast(&waiting_writer);
        printf("Broadcast to finish writers.\n");
      }
    }
    pthread_mutex_unlock(&active_readers_mutex);
  }
}

void *writing_procedure(void *args){
  int writer_id = *((int *)args);
  for(int t = 0; t < READ_WRITE_TIMES; t++){
    usleep(1000 * (random() % READERS + WRITERS));
    pthread_mutex_lock(&active_readers_mutex);
    if(content.size() == CONTENT && !finish_writers){
      pthread_cond_wait(&waiting_writer,&active_readers_mutex);
    }
    content.clear();
    for(int i = 0; i < CONTENT; i++){
      content.push_back(Content(writer_id,i));
      printf("Turn: %d\n",t);
      printf("Who wrote: %d\n",writer_id);
      printf("Written value: %d\n",i);
      printf("Number of readers: %d\n\n",active_readers);
    }
    pthread_mutex_unlock(&active_readers_mutex);
    pthread_cond_broadcast(&waiting_content);
  }
}

struct Writer{
  int id;
  pthread_t thread;
}writers[WRITERS];

struct Reader{
  int id;
  pthread_t thread;
}readers[WRITERS];

int main(void){
  srandom((unsigned int)time(NULL));
  for(int i = 0; i < READERS; i++){
    readers[i].id = i;
    pthread_create(&readers[i].thread,NULL,reading_procedure,&readers[i].id);
  }
  for(int i = 0; i < WRITERS; i++){
    writers[i].id = i;
    pthread_create(&writers[i].thread,NULL,writing_procedure,&writers[i].id);
  }
  for(int i = 0; i < WRITERS; i++){
    if(!pthread_join(writers[i].thread,NULL)){
      printf("Writer %d is done.\n",i);
    }
  }
  for(int i = 0; i < READERS; i++){
    if(!pthread_join(readers[i].thread,NULL)){
      printf("Reader %d is done.\n",i);
    }
  }
  return 0;
}