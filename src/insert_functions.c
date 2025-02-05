#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/insert_functions.h"
#include "../include/scan_funcs.h"

/** Get the key and the value, and create the struct that we are going to use
	  as a record in our file */
Record* create_record(int fileDesc, void* key, void* value) {
	BF_Block* first_block;
	BF_Block_Init(&first_block);
	/* And get access to it */
	BF_GetBlock(fileDesc, 0, first_block);
	/* Get access to its data */
	char* first_block_info = BF_Block_GetData(first_block);
	/* Store the info of the key and the value */
	char attrType1, attrType2;
	int attrLength1, attrLength2;
	int offset = sizeof(char) + 4 * sizeof(int);
	memcpy(&attrType1, first_block_info + offset, sizeof(char));
	offset += sizeof(char);
	memcpy(&attrLength1, first_block_info + offset, sizeof(int));
	offset += sizeof(int);
	memcpy(&attrType2, first_block_info + offset, sizeof(char));
	offset += sizeof(char);
	memcpy(&attrLength2, first_block_info + offset, sizeof(int));
	/* Create a new record, and store the desired info */
	Record* rec = malloc(sizeof(Record));
	/* Allocate space for the key and the value, and copy the data there */
	rec->key = malloc(attrLength1);
	memcpy(rec->key, key, attrLength1);
	rec->value = malloc(attrLength2);
	memcpy(rec->value, value, attrLength2);
  /* The size we are going to use in allocations, is the size of the pointer +
     the size of the key and the value */
  rec->size = sizeof(Record) + attrLength1 + attrLength2;
  /* Return the record */
	return rec;
}

/* Create a root as a result of a previous root split */
int create_root(int fileDesc, char* append, int key_size) {
	/* Initialize pointers to the first block, and the block we want as a root */
  BF_Block *first_block, *root_block;
  int offset;
  /* Get data of 1st block */
  BF_Block_Init(&first_block);
  BF_GetBlock(fileDesc, 0, first_block);
  char *first_block_info = BF_Block_GetData(first_block);
  int root_block_index;
  int blocks_num;
  BF_Block_Init(&root_block);
	/* Determine where our new root is going to be */
  BF_AllocateBlock(fileDesc, root_block);
  BF_GetBlockCounter(fileDesc,  &blocks_num);
  root_block_index = blocks_num - 1;
	/* Get the data of the new B+ tree's root */
  char* root_block_info = BF_Block_GetData(root_block);
  /* Place 'I' and number of indexes in the block */
  char I = 'I';
  offset = 0;
  memcpy(root_block_info + offset, &I, sizeof(char));
	/* OUr indexes will be one, as we are creating a new root */
  int no_indxs = 1;
  offset = sizeof(char);
  memcpy(root_block_info + offset, &no_indxs, sizeof(int));
  /* Place the key and 2 block indexes that we get as an argument */
  offset = sizeof(char) + sizeof(int);
  memcpy(root_block_info + offset, append, key_size + 2 * sizeof(int));
  /* We place the index block of root in the first block */
  offset = sizeof(char) + sizeof(int);
  memcpy(first_block_info + offset, &root_block_index, sizeof(int));
  /* Set dirty, Unpin, Destroy*/
  BF_Block_SetDirty(first_block);
  BF_Block_SetDirty(root_block);
  BF_UnpinBlock(first_block);
  BF_UnpinBlock(root_block);
  BF_Block_Destroy(&root_block);
  BF_Block_Destroy(&first_block);
	/* Finally, return where our root is gonna be from here on now */
  return root_block_index;
}

/** Create an empty root, for the first time that we want to insert something in
		our tree */
