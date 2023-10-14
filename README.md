[![CircleCI](https://dl.circleci.com/status-badge/img/circleci/ApgampwqiLTRb6gFADDgxz/HfgnnTMzZ5UqKMDNqhoTsd/tree/master.svg?style=svg&circle-token=374ece89186b0dcb3daa6434ace0aae24cc861fe)](https://dl.circleci.com/status-badge/redirect/circleci/ApgampwqiLTRb6gFADDgxz/HfgnnTMzZ5UqKMDNqhoTsd/tree/master)

# LALLOC

## Introduction

Lalloc is a C-based library designed for memory allocation management in cases where the number of bytes needed is not known in advance. 

The defines tree basic operations:

- alloc: gives to the user the max amount of unfragmented memory from the pool. (the allocation is started)
- commit: the user confirms the use of a given number of bytes from it. (the allocation is done)
- free: frees (the allocation ends)

It allows to add, peek, and delete nodes from a list of variable-sized elements.

## Features

  - Written in C
  - Static or Dynamic allocation of LALLOC instances.
  - Coalescense algorithm to join freed nodes.
  - Thread safe (the API can be used in a foreground-background architecture or between tasks from a preemptive OS ).
  - User configurable at compile time.

## Usage

   - Within any scope: allocate an object with a given RAM size

```
LALLOC_DECLARE( objname , pool_size , 0 ); 
```

   - Within user aplicaction (fill with data)

```
char* pool;
int size;
lalloc_init( objname );
lalloc_alloc( objname , &pool , &size );
//the user must fill the pool with data, up to 'size' bytes. Asume you write 10 bytes.
lalloc_commit( objname , 10  ); 
```
   - Within user aplicaction (get data)
```
lalloc_get_n ( objname , &pool, &size, 0 ); //retrieve the oldest element.
//use the data
lalloc_free_dest ( objname , pool );        //frees the pool. 
```

## Rationale for a New Memory Allocation Scheme

This library emerged from a necessity to facilitate numerous instances of a UART (Universal Asynchronous Receiver/Transmitter) driver operating on a microcontroller.

Previously, I wanted a driver capable of managing receptions autonomously at the ISR (Interrupt Service Routine) level while also segregating the incoming frames effectively. 
The protocol I was utilizing relied on ASCII character delimitation with frames of varying sizes. 
When a substantial number of frames were being received at a particular rate, the system's processing capability became a limiting factor, resulting in loss of some frames.

While solutions like employing a ping-pong buffer strategy or a memory block allocator could have addressed this issue, my priority was to ensure memory was used efficiently.

Consequently, LALLOC was developed to tackle this challenge.

The central premise was to:: 
- Establish a mechanism that would enable the driver to optimize the available memory for storing incoming messages. 
- Initially, the driver would remain in an IDLE state, but upon receipt of the first byte, it would request a memory segment from a LALLOC instance to store the frame. 
- Once the frame was fully received, the allocated memory segment would be "committed".
- Additionally, given its role in managing incoming data frames, LALLOC incorporated an internal linked list functioning as a FIFO (First In, First Out) queue, enabling the data consumer to process frames in the order of their arrival.

The data handling aspect was relegated to another abstraction layer, which encapsulated the framing logic, separate from the LALLOC implementation.