#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <fcntl.h>
#include <lzma.h>

#define kBufferSize (1 << 15)

typedef struct lzfile {
	uint8_t buf[kBufferSize];
	lzma_stream strm;
	FILE *file;
	bool encoding;
	bool eof;
} LZFILE;

int lzflush(LZFILE *lzfile)
{
    return fflush(lzfile->file);
}

int lzclose(LZFILE *lzfile)
{
    lzma_ret ret;
    int n;

    if (!lzfile)
		return -1;
    if (lzfile->encoding) {
		for (;;) {
			lzfile->strm.avail_out = kBufferSize;
			lzfile->strm.next_out = lzfile->buf;
			ret = lzma_code(&lzfile->strm, LZMA_FINISH);
			if (ret != LZMA_OK && ret != LZMA_STREAM_END)
				return -1;
			n = kBufferSize - lzfile->strm.avail_out;
			if (n && fwrite(lzfile->buf, 1, n, lzfile->file) != n)
				return -1;
			if (ret == LZMA_STREAM_END)
				break;
		}
	}
	lzma_end(&lzfile->strm);
	return fclose(lzfile->file);
    free(lzfile);
}

LZFILE *lzopen(const char *path, const char *mode, int fd)
{
    int level = 5;
    int encoding = 0;
    FILE *fp;
    LZFILE *lzfile;
    lzma_ret ret;

    for (; *mode; mode++) {
		if (*mode == 'w')
			encoding = 1;
		else if (*mode == 'r')
			encoding = 0;
		else if (*mode >= '1' && *mode <= '9')
			level = *mode - '0';
	}
    if (fd != -1)
		fp = fdopen(fd, encoding ? "w" : "r");
    else
		fp = fopen(path, encoding ? "w" : "r");
    if (!fp)
		return 0;
    lzfile = calloc(1, sizeof(*lzfile));
    if (!lzfile) {
		fclose(fp);
		return 0;
    }

    lzfile->file = fp;
    lzfile->encoding = encoding;
    lzfile->eof = 0;
    lzfile->strm = (lzma_stream)LZMA_STREAM_INIT;
    if (encoding) {
	ret = lzma_easy_encoder(&lzfile->strm, LZMA_PRESET_DEFAULT, LZMA_CHECK_CRC32);
    } else {
	ret = lzma_auto_decoder(&lzfile->strm, 0, 0);
    }
    if (ret != LZMA_OK) {
		fclose(fp);
		free(lzfile);
		return 0;
	}
    return lzfile;
}

ssize_t lzread(LZFILE *lzfile, void *buf, size_t len)
{
    lzma_ret ret;
    int eof = 0;

    if (!lzfile || lzfile->encoding)
		return -1;
    if (lzfile->eof)
		return 0;

    lzfile->strm.next_out = buf;
    lzfile->strm.avail_out = len;
    for (;;) {
		if (!lzfile->strm.avail_in) {
			lzfile->strm.next_in = lzfile->buf;
			lzfile->strm.avail_in = fread(lzfile->buf, sizeof(uint8_t), kBufferSize, lzfile->file);
			if (!lzfile->strm.avail_in)
				eof = 1;
		}
		ret = lzma_code(&lzfile->strm, LZMA_RUN);
		if (ret == LZMA_STREAM_END) {
			lzfile->eof = 1;
			return len - lzfile->strm.avail_out;
		}
		if (ret != LZMA_OK){
			return -1;
		}
		if (!lzfile->strm.avail_out)
		{
			return len;
		}
		if (eof)
			return -1;
      }
}

ssize_t lzwrite(LZFILE *lzfile, void *buf, size_t len)
{
    lzma_ret ret;
    int n;
    if (!lzfile || !lzfile->encoding)
		return -1;
    if (!len)
		return 0;
	lzfile->strm.next_in = buf;
    lzfile->strm.avail_in = len;
    for (;;) {
		lzfile->strm.next_out = lzfile->buf;
		lzfile->strm.avail_out = kBufferSize;
		ret = lzma_code(&lzfile->strm, LZMA_RUN);
		if (ret != LZMA_OK)
			return -1;
	n = kBufferSize - lzfile->strm.avail_out;
	if (n && fwrite(lzfile->buf, 1, n, lzfile->file) != n)
	    return -1;
	if (!lzfile->strm.avail_in)
	    return len;
    }
}

int main(int argc, char * argv[])
{
	if (argc < 2 || 0 == strcmp (argv[1], "--help")) {
		printf ("Usage: %s <filename.xz>\n\n", argv[0]);
		return 0;
	}

	ssize_t size = 0;
	unsigned char *buf = malloc (1024);
	size_t count;
	int len;
//	int fd = open("test.lzma", O_RDONLY);
//	LZFILE* lzin = lzopen(argv[1], "r", -1);
/*	LZFILE* lzin = lzopen("", "r", fd);	
	size = lzread(lzin, buf, len);
	printf("%s\n", buf);
	printf("count: %d\n", count);
	lzclose(lzin);*/
	buf = "heisann hoppsann fallerallera, hoppetitoppetihippetihei, tudelidudelididdelidoooo";
	len = strlen(buf);
	LZFILE* lzout = lzopen("xzout.xz", "w", -1);
	lzwrite(lzout, buf, len);
	lzclose(lzout);
}