int create_empty_root(int fileDesc,  void *key, int keyLength) {
  BF_Block *first_block, *root_block, *first_data_block, *second_data_block;
  int offset;
  /* Get data of 1st block */
  BF_Block_Init(&first_block);
  CALL_BF(BF_GetBlock(fileDesc, 0, first_block));
  char *first_block_info = BF_Block_GetData(first_block);
  /* Type of key and the length */
  char keyType;
  offset = sizeof(char) + 4 * sizeof(int);
  memcpy(&keyType, first_block_info + offset, sizeof(char));
  offset = 2 * sizeof(char) + 4 * sizeof(int);
  /* key to a new variable */
  /* Create root block */
  int root_block_index = -1;
  int blocks_num;
  BF_Block_Init(&root_block);

  CALL_BF(BF_AllocateBlock(fileDesc, root_block));
  CALL_BF(BF_GetBlockCounter(fileDesc,  &blocks_num));
  root_block_index = blocks_num - 1;

  char* root_block_info = BF_Block_GetData(root_block);
  /* Place 'I' and number of indexes in the block */
  char I = 'I';
  offset = 0;
  memcpy(root_block_info + offset, &I, sizeof(char) );
  int no_indxs = 1;
  offset = sizeof(char);
  memcpy(root_block_info + offset, &no_indxs, sizeof(int));
  /* Place the key and 2 data block indexes later*/

  /* Create 2 data blocks */
  BF_Block_Init(&first_data_block);
  BF_Block_Init(&second_data_block);
  /* Allocate blocks */
  CALL_BF(BF_AllocateBlock(fileDesc, first_data_block));
	CALL_BF(BF_GetBlockCounter(fileDesc,  &blocks_num));
  int first_data_block_index = blocks_num - 1;
  CALL_BF(BF_AllocateBlock(fileDesc, second_data_block));
	CALL_BF(BF_GetBlockCounter(fileDesc,  &blocks_num));
  int second_data_block_index = blocks_num - 1;
	BF_GetBlock(fileDesc, second_data_block_index, second_data_block);
  char* first_data_block_info = BF_Block_GetData(first_data_block);
  char* second_data_block_info = BF_Block_GetData(second_data_block);
  /* Place 'D' and number of records in the block */
  char D = 'D';
  offset = 0;
  memcpy(first_data_block_info + offset, &D, sizeof(char));
  memcpy(second_data_block_info + offset, &D, sizeof(char));
  int no_records = 0;
  offset = sizeof(char);
  memcpy(first_data_block_info + offset, &no_records, sizeof(int));
  memcpy(second_data_block_info + offset, &no_records, sizeof(int));
  /* we want to keep all the data blocks linked, so store the previous and the
     next block number */
  /* for the first block */
  int prev = -1; int next = second_data_block_index;
  offset = sizeof(char) + sizeof(int);
  memcpy(first_data_block_info + offset, &prev, sizeof(int));
  offset = sizeof(char) + 2 * sizeof(int);
  memcpy(first_data_block_info + offset, &next, sizeof(int));
  /* for the second block */
  prev = first_data_block_index; next = -1;
  offset = sizeof(char) + sizeof(int);
  memcpy(second_data_block_info + offset, &prev, sizeof(int));
  offset = sizeof(char) + 2 * sizeof(int);
  memcpy(second_data_block_info + offset, &next, sizeof(int));

  /* Now place the 2 data block indexes in the root block,
    and the key between them */
  offset = sizeof(char) + sizeof(int);
  memcpy(root_block_info + offset, &first_data_block_index, sizeof(int));
  offset = sizeof(char) + 2 * sizeof(int);
  memcpy(root_block_info + offset, key, keyLength);
  offset = sizeof(char) + 2 * sizeof(int) + keyLength;
  memcpy(root_block_info + offset, &second_data_block_index, sizeof(int));

  /* We place the index block of root in the first block */
  offset = sizeof(char) + sizeof(int);
  memcpy(first_block_info + offset, &root_block_index, sizeof(int));

  /* Set dirty, Unpin, Destroy*/
  BF_Block_SetDirty(first_block);
  BF_Block_SetDirty(root_block);
  BF_Block_SetDirty(first_data_block);
  BF_Block_SetDirty(second_data_block);
  CALL_BF(BF_UnpinBlock(first_block));
  CALL_BF(BF_UnpinBlock(root_block));
  CALL_BF(BF_UnpinBlock(first_data_block));
  CALL_BF(BF_UnpinBlock(second_data_block));
  BF_Block_Destroy(&root_block);
  BF_Block_Destroy(&first_block);
  BF_Block_Destroy(&second_data_block);
  BF_Block_Destroy(&first_data_block);
  return root_block_index;
}

