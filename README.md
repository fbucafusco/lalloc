[![CircleCI](https://dl.circleci.com/status-badge/img/circleci/ApgampwqiLTRb6gFADDgxz/HfgnnTMzZ5UqKMDNqhoTsd/tree/master.svg?style=svg&circle-token=374ece89186b0dcb3daa6434ace0aae24cc861fe)](https://dl.circleci.com/status-badge/redirect/circleci/ApgampwqiLTRb6gFADDgxz/HfgnnTMzZ5UqKMDNqhoTsd/tree/master)

# LALLOC

## Introduction

Lalloc is a C-based library designed for memory allocation management in cases where the number of bytes needed is not known in advance. 

The defines tree basic operations:

- alloc: provides an area with the max amount of unfragmented and contiguous memory from the pool. (the allocation started)
- commit: the user confirms the use of a given number of bytes from it. (the allocation is done)
- free: frees (the allocation ends)

## Features

  - Written in C
  - Static or Dynamic allocation of LALLOC instances.
  - Coalescense algorithm to join freed nodes.
  - Thread safe (the API can be used in a foreground-background architecture or between tasks from a preemptive OS ).
  - User configurable at compile time.

## Basics

For a given instance of lalloc, you can execute calls to alloc, **commit** and **free**. The instace will have a given pools size that will define the memory area used by the user. That size will be used to store nodes for each block the user will allocate.

An **alloc** operation followed by a **commit**, is equivalent for a regular malloc-like call.

When used asynchronous (defered call to **commit**) the first **alloc** call will block the lalloc instance, meaning that the user won't be able to call **alloc** again until either **commit** or **revert** is called.


## Usage

   - Within any scope: allocate an object with a given RAM size. This call is a macro, because it instances more than one object in memory.

```
LALLOC_DECLARE( objname , pool_size  ); 
```

   - Producer

```
char* pool;
int size;
lalloc_init( objname );
lalloc_alloc( objname , &pool , &size );
//the user must fill the pool with data, up to 'size' bytes synch or async. 
//Asume the user writes 10 bytes.
lalloc_commit( objname , 10  ); 
```
   - Consumer
```
lalloc_get_n ( objname , &pool, &size, 0 ); //retrieve the oldest element
//use the data
lalloc_free_dest ( objname , pool );        //frees the pool. 
```

## Rationale for a New Memory Allocation Scheme

LALLOC was developed to address the specific needs of managing multiple UART (Universal Asynchronous Receiver/Transmitter) driver instances on a microcontroller. The main goals were:

* Efficiently manage memory for incoming messages.
* Avoid data copying to maintain performance and efficiency.

### Background

I needed a driver capable of handling receptions autonomously at the ISR (Interrupt Service Routine) level while effectively segregating incoming frames. The protocol used ASCII character delimitation with frames of varying sizes. High frame rates led to system processing limitations, causing frame losses.

Traditional solutions like ping-pong buffers or memory block allocators were insufficient for ensuring efficient memory usage.

### Solution

LALLOC was designed to:

* Optimize memory usage for storing incoming messages.
* Enable the driver to request memory upon receiving the first byte and remain in an IDLE state initially.
* Commit the memory segment once the frame is fully received.
* Manage incoming data frames with an internal linked list functioning as a FIFO (First In, First Out) queue, allowing data processing in the order of arrival.

Unlike traditional methods, such as using malloc, memory blocks, or ping-pong buffers, LALLOC eliminates the need to know the maximum packet size in advance. In these traditional methods, the driver must allocate memory for the worst-case scenario, which can lead to inefficiencies. For instance, with memory blocks, if you want to store N incoming packets, you need a pool of at least N times the maximum packet size, even if the actual packets are much smaller.

Furthermore, LALLOC includes a built-in FIFO algorithm, whereas other methods would require an additional implementation to manage the order of packet processing. This integration simplifies the design and enhances the efficiency of the memory allocation process.

## License

This project is licensed under the terms of the BSD 3-Clause License. For more details, please see the [LICENSE](./LICENSE) file.

