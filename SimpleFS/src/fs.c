#include "disk.h"
#include "fs.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#define min(a,b) (((a) < (b)) ? (a) : (b))

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif

// Debug file system -----------------------------------------------------------

void fs_debug(Disk *disk)
{
    if (disk == 0)
        return;

    Block block;

    // Read Superblock
    disk_read(disk, 0, block.Data);

    uint32_t magic_num = block.Super.MagicNumber;
    uint32_t num_blocks = block.Super.Blocks;
    uint32_t num_inodeBlocks = block.Super.InodeBlocks;
    uint32_t num_inodes = block.Super.Inodes;

    if (magic_num != MAGIC_NUMBER)
    {
        printf("Magic number is invalid: %c\n", magic_num);
        return;
    }

    printf("SuperBlock:\n");
    printf("    magic number is valid\n");
    printf("    %u blocks\n", num_blocks);
    printf("    %u inode blocks\n", num_inodeBlocks);
    printf("    %u inodes\n", num_inodes);

    uint32_t expected_num_inodeBlocks = round((float)num_blocks / 10);

    if (expected_num_inodeBlocks != num_inodeBlocks)
    {
        printf("SuperBlock declairs %u InodeBlocks but expect %u InodeBlocks!\n", num_inodeBlocks, expected_num_inodeBlocks);
    }

    uint32_t expect_num_inodes = num_inodeBlocks * INODES_PER_BLOCK;
    if (expect_num_inodes != num_inodes)
    {
        printf("SuperBlock declairs %u Inodes but expect %u Inodes!\n", num_inodes, expect_num_inodes);
    }

    // FIXME: Read Inode blocks
    Block block2;
    //INODE number
    uint32_t number = 0;
    // iterate through INODE blocks
    for (uint32_t i = 1; i <= num_inodeBlocks; ++i) {
        disk_read(disk, i, block2.Data);
        // iterate throught INODES in INODE block
        for (uint32_t j = 0; j < INODES_PER_BLOCK; ++j) {
            uint32_t valid = block2.Inodes[j].Valid;
            if (valid) {
                uint32_t size = block2.Inodes[j].Size;
                uint32_t direct[POINTERS_PER_INODE];
                uint32_t indirect[POINTERS_PER_BLOCK];
                uint32_t indirect_num = 0;
                // determine number of valid direct pointers in INODE
                for (uint32_t k = 0; k < POINTERS_PER_INODE; ++k) {
                    if (block2.Inodes[j].Direct[k] != 0) {
                        direct[k] = block2.Inodes[j].Direct[k];
                    }
                    else {
                        direct[k] = 0;
                    }
                }
                Block block3;
                if (block2.Inodes[j].Indirect != 0) {
                    indirect_num = block2.Inodes[j].Indirect;
                    disk_read(disk, block2.Inodes[j].Indirect, block3.Data);
                    for (uint32_t k = 0; k < POINTERS_PER_BLOCK; ++k) {
                        if (block3.Pointers[k] != 0) {
                            indirect[k] = block3.Pointers[k];
                        }
                        else {
                            indirect[k] = 0;
                        }
                    }
                }
                printf("Inode %u:\n", number);
                printf("    size: %u bytes\n", size);
                printf("    direct blocks:");
                for (uint32_t k = 0; k < POINTERS_PER_INODE; ++k) {
                    if (direct[k] != 0) {
                        printf(" %u", direct[k]);
                    }
                }
                printf("\n");
                if (indirect_num != 0) {
                    printf("    indirect block: %u\n", indirect_num);
                    printf("    indirect data blocks:");
                    for (uint32_t k = 0; k < POINTERS_PER_BLOCK; ++k) {
                        if (indirect[k] != 0) {
                            printf(" %u", indirect[k]);
                        }
                    }
                    printf("\n");
                }
            }
            ++number;
        }
    }

}

// Format file system ----------------------------------------------------------

bool fs_format(Disk *disk)
{
    if (disk_mounted(disk)) {
        return false;
    }
 
    // Write superblock
    Block block;
    block.Super.MagicNumber = MAGIC_NUMBER;
    block.Super.Blocks = disk_size(disk);
    block.Super.InodeBlocks = round((float)block.Super.Blocks / 10);
    block.Super.Inodes = block.Super.InodeBlocks*INODES_PER_BLOCK;
    disk_write(disk, 0, block.Data);
    
    char clear[BUFSIZ] = {0};
    for (uint32_t i = 1; i < block.Super.Blocks; ++i) {
        disk_write(disk, i, clear);
    }
    
    return true;
}

