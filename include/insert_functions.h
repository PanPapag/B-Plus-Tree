#pragma once

/* Includes */
#include "AM.h"
#include "bf.h"
#include "defn.h"

#include "./util.h"

/* Returns a string which contains exactly the info we want to append
  to the higher level block. Also, it returns the allocated blocks's number
  by passing its value to the new_block_num pointer argument */
char* split_data_block(int fileDesc, int block_num, Record* new_record,
                       int* new_block_num);
/* Crates the root of B+ tree and returns its block index */
int create_root(int fileDesc, void* key);

boolean data_sorted_insert(int block_num, int fileDesc, Record new_record);
/* Returns the index of the data block to potentially insert our record */
int find_data_block_to_insert(int fileDesc, int root_num, void *key);