boolean data_sorted_insert(int block_num, int fileDesc, Record* new_record, char key_type) {
	/* Initialize a pointer to the first block, and get its data */
  BF_Block* first_block;
  BF_Block_Init(&first_block);
  CALL_BF(BF_GetBlock(fileDesc, 0, first_block));
  char* first_block_info = BF_Block_GetData(first_block);
  int	offset = sizeof(char) + 3 * sizeof(int);
  /* Get the max possible records in a data block */
  int max_records;
  memcpy(&max_records, first_block_info + offset, sizeof(int));
  CALL_BF(BF_UnpinBlock(first_block));
  BF_Block_Destroy(&first_block);
  /*data block*/
  BF_Block* block;
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(fileDesc, block_num, block));
  char* data = BF_Block_GetData(block);
  if (data[0] != 'D') {
    printf("Not a data block\n");
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    return false;
  }
  /* Initialize the offset to find where to store the new record */
  offset = sizeof(char) + 3 * sizeof(int);
  int total_records;
	memcpy(&total_records, data + sizeof(char), sizeof(int));
  /*If the block is full, return an error*/
  if (total_records == max_records){
    printf("Record doesn't fit\n");
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    return false;
  }
  int pos = 0;
  int new_total_records;
  /* Traverse all the current records */
  for (int i = 0; i < total_records; ++i) {
    Record* curr_rec = malloc(new_record->size);
    memcpy(curr_rec, data + offset, new_record->size);
    /* If the key of the record we want to insert is smaller than the current one */
    if (compare(new_record->key, LESS_THAN, curr_rec->key, key_type)) {
      /* Shift all records to the right to create space */
      memmove(data + offset + new_record->size, data + offset, (total_records - i) * new_record->size);
      /* Insert the new record */
      memcpy(data + offset, new_record, new_record->size);
      /*total records of block*/
      new_total_records = total_records + 1;
      memcpy(data + sizeof(char), &new_total_records, sizeof(int));

      BF_Block_SetDirty(block);
      CALL_BF(BF_UnpinBlock(block));
      BF_Block_Destroy(&block);
      return true;
    }
    /* Each time, update the offset */
    offset += new_record->size;
		// free(curr_rec);
  }
  /* If we haven't yet inserted the record, now its time(at the end of the block) */
  memcpy(data + offset, new_record, new_record->size);
  /* Increase the total records of the block */
  new_total_records =  total_records + 1;
  memcpy(data + sizeof(char), &new_total_records, sizeof(int));

  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);
  return true;
}

boolean index_sorted_insert(int block_num, int fileDesc, char* new_tuple, char key_type, int key_size) {
	/* Initialize a pointer to the first block, and get its data */
	BF_Block* first_block;
	BF_Block_Init(&first_block);
	CALL_BF(BF_GetBlock(fileDesc, 0, first_block));
	char* first_block_info = BF_Block_GetData(first_block);
	int	offset = sizeof(char) + 2 * sizeof(int);
	/* Get the max possible keys in a data block */
	int max_keys;
	memcpy(&max_keys, first_block_info + offset, sizeof(int));
  CALL_BF(BF_UnpinBlock(first_block));
	BF_Block_Destroy(&first_block);
	/*Initialize a pointer to the block we want to insert */
	BF_Block* block;
	BF_Block_Init(&block);
	CALL_BF(BF_GetBlock(fileDesc, block_num, block));
	char* data = BF_Block_GetData(block);
	/* If we are not dealing with an index block, exit */
	if (data[0] != 'I') {
		printf("Not a index block\n");
		BF_UnpinBlock(block);
		BF_Block_Destroy(&block);
		return false;
	}
	int total_keys;
	memcpy(&total_keys, data + sizeof(char), sizeof(int));
	/* Initialize the offset to find where to store the new key */
	offset = sizeof(char) + sizeof(int);
	/*If the block is full, return an error*/
	if (total_keys == max_keys){
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		return false;
  }
  int pos = 0;
	void* key_to_insert = malloc(key_size);
	memcpy(key_to_insert, new_tuple + sizeof(int), key_size);
	/* Traverse all the current keys */
  int new_total_keys;
	for (int i = 0; i < total_keys; ++i) {
		char* curr_tuple = malloc(2 * sizeof(int) + key_size);
		memcpy(curr_tuple, data + offset, 2 * sizeof(int) + key_size);
		void* curr_key = malloc(key_size);
		memcpy(curr_key, curr_tuple + sizeof(int), key_size);
		/* If the key of the keys we want to insert is smaller than the current one */
		if (compare(key_to_insert, LESS_THAN, curr_key, key_type)) {
			/* Shift all keys to the right to create space */
			memmove(data + offset + 2 * sizeof(int) + key_size ,data + offset + sizeof(int),(total_keys - i) * (key_size + sizeof(int)));
			/* Insert the new key */
			memcpy(data + offset, new_tuple, 2 * sizeof(int) + key_size);
			new_total_keys = total_keys + 1;
	 	 	memcpy(data + sizeof(char), &new_total_keys, sizeof(int));
      BF_Block_SetDirty(block);
	    CALL_BF(BF_UnpinBlock(block));
	    BF_Block_Destroy(&block);
			// free(key_to_insert);
			return true;
		}
		/* Each time, update the offset */
		offset += sizeof(int) + key_size;
		// free(curr_key);
	 }
	 /* If we haven't yet inserted the key, now its time(at the end of the block) */
	 memcpy(data + offset, new_tuple, 2 * sizeof(int) + key_size);
	 new_total_keys = total_keys + 1;
	 memcpy(data + sizeof(char), &new_total_keys, sizeof(int));
	 /* Unpin and destroy the block */
	 BF_Block_SetDirty(block);
	 BF_UnpinBlock(block);
	 BF_Block_Destroy(&block);
	 // free(key_to_insert);

	 /* Everything is ok */
	 return true;
}

