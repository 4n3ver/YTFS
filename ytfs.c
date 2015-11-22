#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <unistd.h>

void get_filename(char* path,char* buf)
{
	int l = 0;
	buf = strstr(path, "/");
	do{
	    l = strlen(buf) + 1;
	    path = &path[strlen(path)-l+2];
	    buf = strstr(path, "/");
	}while(buf);
}

void get_realpath(const char* path, char* buf)
{
	char *filename;
	char* tmp1;
	tmp1=strdup(path);
	filename=basename(tmp1);
	strcpy(buf,"/home/william/.ytfsmusic/");
	strcat(buf,filename);
	
	printf("REALPATH: %s\n",buf);
	return;
}

static int ytfs_open(const char *path, struct fuse_file_info *fi)
{
	int res;

	printf("######HI WILLIAM#####\n");
	char realpath[strlen(path)+25];
	get_realpath(path,realpath);
	
	res = open(realpath, fi->flags);

	printf("######BYE WILLIAM#####\n");
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}



static int ytfs_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if ((strcmp(path,"/Decades")==0) | (strcmp(path,"/Albums")==0)){
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else {
		char realpath[strlen(path)+25];
		get_realpath(path,realpath);

		res = lstat(realpath, stbuf);
		if (res == -1)
			return -errno;
	}
	return res;
}

static int ytfs_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;


	char realpath[strlen(path)+25];
	get_realpath(path,realpath);

	fd = open(realpath, O_WRONLY);
	if (fd == -1)
		return -errno;
	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;
	//check here whether or not file already exists in metadata structures.  can be done by checking the path, and whether or not 
	//run this if the file does not actually exist
		//deter
		//add file to metadata structures
	close(fd);
	return res;
}

static int ytfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	
	//char* tmp1;
	char* tmp2;
	//char *dir;
	char *base;
	//tmp1=strdup(path);
	tmp2=strdup(path);
	//dir=dirname(tmp1);
	base=basename(tmp2);
	if (strcmp(base,".")==0 || strcmp(base,"/")==0){//IF ON TOP LEVEL
		filler(buf,"Decades",NULL,0);
		filler(buf,"Albums",NULL,0);
		//LIST "ALBUMS" and "DECADES"
	} /*else if (strcmp(base,"Decades")==0){//ELSE IF IN DECADES
		decade_node *tmp_decade=decade_head;
		while (tmp_decade != NULL){
			filler(buf,tmp_decade->decade,NULL,0);
		}
		//LIST DECADES
	} else if (strcmp(base,"Albums")==0){//ELSE IF IN ALBUMS (./Albums)
		album_node *tmp_album=album_head;
		while (tmp_album != NULL){
			filler(buf,tmp_album->album,NULL,0);
		}
		//LIST ALL ALBUMS
	} else if (strcmp(dir,"Decades")==0){//ELSE IF IN SUBDIR OF DECADES (./DECADE/{DECADE})
		//LIST APPROPRIATE ALBUMS
		//this will be a function which uses "base" as an input
	} else if (strcmp(dir,"Albums")==0){ // (./Albums/{album})

	}else {//ELSE (./DECADE/{DECADE}/{Album}) OR (./{music})
		//LIST APPROPRIATE MP3S
	}*/
	return 0;
}

static int ytfs_chmod(const char *path, mode_t mode)
{
	int res;

	char realpath[strlen(path)+25];
	get_realpath(path,realpath);

	res = chmod(realpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int ytfs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
	char realpath[strlen(path)+25];
	get_realpath(path,realpath);
	res = mknod(realpath, mode, rdev);
	if (res == -1)
		return -errno;
	return 0;
}

static int ytfs_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;

	char realpath[strlen(path)+25];
	get_realpath(path,realpath);

	fd = open(realpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static struct fuse_operations ytfs_oper = {
	.getattr	= ytfs_getattr,
	.readdir	= ytfs_readdir,
	.open		= ytfs_open,
	.read		= ytfs_read,
	.chmod		= ytfs_chmod,
	.write		= ytfs_write,
	.mknod		= ytfs_mknod
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &ytfs_oper, NULL);
}