// FileSystem constructor 
FileSystem *new_fs()
{
    FileSystem *fs = malloc(sizeof(FileSystem));
    return fs;
}

// FileSystem destructor 
void free_fs(FileSystem *fs)
{
    // FIXME: free resources and allocated memory in FileSystem
    free(fs->bitmap);

    free(fs);
}

// Mount file system -----------------------------------------------------------

bool fs_mount(FileSystem *fs, Disk *disk)
{
    
    if (disk_mounted(disk)) {
        return false;
    }
    
    // Read superblock
    Block block;
    disk_read(disk, 0, block.Data);

    uint32_t magic_num = block.Super.MagicNumber;
    uint32_t num_blocks = block.Super.Blocks;
    uint32_t num_inodeBlocks = block.Super.InodeBlocks;
    uint32_t num_inodes = block.Super.Inodes;

    if (magic_num != MAGIC_NUMBER)
    {
        return false;
    }

    uint32_t expected_num_inodeBlocks = round((float)num_blocks / 10);

    if (expected_num_inodeBlocks != num_inodeBlocks)
    {
        return false;
    }

    uint32_t expect_num_inodes = num_inodeBlocks * INODES_PER_BLOCK;
    if (expect_num_inodes != num_inodes)
    {
        return false;
    }

    // Set device and mount
    disk_mount(disk);

    // Copy metadata
    fs->Blocks = block.Super.Blocks;
    fs->InodeBlocks = block.Super.InodeBlocks;
    fs->Inodes = block.Super.Inodes;
    fs->disk = disk;

    // Allocate free block bitmap
    fs->bitmap = malloc(sizeof(uint32_t)*fs->Blocks);
    for (int i = 0; i < fs->Blocks; ++i) {
        fs->bitmap[i] = 1;
    }
    // SuperBlock not free
    fs->bitmap[0] = 0;
    // Inodes not free
    for (uint32_t i = 1; i <= fs->InodeBlocks; ++i) {
        fs->bitmap[i] = 0;
    }
    Block block2;
    for (uint32_t i = 1; i <= fs->InodeBlocks; ++i) {
        disk_read(disk, i, block2.Data);
        for (uint32_t j = 0; j < INODES_PER_BLOCK; ++j) {
            if (block2.Inodes[j].Valid) {
                uint32_t direct_blocks = 0;
                for (int k = 0; k < POINTERS_PER_INODE; ++k) {
                    if (block2.Inodes[j].Direct[k] != 0) {
                        ++direct_blocks;
                        fs->bitmap[block2.Inodes[j].Direct[k]] = 0;
                    }
                }
                uint32_t num_blocks = ceil(block2.Inodes[j].Size/BLOCK_SIZE);
                if (num_blocks > direct_blocks) {
                    Block block3;
                    disk_read(disk, block2.Inodes[j].Indirect, block3.Data);
                    fs->bitmap[block2.Inodes[j].Indirect] = 0;
                    for (int k = 0; k < num_blocks - direct_blocks + 1; ++k) {
                        fs->bitmap[block3.Pointers[k]] = 0;
                    }
                }
            }
        }
    }

    return true;
}

// Create inode ----------------------------------------------------------------

ssize_t fs_create(FileSystem *fs)
{
    
    // Locate free inode in inode table
    uint32_t inode_index = -1;
    uint32_t block_num = 0;
    Block block;
    for (uint32_t i = 1; i <= fs->InodeBlocks; ++i) {
        disk_read(fs->disk, i, block.Data);
        for (uint32_t j = 0; j < INODES_PER_BLOCK; ++j) {
            if (!block.Inodes[j].Valid) {
                inode_index = j;
                block_num = i;
                break;
            }
        }
        if (inode_index != -1) {
            break;
        }
    }
    
    if (inode_index == -1) {
        return -1;
    }

    // Record inode if found
    uint32_t inumber = inode_index + (block_num-1) * INODES_PER_BLOCK;

    if (inumber >= fs->Inodes) {
        return -1;
    }

    Inode new;
    new.Valid = 1;
    new.Size = 0;
    for (uint32_t i = 0; i < POINTERS_PER_INODE; ++i) {
        new.Direct[i] = 0;
    }
    new.Indirect = 0;

    Block block2;
    disk_read(fs->disk, block_num, block2.Data);
    block2.Inodes[inode_index] = new;
    disk_write(fs->disk, block_num, block2.Data);
    return inumber;
}

// Optional: the following two helper functions may be useful. 

