/*
 * niffs.h
 *
 *  Created on: Feb 2, 2015
 *      Author: petera
 */

#ifndef NIFFS_H_
#define NIFFS_H_

#include "niffs_config.h"

#define NIFFS_SEEK_SET          (0)
#define NIFFS_SEEK_CUR          (1)
#define NIFFS_SEEK_END          (2)

/* Any write to the filehandle is appended to end of the file */
#define NIFFS_O_APPEND                   (1<<0)
/* If the opened file exists, it will be truncated to zero length before opened */
#define NIFFS_O_TRUNC                    (1<<1)
/* If the opened file does not exist, it will be created before opened */
#define NIFFS_O_CREAT                    (1<<2)
/* The opened file may only be read */
#define NIFFS_O_RDONLY                   (1<<3)
/* The opened file may only be writted */
#define NIFFS_O_WRONLY                   (1<<4)
/* The opened file may be both read and written */
#define NIFFS_O_RDWR                     (NIFFS_O_RDONLY | NIFFS_O_WRONLY)
/* Any writes to the filehandle will never be cached */
#define NIFFS_O_DIRECT                   (1<<5)
/* If O_CREAT and O_EXCL are set, open() fails if the file exists. */
#define NIFFS_O_EXCL                     (1<<6)


#define NIFFS_OK                            0
#define ERR_NIFFS_BAD_CONF                  -11001
#define ERR_NIFFS_NOT_A_FILESYSTEM          -11002
#define ERR_NIFFS_BAD_SECTOR                -11003
#define ERR_NIFFS_DELETING_FREE_PAGE        -11004
#define ERR_NIFFS_DELETING_DELETED_PAGE     -11005
#define ERR_NIFFS_MOVING_FREE_PAGE          -11006
#define ERR_NIFFS_MOVING_DELETED_PAGE       -11007
#define ERR_NIFFS_MOVING_TO_UNFREE_PAGE     -11008
#define ERR_NIFFS_MOVING_TO_SAME_PAGE       -11009
#define ERR_NIFFS_MOVING_BAD_FLAG           -11010
#define ERR_NIFFS_NO_FREE_PAGE              -11011
#define ERR_NIFFS_SECTOR_UNFORMATTABLE      -11012
#define ERR_NIFFS_NULL_PTR                  -11013
#define ERR_NIFFS_NO_FREE_ID                -11014
#define ERR_NIFFS_WR_PHDR_UNFREE_PAGE       -11015
#define ERR_NIFFS_WR_PHDR_BAD_ID            -11016
#define ERR_NIFFS_NAME_CONFLICT             -11017
#define ERR_NIFFS_FULL                      -11018
#define ERR_NIFFS_OUT_OF_FILEDESCS          -11019
#define ERR_NIFFS_FILE_NOT_FOUND            -11020
#define ERR_NIFFS_FILEDESC_CLOSED           -11021
#define ERR_NIFFS_FILEDESC_BAD              -11022
#define ERR_NIFFS_INCOHERENT_ID             -11023
#define ERR_NIFFS_PAGE_NOT_FOUND            -11024
#define ERR_NIFFS_END_OF_FILE               -11025
#define ERR_NIFFS_MODIFY_BEYOND_FILE        -11026
#define ERR_NIFFS_TRUNCATE_BEYOND_FILE      -11027
#define ERR_NIFFS_NO_GC_CANDIDATE           -11028
#define ERR_NIFFS_PAGE_DELETED              -11029
#define ERR_NIFFS_PAGE_FREE                 -11030
#define ERR_NIFFS_MOUNTED                   -11031
#define ERR_NIFFS_NOT_MOUNTED               -11032
#define ERR_NIFFS_NOT_WRITABLE              -11033
#define ERR_NIFFS_NOT_READABLE              -11034
#define ERR_NIFFS_FILE_EXISTS               -11035
#define ERR_NIFFS_OVERFLOW                  -11036

typedef int (* niffs_hal_erase_f)(u8_t *addr, u32_t len);
typedef int (* niffs_hal_write_f)(u8_t *addr, u8_t *src, u32_t len);
// dummy type, for posix compliance
typedef u16_t niffs_mode;
// niffs file descriptor flags
typedef u8_t niffs_fd_flags;