char* split_data_block(int fileDesc, int block_num, Record* new_record, char key_type, int key_size) {
	/* Initialize a pointer to the block */
  BF_Block* block;
  BF_Block_Init(&block);
  /* And get access to it */
  BF_GetBlock(fileDesc, block_num, block);
  /* Get access to its data */
  char* block_info = BF_Block_GetData(block);
  /* Get the total records that the block contains*/
  int total_records;
  memcpy(&total_records, block_info + sizeof(char), sizeof(int));
  /* Get the position that we want to split. */
  int split_pos = total_records / 2;

  /* If the key we want to insert is grater than the key of the middle record of
     the block, then we want to keep +1 records in the left block, so use the
     generic compare function that we have */
  Record* middle = malloc(new_record->size);
  int offset = 3 * sizeof(int) + sizeof(char) + split_pos * new_record->size;
  memcpy(middle, block_info + offset, new_record->size);
  int right = compare(new_record->key, GREATER_THAN, middle->key, key_type);
  /* Update the total_records. There will be those that the block will have after
     the split */
  offset = sizeof(char);
  memcpy(block_info + offset, &split_pos, sizeof(int));
  /* Allocate and initialize a new data block */
  BF_Block* new_block;
  BF_Block_Init(&new_block);
  BF_AllocateBlock(fileDesc, new_block);
  char* new_block_data = BF_Block_GetData(new_block);

  /* Save its info: the fact that is a data block + the amount of records we are
    going to insert */
  char type = 'D'; int init_records = total_records - split_pos;
  memcpy(new_block_data, &type, sizeof(char));
  memcpy(new_block_data + sizeof(char), &init_records, sizeof(int));

  /* Update the neighbor data block pointers as following */
  int prev; int old_next; int next;
  BF_GetBlockCounter(fileDesc, &next); next--;
  /* the new next of the first block, is the one that we just allocated */
  offset += 2 * sizeof(int);
  memcpy(&old_next, block_info + offset, sizeof(int));
  memcpy(block_info + offset, &next, sizeof(int));
	offset += sizeof(int);
  /* the previous block of the new one, is the one that we got as an argument */
  prev = block_num;
  next = old_next;
  int new_block_offset = sizeof(char) + sizeof(int);
  memcpy(new_block_data + new_block_offset, &prev, sizeof(int));
  new_block_offset += sizeof(int);
  memcpy(new_block_data + new_block_offset, &next, sizeof(int));
  new_block_offset += sizeof(int);
  offset += split_pos * new_record->size;

  /* Copy the data from the full block to the empty, so we have two half-full
  data blocks */
  memcpy(new_block_data + new_block_offset, block_info + offset, init_records * new_record->size);

  /* Fill the rest of the first block with -1 values */
  memset(block_info + offset, -1, init_records * new_record->size);
  /*set dirty before calling the other functions*/
  BF_Block_SetDirty(block);
  BF_Block_SetDirty(new_block);
  /*Manage the total records of each block accoridngly before inserting a new one*/
  /*Finally, add the new record in the correct block */
  if (right == 0) {
    data_sorted_insert(block_num, fileDesc, new_record, key_type);
    /*the total records of the block have been updated inside the sorted_insert_block */
  }
  else {
		int new_block_num;
		BF_GetBlockCounter(fileDesc, &new_block_num);
    new_block_num--;
    data_sorted_insert(new_block_num, fileDesc, new_record, key_type);
  }
	/* We want to return the key of the first record of the left block, and the
		 two pointers surrounding it */
	char* to_return = malloc(key_size + 2 * sizeof(int));
  memcpy(to_return, &block_num, sizeof(int));
	offset = 3 * sizeof(int) + sizeof(char);
	Record* rec_to_return = malloc(new_record->size);
	memcpy(rec_to_return, new_block_data + offset, new_record->size);
  void * key1 = malloc(key_size);
  memcpy(key1,rec_to_return->key,key_size);
  memcpy(to_return + sizeof(int), key1, key_size);
	/* Call BF_GetBlockCounter to find the number of the newly allocated block */
	int new_block_num;
	BF_GetBlockCounter(fileDesc, &new_block_num);
	new_block_num--;
	memcpy(to_return + sizeof(int) + key_size, &new_block_num, sizeof(int));

  /* Set the previously edited blocks dirty, and unpin the, from the memory */
  BF_Block_SetDirty(block);
  BF_Block_SetDirty(new_block);
  BF_UnpinBlock(block);
  BF_UnpinBlock(new_block);
  BF_Block_Destroy(&block);
  BF_Block_Destroy(&new_block);
	// free(rec_to_return);
	// free(key1);
  /* Return the key of the record we want as an index */
  return to_return;
}

