[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://gitlab.com/fbucafusco/lalloc/-/network/master)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/37c027e716c64b5eaa6acad2f3e88186)](https://www.codacy.com?utm_source=gitlab.com&amp;utm_medium=referral&amp;utm_content=fbucafusco/lalloc&amp;utm_campaign=Badge_Grade)
# LALLOC

An ADT that handles a pool of memory as a variable-sized element double linked list. The allocation scheme is not like malloc, where you define the amount of memory the block will have, but it is based on a dual process architecture. The first action is to allocate a block with the maximum size possible. Then, when the user has filled that block, the user must commit that information, specifying the used storage. 

It allows to add, peek, and delete nodes from a list of variable-sized elements without using any other dynamic allocation routines.

## Features

  - Written in C
  - Static or Dynamic allocation of LALLOC instances.
  - Coalescense algorithm to join freed nodes.
  - Thread safe (the API can be used in a foreback-background architecture or between tasks from a preemptive OS ).
  - User configurable at compile time.

## Usage

   - Within any scope:

```
LALLOC_DECLARE( objname , pool_size , 0 ); 
```

This will allocate an object with a given RAM size (fixed by the processor alignment)

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

## Why another memory allocation scheme? A little story.

Originally it was written to implement many instances of an UART driver within a microcontroller.

**Why?**

Some time ago, I wanted a driver that handles all receptions asynchronously at ISR level and split all the frames automatically. 
The protocol I needed to use was an ASCII character delimited one with variable size frames. 
When MANY frames were coming at a certain rate, and depending on the system's processing speed, some frames were lost.

I didn't want a malloc/free to do this to avoid fragmentation in any way. Then, I could have used the N buffering strategy, but to avoid using memory inefficiently (I mean, reserving N bytes for each buffer), I thought of another solution.

Then, I implemented LALLOC to handle that problem.

The main idea behind this was: 
- To have some mechanism to offer to the driver that maximizes the available memory to store the incoming message. 
- Initially the driver was in IDLE, but when a byte first incomes, the driver requested a memory pool to store the frame. 
- When the frame is complete, the pool of memory is "closed".

All the handling of the data was sent to another abstraction layer that implements the framing logic. Is not included in LALLOC (see packet_framer)