typedef struct {
  niffs_obj_id obj_id;
  niffs_page_ix obj_pix;
  u32_t offs;
  niffs_page_ix cur_pix;
  niffs_fd_flags flags;
} niffs_file_desc;

typedef struct {
  // cfg
  u8_t *phys_addr;
  u32_t sectors;
  u32_t sector_size;
  u32_t page_size;

  u8_t *buf;
  u32_t buf_len;

  niffs_hal_write_f hal_wr;
  niffs_hal_erase_f hal_er;

  // dyna
  u32_t pages_per_sector;
  niffs_page_ix last_free_pix;
  u8_t mounted;
  u32_t free_pages;
  u32_t dele_pages;
  niffs_file_desc *descs;
  u32_t descs_len;
  u32_t max_era;
#ifdef NIFFS_RD_ALLO_TEST
  u8_t rd_buf[256]; // TODO for test only
#endif
} niffs;

/* niffs file status struct */
typedef struct {
  niffs_obj_id obj_id;
  u32_t size;
  u8_t name[NIFFS_NAME_LEN];
} niffs_stat;

struct niffs_dirent {
  niffs_obj_id obj_id;
  u8_t name[NIFFS_NAME_LEN];
  u32_t size;
  niffs_page_ix pix;
};

typedef struct {
  niffs *fs;
  niffs_page_ix pix;
} niffs_DIR;

/**
 * Initializes and configures the file system
 * @param fs            the file system struct
 * TODO
 */
int NIFFS_init(niffs *fs,
    u8_t *phys_addr,
    u32_t sectors,
    u32_t sector_size,
    u32_t page_size,
    u8_t *buf,
    u32_t buf_len,
    niffs_file_desc *descs,
    u32_t file_desc_len,
    niffs_hal_erase_f erase_f,
    niffs_hal_write_f write_f
    );

/**
 * Mounts the filesystem
 */
int NIFFS_mount(niffs *fs);

/**
 * Returns some general info
 * @param fs            the file system struct
 * @param total         will be populated with total amount of bytes in filesystem
 * @param used          will be populated with used bytes in filesystem
 * @param overflow      if !0, this means you should delete some files and run a check.
 *                      This can happen if filesystem loses power repeatedly during
 *                      garbage collection or check.
 */
int NIFFS_info(niffs *fs, s32_t *total, s32_t *used, u8_t *overflow);

/**
 * Creates a new file.
 * @param fs            the file system struct
 * @param name          the name of the new file
 * @param mode          ignored, for posix compliance
 */
int NIFFS_creat(niffs *fs, char *name, niffs_mode mode);

/**
 * Opens/creates a file.
 * @param fs            the file system struct
 * @param path          the path of the new file
 * @param flags         the flags for the open command, can be combinations of
 *                      NIFFS_O_APPEND, NIFFS_O_TRUNC, NIFFS_O_CREAT, NIFFS_O_RDONLY,
 *                      NIFFS_O_WRONLY, NIFFS_O_RDWR, NIFFS_O_DIRECT
 * @param mode          ignored, for posix compliance
 */
int NIFFS_open(niffs *fs, char *name, u8_t flags, niffs_mode mode);

/**
 * Returns a pointer directly to the flash where data resides, and how many
 * bytes which can be read.
 * This function does not advance the file descriptor offset, so NIFFS_lseek
 * should be called prior to NIFFS_read_ptr.
 * @param fs            the file system struct
 * @param fd            the filehandle
 * @param ptr           ptr which is populated with adress to data
 * @param len           populated with valid data length
 */

int NIFFS_read_ptr(niffs *fs, int fd, u8_t **ptr, u32_t *len);
/**
 * Reads from given filehandle.
 * NB: consider using NIFFS_read_ptr instead. This will basically copy from your
 * internal flash to your ram. If you're only interested in reading data and not
 * modifying it, this will basically waste cycles and ram on memcpy.
 * @param fs            the file system struct
 * @param fd            the filehandle
 * @param buf           where to put read data
 * @param len           how much to read
 * @returns number of bytes read, or error
 */
