#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <taglib/tag_c.h>

// compile with "gcc -Wall ytfs.c `pkg-config fuse --cflags --libs` -o ytfs -ltag_c"

struct decade_node* decade_head; 
struct decade_node* album_head; 

static const char *base_path = "/home/ubuntu/.ytfsmusic";



void get_filename(char* path,char* buf)
{
	int l = 0;
	buf = strstr(path, "/");
	do{
	    l = strlen(buf) + 1;
	    path = &path[strlen(path)-l+2];
	    buf = strstr(path, "/");
	}while(buf);
	printf("FILENAME: %s\n",buf);
}

void get_realpath(const char* path, char* buf)
{
	char *filename;
	char* tmp1;
	tmp1=strdup(path);
	filename=basename(tmp1);
	strcpy(buf,"/home/ubuntu/.ytfsmusic/");
	strcat(buf,filename);
	return;
}


void insert_mp3_2(const char* filename, char* album, char* decade){
        char realpath[256];
        get_realpath(filename,realpath);

	char tmpDecDir[256];
        strcpy(tmpDecDir,base_path);
        strcat(tmpDecDir,"/Decades/");
        strcat(tmpDecDir,decade);
        printf("DECADE: %s\n",decade);
        mkdir(tmpDecDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);  //only if found_decade is false;

        strcat(tmpDecDir,"/");
        strcat(tmpDecDir,album);
        printf("ALBUM: %s\n",album);
	mkdir(tmpDecDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); //only if found_album is false

        strcat(tmpDecDir,filename);
        symlink(realpath,tmpDecDir);

        char tmpAlbDir[256];
        strcpy(tmpAlbDir,base_path);
        strcat(tmpAlbDir,"/Albums/");
        strcat(tmpAlbDir,album);

        mkdir(tmpAlbDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);  //only if found_album is f
        strcat(tmpAlbDir,filename);
        symlink(realpath,tmpAlbDir);
}

static int ytfs_open(const char *path, struct fuse_file_info *fi)
{
	int res;

	char realpath[strlen(path)+25];
	get_realpath(path,realpath);
	
	res = open(realpath, fi->flags);

	if (res == -1)
		return -errno;

	close(res);
	return 0;
}



static int ytfs_getattr(const char *path, struct stat *stbuf)
{
	//THIS SECTION WILL NEED TO BE IMPROVED.  A LOT
	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));
	printf("PATH: %s\n",path);
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else {
		char realpath[strlen(path)+strlen(base_path)];
		strcpy(realpath,base_path);
		strcat(realpath,path);
		res = stat(realpath, stbuf);
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
	}else {
		DIR *dp;
		struct dirent *de;

		(void) offset;
		(void) fi;


		char realpath[strlen(path)+strlen(base_path)];
		strcpy(realpath,base_path);
		strcat(realpath,path);
		printf("REALPATH: %s\n",realpath);

		dp = opendir(realpath);
		if (dp == NULL)
		return -errno;

		while ((de = readdir(dp)) != NULL) {
			struct stat st;
			memset(&st, 0, sizeof(st));
			st.st_ino = de->d_ino;
			st.st_mode = de->d_type << 12;
			if (filler(buf, de->d_name, &st, 0))
				break;
		}
		closedir(dp);
	}
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

static int ytfs_readlink(const char *path, char *buf, size_t size)
{
	printf("%s\n",path);
	
	int res;

	char realpath[strlen(path)+25];
	get_realpath(path,realpath);

	res = readlink(realpath, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	
	return 0;
}

static int ytfs_flush(const char *path, struct fuse_file_info *fi)
{
  char realpath[256];
  get_realpath(path,realpath);

  TagLib_File *file;
  file=taglib_file_new(realpath);
  TagLib_Tag *tag;
  tag = taglib_file_tag(file);

  int year_int=taglib_tag_year(tag);
  int dec_int=year_int-(year_int%10);
  char decade[4];
  sprintf(decade,"%d",dec_int);
  insert_mp3_2(path,taglib_tag_album(tag),decade);
  taglib_tag_free_strings();
  taglib_file_free(file);

  //send file to remote server
  char curl_command[256];
  sprintf(curl_command,"curl -X POST -F file=@\"%s\" http://127.0.0.1:8000",realpath);
  system(curl_command);

  return 0;
}

static struct fuse_operations ytfs_oper = {
	.getattr	= ytfs_getattr,
	.readdir	= ytfs_readdir,
	.open		= ytfs_open,
	.read		= ytfs_read,
	.chmod		= ytfs_chmod,
	.write		= ytfs_write,
	.mknod		= ytfs_mknod,
	.readlink 	= ytfs_readlink,
	.flush          = ytfs_flush
	
};

int main(int argc, char *argv[])
{
	umask(0);
	mkdir("/home/ubuntu/.ytfsmusic/",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("/home/ubuntu/.ytfsmusic/Decades",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("/home/ubuntu/.ytfsmusic/Albums",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	return fuse_main(argc, argv, &ytfs_oper, NULL);
}