// bool find_inode(FileSystem *fs, size_t inumber, Inode *inode)
// {
//     return true;
// }

// bool store_inode(FileSystem *fs, size_t inumber, Inode *inode)
// {
//     return true;
// }

// Remove inode ----------------------------------------------------------------

bool fs_remove(FileSystem *fs, size_t inumber)
{
    // Load inode information
    if (inumber >= fs->Inodes) {
        return false;
    }
    uint32_t block_num = floor(inumber/INODES_PER_BLOCK) + 1;
    uint32_t inode_index = inumber % INODES_PER_BLOCK;
    Block block;
    disk_read(fs->disk, block_num, block.Data);

    Inode del = block.Inodes[inode_index];
    
    if (!del.Valid) {
        return false;
    }

    // Free direct blocks
    for (uint32_t i = 0; i < POINTERS_PER_INODE; ++i) {
        if (del.Direct[i] != 0) {
            fs->bitmap[del.Direct[i]] = 1;
            del.Direct[i] = 0;
        }
    }

    // Free indirect blocks
    if (del.Indirect != 0) {
        Block block2;
        disk_read(fs->disk, del.Indirect, block2.Data);
        for (int i = 0; i < POINTERS_PER_BLOCK; ++i) {
            if (block2.Pointers[i] != 0) {
                fs->bitmap[block2.Pointers[i]] = 1;
                block2.Pointers[i] = 0;
            }
        }
        disk_write(fs->disk, del.Indirect, block2.Data);
        fs->bitmap[del.Indirect] = 1;
    }

    // Clear inode in inode table
    del.Valid = 0;
    del.Size = 0;
    del.Indirect = 0;

    block.Inodes[inode_index] = del;
    disk_write(fs->disk, block_num, block.Data);

    return true;
}

// Inode stat ------------------------------------------------------------------

ssize_t fs_stat(FileSystem *fs, size_t inumber)
{
    // Load inode information
    if (inumber >= fs->Inodes) {
        return -1;
    }
    uint32_t block_num = floor(inumber/INODES_PER_BLOCK) + 1;
    uint32_t inode_index = inumber % INODES_PER_BLOCK;
    Block block;
    disk_read(fs->disk, block_num, block.Data);

    Inode in = block.Inodes[inode_index];
    
    if (!in.Valid) {
        return -1;
    }
    return in.Size;
}

// Read from inode -------------------------------------------------------------

ssize_t fs_read(FileSystem *fs, size_t inumber, char *data, size_t length, size_t offset)
{
    // Load inode information
    if (inumber >= fs->Inodes) {
        return -1;
    }
    uint32_t block_num = floor(inumber/INODES_PER_BLOCK) + 1;
    uint32_t inode_index = inumber % INODES_PER_BLOCK;
    Block block;
    disk_read(fs->disk, block_num, block.Data);

    Inode read_block = block.Inodes[inode_index];
    
    if (!read_block.Valid) {
        return -1;
    }
    if (offset > read_block.Size) {
        return -1;
    }

    // Adjust length

    length = min(length, read_block.Size - offset);


    // Read block and copy to data

    uint32_t num_blocks = ceil((float)length/BLOCK_SIZE);


    // Check if Indirect blocks will be needed
    uint32_t indirect = 0;
    uint32_t total_num = ceil((float)(length + offset)/BLOCK_SIZE);
    if (total_num > POINTERS_PER_INODE) {
        if (read_block.Indirect != 0) {
            indirect = read_block.Indirect;
        }
        else {
            return -1;
        }
    }

    size_t read_total = 0;
    uint32_t start = (float)offset/BLOCK_SIZE;
    Block Indirect;
    if (indirect) {
        disk_read(fs->disk, indirect, Indirect.Data);
    }
    for (uint32_t cur_block = start; cur_block < start + num_blocks; ++cur_block) {
        // check Direct or Indirect
        Block cur;
        uint32_t cur_num;
        if (cur_block >= POINTERS_PER_INODE) {
            cur_num = Indirect.Pointers[cur_block-POINTERS_PER_INODE];
        }
        else {
            cur_num = read_block.Direct[cur_block];
        }

        
        if (cur_num != 0) {
            disk_read(fs->disk, cur_num, cur.Data);
        }
        else {
            return -1;
        }
        

        size_t length_read;
        char cpy[BUFSIZ];
        if (cur_block == start) {
            size_t offset_read = offset % BLOCK_SIZE;
            length_read = min(BLOCK_SIZE - offset_read, length);
            strncpy(cpy, cur.Data + offset_read, length_read);
        }
        else {
            length_read = min(BLOCK_SIZE, length-read_total);
            strncpy(cpy, cur.Data, length_read);
            
        }
        strncat(data, cpy, length_read);
        read_total += length_read;
    }

    return read_total;
}

