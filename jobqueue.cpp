/*
 * jobqueue.cpp
 *
 *  Created on: 22 sep 2013
 *      Author: pontus
 */

#include <Arduino.h>

#include "jobqueue.hpp"
#include "leds.hpp"

static char r_ptr;
static char w_ptr;
static uint8_t cnt;

static job_entry_t queue[JOB_QUEUE_SIZE];

//*****************************************************************************
//*
//*****************************************************************************
void init_queue(void)
{
  r_ptr = 0;
  w_ptr = 0;
  cnt = 0;
}

//*****************************************************************************
//*
//*****************************************************************************
char queue_put(job_entry_t *job)
{
  if ((job->job == 0) || (job->job >= JOB_LAST_JOB_MARKER))
    return JOB_QUE_UNKNOWN_JOB;
  if (cnt == JOB_QUEUE_SIZE)
    return JOB_QUE_FULL;

  queue[w_ptr] = *job;

  w_ptr = (w_ptr + 1) % JOB_QUEUE_SIZE;
  cnt++;
}

//*****************************************************************************
//*
//*****************************************************************************
char queue_get(job_entry_t *job)
{
  if (!cnt)
    return JOB_QUE_EMPTY;

  *job = queue[r_ptr];

  r_ptr--;
  if (r_ptr < 0)
    // Wrap around buffer if required
    r_ptr = JOB_QUEUE_SIZE + r_ptr;
  cnt--;

  return JOB_QUE_OK;
}

//*****************************************************************************
//*
//*****************************************************************************
char process_jobs (void)
{
  job_entry_t job;

  while (queue_get(&job) == JOB_QUE_OK) {
    switch (job.job) {
      case JOB_SET_LED:
        setled (job.data, 1);
        break;

      case JOB_CLR_LED:
        setled (job.data, 0);
        break;
    }
  }
}
