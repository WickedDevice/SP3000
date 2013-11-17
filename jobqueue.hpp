/*
 * jobqueue.hpp
 *
 *  Created on: 22 sep 2013
 *      Author: pontus
 */

#ifndef JOBQUEUE_HPP_
#define JOBQUEUE_HPP_

#include <stdint.h>
/**
 * List of available jobs in the system.
 */
enum available_jobs {
  JOB_DONT_USE_0 = 0,
  JOB_SET_LED,
  JOB_CLR_LED,
  JOB_DELAYED_CLR,
  JOB_LAST_JOB_MARKER
};
/**
 * Error and status codes that the queue manager can return
 */
#define JOB_QUE_OK           0
#define JOB_QUE_FULL        -1
#define JOB_QUE_EMPTY       -2
#define JOB_QUE_PROC_AGAIN  -3
#define JOB_QUE_UNKNOWN_JOB -4

/**
 * The number of entries in the job queue
 */
#define JOB_QUEUE_SIZE       8

/**
 * This structure describes a queue job
 */
struct job_entry {
    /*@{*/
    uint8_t job;  /**< The job that is to be performed */
    uint8_t data; /**< The data associated with the job */
    /*}@*/
};
typedef struct job_entry job_entry_t;

//*****************************************************************************
/*!
 * Initializes the queue manager.
 */
//*****************************************************************************
void init_queue(void);

//*****************************************************************************
/*!
 * Puts a job entry into the job queue. The job is appended to the end of the
 * queue. If the job queue is full an error will be returned.
 *
 * @param[in] job     A pointer to a job_entry_t which holds the job data.
 *
 * @return            JOB_QUE_OK if the job was successfully placed in the queue
 *                    or JOB_QUE_FULL of the queue was full.
 */
//*****************************************************************************
char queue_put(job_entry_t *job);

//*****************************************************************************
/*!
 * Gets a job from the job queue. The function must be called with a pointer
 * to where the job data shall be placed. If the job queue is empty an error
 * will be returned.
 *
 * @param[out]  job   A pointer to a job_entry_t where data shall be placed
 *
 * @return            JOB_QUE_OK if a job was successfully copied,
 *                    or JOB_QUE_EMPTY if there was no data in the queue
 */
//*****************************************************************************
char queue_get(job_entry_t *job);

//*****************************************************************************
/*!
 * Processes the job queue. Will return JOB_QUE_OK if everything was okay
 * and all jobs were processes this iteration. The function can also return
 * JOB_PROC_PROC_AGAIN which means that the queue process manager made the
 * decision that the job processing took to long. The function can if needed be
 * called again directly or do some processing of its own before attempting to
 * process the jobs again.
 *
 * @return            JOB_QUE_OK if processing successful,
 *                    or JOB_PROC_PROC_AGAIN if there were more jobs left to do.
 */
//*****************************************************************************
char process_jobs (void);

#endif /* JOBQUEUE_HPP_ */