// Optional: the following helper function may be useful. 

ssize_t fs_allocate_block(FileSystem *fs)
{
    ssize_t found = 0;
    for (int i = fs->InodeBlocks+1; i < fs->Blocks; ++i) {
        if (fs->bitmap[i] != 0) {
            found = i;
            fs->bitmap[i] = 0;
            break;
        }
    }
    if (found) {
        char data[BLOCK_SIZE] = {0};
        disk_write(fs->disk, found, data);
        return found;
    }
    else {
        return -1;
    }
}

// Write to inode --------------------------------------------------------------

ssize_t fs_write(FileSystem *fs, size_t inumber, char *data, size_t length, size_t offset)
{
    
    // Load inode information
    if (inumber >= fs->Inodes) {
        return -1;
    }
    uint32_t block_num = floor(inumber/INODES_PER_BLOCK) + 1;
    uint32_t inode_index = inumber % INODES_PER_BLOCK;
    Block block;
    disk_read(fs->disk, block_num, block.Data);

    Inode write_block = block.Inodes[inode_index];
    
    if (!write_block.Valid) {
        return -1;
    }
    /*
    if (offset > write_block.Size) {
        return -1;
    }
    */

    // Adjust length
    size_t MAX = BLOCK_SIZE*(POINTERS_PER_INODE + POINTERS_PER_BLOCK);
    length = min(length, MAX - offset);

    // Write block and copy to data

    uint32_t num_blocks = ceil((float)length/BLOCK_SIZE);

    size_t write_total = 0;
    uint32_t start = (float)offset/BLOCK_SIZE;
    Block Indirect;
    uint32_t indirect = write_block.Indirect;
    bool mod_ind = false;
    bool indirect_read = false;
    bool mod_indirect = false;
    for (uint32_t cur_block = start; cur_block < start + num_blocks; ++cur_block) {
        // check Direct or Indirect
        Block cur;
        uint32_t cur_num;
        if (cur_block >= POINTERS_PER_INODE) {
            if (!indirect) {
                indirect = fs_allocate_block(fs);
                if (indirect == -1) {
                    break;
                }
                else {
                    // check if indirect block will actually be used
                    uint32_t temp;
                    temp = fs_allocate_block(fs);
                    if (temp == -1) {
                        // deallocate indirect block;
                        fs->bitmap[indirect] = 1;
                        break;
                    }
                    fs->bitmap[temp] = 1;
                    write_block.Indirect = indirect;
                    mod_ind = true;
                }
            }
            if (!indirect_read) {
                disk_read(fs->disk, indirect, Indirect.Data);
                indirect_read = true;
            }
            cur_num = Indirect.Pointers[cur_block - POINTERS_PER_INODE];
            if (cur_num == 0) {
                cur_num = fs_allocate_block(fs);
                if (cur_num == -1) {
                    break;
                }
                Indirect.Pointers[cur_block - POINTERS_PER_INODE] = cur_num;
                mod_indirect = true;
            }
        }
        else {
            cur_num = write_block.Direct[cur_block];
            if (cur_num == 0) {
                cur_num = fs_allocate_block(fs);
                if (cur_num == -1) {
                    break;
                }
                write_block.Direct[cur_block] = cur_num;
                mod_ind = true;
            }
        }
        
        disk_read(fs->disk, cur_num, cur.Data);



        size_t length_write;
        size_t offset_write;
        if (cur_block == start) {
            offset_write = offset % BLOCK_SIZE;
            length_write = min(BLOCK_SIZE - offset_write, length);
        }
        else {
            offset_write = 0;
            length_write = min(BLOCK_SIZE, length-write_total);
        }
        memcpy(cur.Data + offset_write, data + write_total, length_write);
        disk_write(fs->disk, cur_num, cur.Data);
        write_total += length_write;
    }

    uint32_t mod_size = write_total + offset;
    if (mod_size > write_block.Size) {
        write_block.Size = mod_size;        
        mod_ind = true;
    }
    if (mod_ind) {
        block.Inodes[inode_index] = write_block;
        disk_write(fs->disk, block_num, block.Data);
    }
    if (mod_indirect) {
        disk_write(fs->disk, indirect, Indirect.Data);
    }

    return write_total;

}
