/* CS361 Homework 6: Concurrent Elevator Controller
 * Name:   Shyam Patel
 * NetID:  spate54
 * Date:   Nov 28, 2018
 */

#include "hw6.h"
#include <stdio.h>
#include <pthread.h>


struct Elevator {                 // elevator :
    enum {                        //   status
        ELEVATOR_ARRIVED, ELEVATOR_OPEN, ELEVATOR_CLOSED
    } state;
    int current_floor;            //   curr floor
    int from_floor;               //   src  floor
    int to_floor;                 //   dest floor
    int direction;                //   direction
    int occupancy;                //   occupancy
    pthread_mutex_t   e_lock;     //   elevator  lock
    pthread_mutex_t   p_lock;     //   passenger lock
    pthread_barrier_t e_barrier;  //   elevator  barrier
    pthread_barrier_t p_barrier;  //   passenger barrier
} elevators[ELEVATORS];


// initialize scheduler
void scheduler_init() {
    int i;
    for (i = 0; i < ELEVATORS; i++) {                         // initialize :
        elevators[i].state         = ELEVATOR_ARRIVED;        //   status
        elevators[i].current_floor = 0;                       //   curr floor
        elevators[i].from_floor    = -1;                      //   src  floor
        elevators[i].to_floor      = -1;                      //   dest floor
        elevators[i].direction     = -1;                      //   direction
        elevators[i].occupancy     = 0;                       //   occupancy
        pthread_mutex_init(&elevators[i].e_lock, 0);          //   e lock
        pthread_mutex_init(&elevators[i].p_lock, 0);          //   p lock
        pthread_barrier_init(&elevators[i].e_barrier, 0, 2);  //   e barrier
        pthread_barrier_init(&elevators[i].p_barrier, 0, 2);  //   p barrier
    }
}//end scheduler_init()


// passenger request operation
void passenger_request(int passenger, int from_floor, int to_floor,
                       void (*enter)(int, int), void (*exit)(int, int)) {
    int p  = passenger;                             // passenger p
    int e  = p % ELEVATORS;                         // elevator  e (assign)
    int ff = from_floor;                            // p's  floor ff
    int tf = to_floor;                              // dest floor tf

    pthread_mutex_lock(&elevators[e].p_lock);       // lock p

    elevators[e].from_floor = ff;                   // set e's src  floor
    elevators[e].to_floor   = tf;                   // set e's dest floor

    pthread_barrier_wait(&elevators[e].p_barrier);  // (e arrives on p's floor)

    if (elevators[e].state         == ELEVATOR_OPEN &&
        elevators[e].current_floor == ff            &&
        elevators[e].occupancy     == 0) {
        log(3, "Passenger %d entering elevator %d on floor %d\n", p, e, ff);
        enter(p, e);                                // p enters e
        elevators[e].occupancy++;                   // increment e's occupancy
    }

    pthread_barrier_wait(&elevators[e].e_barrier);  // (e carries p to dest)
    pthread_barrier_wait(&elevators[e].p_barrier);  // (e arrives on dest floor)

    if (elevators[e].state         == ELEVATOR_OPEN &&
        elevators[e].current_floor == tf            &&
        elevators[e].occupancy     == 1) {
        log(3, "Passenger %d exiting elevator %d on floor %d\n", p, e, tf);
        exit(p, e);                                 // p exits e
        elevators[e].from_floor = -1;               // clear e's src  floor
        elevators[e].to_floor   = -1;               // clear e's dest floor
        elevators[e].occupancy--;                   // decrement e's occupancy
    }

    pthread_barrier_wait(&elevators[e].e_barrier);  // (e ready for next p)
    pthread_mutex_unlock(&elevators[e].p_lock);     // unlock p
}//end passenger_request()


// elevator ready operation
void elevator_ready(int elevator, int at_floor, void (*move_direction)(int, int),
                    void (*door_open)(int), void (*door_close)(int)) {
    int e = elevator;                                       // elevator e

    pthread_mutex_lock(&elevators[e].e_lock);               // lock e

    int cf  = at_floor;                                     // e's curr floor
    int ff  = elevators[e].from_floor;                      // e's src  floor
    int tf  = elevators[e].to_floor;                        // e's dest floor
    int dir = elevators[e].direction;                       // e's direction
    int occ = elevators[e].occupancy;                       // e's occupancy

    // e arrived...
    if (elevators[e].state == ELEVATOR_ARRIVED) {
        // ...on p's floor (empty) or on dest floor (with p)
        if ((occ == 0 && cf == ff) || (occ == 1 && cf == tf)) {
            door_open(e);                                   // e opens doors
            elevators[e].state = ELEVATOR_OPEN;             // set e's status
            pthread_barrier_wait(&elevators[e].p_barrier);  // (e ready for p)
            pthread_barrier_wait(&elevators[e].e_barrier);  // (p ready for e)
        }
        // ...on other floor
        else
            elevators[e].state = ELEVATOR_CLOSED;           // set e's status
    }

    // e's doors are open
    else if (elevators[e].state == ELEVATOR_OPEN) {
        door_close(e);                                      // e closes doors
        elevators[e].state = ELEVATOR_CLOSED;               // set e's status
    }

    // redirect and/or move e to dest
    else {
        // (1) e on edge floor, or
        // (2) e headed up   (empty)  + src  floor is below, or
        // (3) e headed up   (with p) + dest floor is below, or
        // (4) e headed down (empty)  + src  floor is above, or
        // (5) e headed down (with p) + dest floor is above
        if ((cf  == 0) || (cf == FLOORS - 1)   ||           // (1),
            (dir == 1  && occ == 0 && ff < cf) ||           // (2),
            (dir == 1  && occ == 1 && tf < cf) ||           // (3),
            (dir == -1 && occ == 0 && cf < ff) ||           // (4),
            (dir == -1 && occ == 1 && cf < tf))             // (5) :
            elevators[e].direction *= -1;                   //   redirect e

        move_direction(e, elevators[e].direction);          // move e to dest
        elevators[e].current_floor = cf + elevators[e].direction;
        elevators[e].state         = ELEVATOR_ARRIVED;      // reset e's status
    }

    pthread_mutex_unlock(&elevators[e].e_lock);             // unlock e
}//end elevator_ready()

