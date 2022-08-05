#ifndef __IDE_H_
#define __IDE_H_

#define BLOCK_SIZE   512

/* Disk is an array of blocks (512Bytes):
 *       
 *       _0___1___2__________________N_
 *      |   |   |   |     ...      |   |
 *      |___|___|___|______________|___|
 *      \___/
 *        |
 *       512 bytes
 *
 */

/* ideinit - function to initialize disk. Must be called once before any other disk operation.
 * Returns -1 if no disk is found */
int ideinit(void);

/* idewrite - Write 512bytes from memory at 'b' to block number 'bloc'.
 * Returns -1 in case of error */
int idewrite(char* b,  unsigned bloc);

/* ideread - Read 512bytes from block number 'bloc' to memory at 'b'.
 * Returns -1 in case of error */
int ideread (char* b,  unsigned bloc);

#endif /* __IDE_H_ */
