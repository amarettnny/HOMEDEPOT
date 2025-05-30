#include <cassert>
#include <ctime>
#include "guard.h"
#include "message.h"
#include "message_queue.h"

MessageQueue::MessageQueue() {
  // initialize the mutex and the semaphore
  pthread_mutex_init(&m_lock, nullptr);
  sem_init(&m_avail, 0, 0);
}

MessageQueue::~MessageQueue() {
  {
    Guard lock(m_lock);
    // for each message left in queue, free the memory 
    for (Message* m : m_messages) {
      delete m;
    }
  }
  // destroy the mutex and the semaphore
  pthread_mutex_destroy(&m_lock);
  sem_destroy(&m_avail);
}

void MessageQueue::enqueue(Message *msg) {
  // block scoped, critical section
  {
    Guard lock(m_lock);
    // put message on queue
    m_messages.push_back(msg);
  }
  // be sure to notify any thread waiting for a message to be
  // available by calling sem_post
  sem_post(&m_avail);
}

Message *MessageQueue::dequeue() {
  struct timespec ts;

  // get the current time using clock_gettime:
  // we don't check the return value because the only reason
  // this call would fail is if we specify a clock that doesn't
  // exist
  clock_gettime(CLOCK_REALTIME, &ts);

  // compute a time one second in the future
  ts.tv_sec += 1;

  // call sem_timedwait to wait up to 1 second for a message
  // to be available, return nullptr if no message is available
  if (sem_timedwait(&m_avail, &ts) == -1) {
        return nullptr;
  }

  // remove the next message from the queue, return it
  Message *msg = nullptr;
  {
    Guard lock(m_lock);
    msg = m_messages.front();
    m_messages.pop_front();
  }  // lock released 

  return msg;
}