char* split_index_block(int fileDesc, int block_num, char* new_entry, char key_type, int key_size) {
  /* Initialize a pointer to the block */
  BF_Block* block;
  BF_Block_Init(&block);
  /* And get access to it */
  BF_GetBlock(fileDesc, block_num, block);
  /* Get access to its data */
  char* block_info = BF_Block_GetData(block);
  /* Get the total records that the block contains*/
  int total_keys;
  memcpy(&total_keys, block_info + sizeof(char), sizeof(int));
  int offset = sizeof(char) + sizeof(int);
  /* Get the position that we want to split. If the block contains odd number of
    records, we want one more on the right */
  int split_pos = total_keys / 2;
	/* Store the new total records of the left block */
	memcpy(block_info + sizeof(char), &split_pos, sizeof(int));
	/* If the key we want to insert is grater than the middle key of the of the
     block, then we want to keep +1 keys in the left block, so use the
     generic compare function that we have */
	/* First, get the new key from the new entry argument */
	void* new_key = malloc(key_size);
	memcpy(new_key, new_entry + sizeof(int), key_size);
	/* Find the middle entry */
  void* middle = malloc(key_size);
  offset = sizeof(int) + sizeof(char) + split_pos * sizeof(int) + (split_pos - 1) * key_size;
  memcpy(middle, block_info + offset, key_size);
  int right = compare(new_key, GREATER_THAN, middle, key_type);
  /* Allocate and initialize a new index block */
  BF_Block* new_block;
  BF_Block_Init(&new_block);
  BF_AllocateBlock(fileDesc, new_block);
  char* new_block_data = BF_Block_GetData(new_block);
  char type = 'I'; int init_keys = total_keys - split_pos;
  memcpy(new_block_data, &type, sizeof(char));
  memcpy(new_block_data + sizeof(char), &init_keys, sizeof(int));
  offset = sizeof(int) + sizeof(char) + split_pos * sizeof(int) + split_pos * key_size;
  int new_block_offset = sizeof(char) + sizeof(int);
  /* Copy the data from the full block to the empty, so we have two half-full
  index blocks */
	int copy_size = (init_keys + 1) * sizeof(int) + init_keys * key_size;
  memcpy(new_block_data + new_block_offset, block_info + offset, copy_size);
	/** Set the remainder of the left block to have -1 values (nut also keep the
		 	pointer stored at the end of the usefull data) */
	memset(block_info + offset + sizeof(int), -1, BF_BLOCK_SIZE - (offset + sizeof(int)));
  /*set dirty before calling the other functions*/
  BF_Block_SetDirty(block);
	int blockno;
	BF_GetBlockCounter(fileDesc, &blockno); blockno--;
  BF_Block_SetDirty(new_block);
	/*Finally, add the new key(and its pointers) in the correct block */
	if (right == 0) {
		index_sorted_insert(block_num, fileDesc, new_entry, key_type, key_size);
		/*the total records of the block have been updated inside the sorted_insert_block */
	}
	else {
		int new_block_num;
		BF_GetBlockCounter(fileDesc, &new_block_num);
		new_block_num--;
		index_sorted_insert(new_block_num, fileDesc, new_entry, key_type, key_size);
	}

	void* key_to_return = malloc(key_size);
	/* Surpass the metadata and the first pointer, to get to the first key */
	new_block_offset = sizeof(char) + sizeof(int) + sizeof(int);
	memcpy(key_to_return, new_block_data + new_block_offset, key_size);
	new_block_offset -= sizeof(int);
	int move_offset = new_block_offset + sizeof(int) + key_size;
	memmove(new_block_data + new_block_offset, new_block_data + move_offset,  BF_BLOCK_SIZE - move_offset);
	int new_total_keys;
	/* Decrease the total keys of the second block, as we deleted one */
	memcpy(&new_total_keys, new_block_data + sizeof(char), sizeof(int));
	new_total_keys--;
	memcpy(new_block_data + sizeof(char), &new_total_keys, sizeof(int));

	int pointer1 = block_num; int pointer2;
	BF_GetBlockCounter(fileDesc, &pointer2);
	pointer2--;
	char* to_return = malloc(key_size + 2 * sizeof(int));
	memcpy(to_return, &pointer1, sizeof(int));
	memcpy(to_return + sizeof(int), key_to_return, key_size);
	memcpy(to_return + key_size + sizeof(int), &pointer2, sizeof(int));
  /* Set dirty, unpin and destroy the blocks */
  BF_Block_SetDirty(block);
  BF_Block_SetDirty(new_block);
  BF_UnpinBlock(block);
  BF_UnpinBlock(new_block);
  BF_Block_Destroy(&block);
  BF_Block_Destroy(&new_block);
	// free(key_to_return);
	// free(middle);
  /* Return the string we want to append to the parent */
  return to_return;
}

