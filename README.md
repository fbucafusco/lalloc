[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://gitlab.com/fbucafusco/lalloc/-/network/master)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/37c027e716c64b5eaa6acad2f3e88186)](https://www.codacy.com?utm_source=gitlab.com&amp;utm_medium=referral&amp;utm_content=fbucafusco/lalloc&amp;utm_campaign=Badge_Grade)

# LALLOC

## Introduction

Lalloc is a C-based library designed for memory allocation management in cases where the number of bytes needed is not known in advance. 

The defines tres basic operations:

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

## Why another memory allocation scheme? 

This library was written originally to support many instances of an UART driver running on a microcontroller.

**Why?**

Some time ago, I wanted a driver that handled all receptions asynchronously at ISR level and split all the frames automatically. 
The protocol I needed to use was an ASCII character delimited one with variable size frames. 
When MANY frames were coming at a certain rate, and depending on the system's processing speed, some frames were lost.

One way to achieve this, would have been using a ping pong buffer, or a memory block strategy, but I wanted to avoid using memory inefficiently.

Then, I implemented LALLOC to handle that problem.

The main idea behind this was: 
- To have some mechanism to offer to the driver that maximizes the available memory to store the incoming message. 
- Initially the driver was in IDLE, but when a byte first incomes, the driver requested a memory pool to store the frame. 
- When the frame is complete, the pool of memory was "closed".

All the handling of the data was sent to another abstraction layer that implements the framing logic. Is not included in LALLOC (see packet_framer)

