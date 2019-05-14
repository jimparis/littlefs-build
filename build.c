#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <err.h>
#include "lfs.h"

#define BLOCK_SIZE  4096
#define BLOCK_COUNT 4096

uint8_t flash[BLOCK_SIZE * BLOCK_COUNT];
uint8_t tmp_buf[BLOCK_SIZE * BLOCK_COUNT];

int lfs_api_read(const struct lfs_config *c, lfs_block_t block,
                 lfs_off_t off, void *buffer, lfs_size_t size)
{
        memcpy(buffer, &flash[block * c->block_size + off], size);
        return 0;
}

int lfs_api_prog(const struct lfs_config *c, lfs_block_t block,
                 lfs_off_t off, const void *buffer, lfs_size_t size)
{
        memcpy(&flash[block * c->block_size + off], buffer, size);
        return 0;
}

int lfs_api_erase(const struct lfs_config *c, lfs_block_t block)
{
        memset(&flash[block * c->block_size], 0xff, c->block_size);
        return 0;
}

int lfs_api_sync(const struct lfs_config *c)
{
        return 0;
}

const struct lfs_config cfg = {
        // block device operations
        .read  = lfs_api_read,
        .prog  = lfs_api_prog,
        .erase = lfs_api_erase,
        .sync  = lfs_api_sync,

        // block device configuration
        .read_size = 256,
        .prog_size = 256,
        .block_size = BLOCK_SIZE,
        .block_count = BLOCK_COUNT,
        .cache_size = 4096,
        .lookahead_size = 256,
};

lfs_t lfs;
lfs_file_t file;

int main(int argc, char **argv)
{
        memset(flash, 0xff, sizeof(flash));

        /* create image */
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);

        int i, len;
        FILE *in;

        for (i = 1; i < argc; i++)
        {
                if ((in = fopen(argv[i], "rb")) == NULL)
                        err(1, "can't open %s", argv[i]);
                len = fread(tmp_buf, 1, sizeof(tmp_buf), in);
                lfs_file_open(&lfs, &file, argv[i], LFS_O_RDWR | LFS_O_CREAT);
                lfs_file_write(&lfs, &file, tmp_buf, len);
                lfs_file_close(&lfs, &file);
                fclose(in);
                printf("wrote %s\n", argv[i]);
        }

        lfs_unmount(&lfs);

        /* write image to disk */
        FILE *out = fopen("flash.img", "wb");
        fwrite(flash, 1, sizeof(flash), out);
        fclose(out);

        return 0;
}