boolean record_fits_data(int fileDesc, int target_block_index) {
	boolean fits;
	BF_Block *first_block, *target_block;
	BF_Block_Init(&first_block);
	BF_Block_Init(&target_block);

	BF_GetBlock(fileDesc, 0, first_block);
	char* first_block_info = BF_Block_GetData(first_block);
	BF_GetBlock(fileDesc, target_block_index, target_block);
	char* target_block_info = BF_Block_GetData(target_block);
	int offset = sizeof(char);
	int target_block_records;
	memcpy(&target_block_records, target_block_info + offset, sizeof(int));

	int max_block_records;
  offset = sizeof(char) + 3 * sizeof(int);
	memcpy(&max_block_records,first_block_info + offset, sizeof(int));

	if (max_block_records > target_block_records) {
		fits = true;
	}
	else {
		fits = false;
	}
	BF_UnpinBlock(target_block);
	BF_UnpinBlock(first_block);
	BF_Block_Destroy(&first_block);
	BF_Block_Destroy(&target_block);
	return fits;
}

boolean key_fits_index(int fileDesc, int target_block_index) {
	boolean fits;
	BF_Block *first_block, *target_block;
	BF_Block_Init(&first_block);
	BF_Block_Init(&target_block);

	BF_GetBlock(fileDesc, 0, first_block);
	char* first_block_info = BF_Block_GetData(first_block);
	BF_GetBlock(fileDesc, target_block_index, target_block);
	char* target_block_info = BF_Block_GetData(target_block);
	int offset = sizeof(char);
	int target_block_keys;
	memcpy(&target_block_keys, target_block_info + offset, sizeof(int));

	int max_block_keys;
  offset = sizeof(char) + 2 * sizeof(int);
	memcpy(&max_block_keys, first_block_info + offset, sizeof(int));

	if (max_block_keys > target_block_keys) {
		fits = true;
	}
	else {
		fits = false;
	}
	BF_UnpinBlock(target_block);
	BF_UnpinBlock(first_block);
	BF_Block_Destroy(&first_block);
	BF_Block_Destroy(&target_block);
	return fits;
}