int NIFFS_read(niffs *fs, int fd, u8_t *dst, u32_t len);

/**
 * Moves the read/write file offset
 * @param fs            the file system struct
 * @param fh            the filehandle
 * @param offs          how much/where to move the offset
 * @param whence        if NIFFS_SEEK_SET, the file offset shall be set to offset bytes
 *                      if NIFFS_SEEK_CUR, the file offset shall be set to its current location plus offset
 *                      if NIFFS_SEEK_END, the file offset shall be set to the size of the file plus offset
 */
int NIFFS_lseek(niffs *fs, int fd, s32_t offs, int whence);

/**
 * Removes a file by name
 * @param fs            the file system struct
 * @param name          the name of the file to remove
 */
int NIFFS_remove(niffs *fs, char *name);

/**
 * Removes a file by filehandle
 * @param fs            the file system struct
 * @param fd            the filehandle of the file to remove
 */
int NIFFS_fremove(niffs *fs, int fd);

/**
 * Writes to given filehandle.
 * @param fs            the file system struct
 * @param fd            the filehandle
 * @param buf           the data to write
 * @param len           how much to write
 * @returns number of bytes written or error
 */
int NIFFS_write(niffs *fs, int fd, u8_t *data, u32_t len);

/**
 * Flushes all pending write operations from cache for given file
 * @param fs            the file system struct
 * @param fd            the filehandle of the file to flush
 */
int NIFFS_fflush(niffs *fs, int fd);

/**
 * Gets file status by name
 * @param fs            the file system struct
 * @param path          the name of the file to stat
 * @param s             the stat struct to populate
 */
int NIFFS_stat(niffs *fs, char *name, niffs_stat *s);

/**
 * Gets file status by filehandle
 * @param fs            the file system struct
 * @param fd            the filehandle of the file to stat
 * @param s             the stat struct to populate
 */
int NIFFS_fstat(niffs *fs, int fd, niffs_stat *s);

/**
 * Gets current position in stream
 * @param fs            the file system struct
 * @param fd            the filehandle of the file to return position from
 */
int NIFFS_ftell(niffs *fs, int fd);

/**
 * Closes a filehandle. If there are pending write operations, these are finalized before closing.
 * @param fs            the file system struct
 * @param fd            the filehandle of the file to close
 */
int NIFFS_close(niffs *fs, int fd);

/**
 * Renames a file.
 * @param fs            the file system struct
 * @param old           name of file to rename
 * @param new           new name of file
 */
int NIFFS_rename(niffs *fs, char *old, char *new);

/**
 * Opens a directory stream corresponding to the given name.
 * The stream is positioned at the first entry in the directory.
 * The name argument is ignored as hydrogen builds always correspond
 * to a flat file structure - no directories.
 * @param fs            the file system struct
 * @param name          the name of the directory
 * @param d             pointer the directory stream to be populated
 */
niffs_DIR *NIFFS_opendir(niffs *fs, char *name, niffs_DIR *d);

/**
 * Closes a directory stream
 * @param d             the directory stream to close
 */
int NIFFS_closedir(niffs_DIR *d);

/**
 * Reads a directory into given niffs_dirent struct.
 * @param d             pointer to the directory stream
 * @param e             the dirent struct to be populated
 * @returns null if error or end of stream, else given dirent is returned
 */
struct niffs_dirent *NIFFS_readdir(niffs_DIR *d, struct niffs_dirent *e);

/**
 * Unmounts the file system. All file handles will be flushed of any
 * cached writes and closed.
 * @param fs            the file system struct
 */
int NIFFS_unmount(niffs *fs);

/**
 * Formats the entire filesystem.
 * @param fs            the file system struct
 */
int NIFFS_format(niffs *fs);

/**
 * Runs a consistency check on given filesystem and mends any aborted operations.
 * @param fs            the file system struct
 */
int NIFFS_chk(niffs *fs);

#ifdef NIFFS_DUMP
/**
 * Prints out a visualization of the filesystem.
 * @param fs            the file system struct
 */
void NIFFS_dump(niffs *fs);
#endif

#endif /* NIFFS_H_ */
