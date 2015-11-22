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

struct mp3_node {
	struct mp3_node *next;
	char album[30];
	char decade[4];
	char filename[30];
	//point to actual music data here.  this will likely include the actual file name
};

struct album_node {
	struct album_node *next;
	char decade[4];
	struct mp3_node *music;
	char album_name[30];
	//permissions data and other data we may want to associate with the album
};

struct decade_node {
	char decade[4];
	struct decade_node *next;
	struct album_node *albums;
};

struct decade_node* decade_head; 
struct decade_node* album_head; 

static const char *base_path = "/home/william/.ytfsmusic/";



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
	//ensure basename is 
	strcpy(buf,"/home/william/.ytfsmusic/");
	strcat(buf,filename);/*
	printf("PATH: %s\n",path);
	printf("FILENAME: %s\n",tmp1);
	
	printf("REALPATH: %s\n",buf);*/
	return;
}

void insert_mp3(struct album_node *head, struct mp3_node *new_node){
	struct decade_node *tmp_decade=decade_head;
	struct album_node *tmp_album;
	int found_decade=0;
	int found_album=0;
	//find decade node to insert into.  store that in tmp_decade
	while(tmp_decade != NULL){ //if at least one decade exists
		if(tmp_decade->decade == new_node->decade){
			found_decade=1;
			break;
		}
		tmp_decade=tmp_decade->next;
	}
	
	if (found_decade){
		tmp_album=tmp_decade->albums;
		
		while(tmp_album != NULL){ //if at least one decade exists
			if(strcmp(tmp_album->album_name,new_node->album)==0){
				found_decade=1;
				break;
			}
			if (strcmp(tmp_album->decade,new_node->decade) != 0){ //is this still necessary?
				break;
			}
			tmp_album=tmp_album->next;
		}
		
		//figure out if this is a new album.  if not, store album node in tmp_album
		if (found_album){
			new_node->next=tmp_album->music;
			tmp_album->music=new_node;
		} /*else {
			album_node* cur_al_head=tmp_decade->albums;
			//TODO: fix this
			new_node->next=cur_al_head->next;
			cur_al_head->next=new_node;
		}*/
	} else { //if this is the first song in this decade
		//insert decade
		tmp_decade=malloc(sizeof(struct decade_node));
		tmp_album=malloc(sizeof(struct album_node));
		tmp_decade->albums=tmp_album;
		tmp_decade->next=decade_head;
		decade_head=tmp_decade;
		//define album properties
		strcpy(tmp_album->album_name,new_node->album);
		tmp_album->music=new_node;
		strcpy(tmp_album->decade,new_node->decade);
		
		//define decade properties
		strcpy(tmp_decade->decade,new_node->decade);
	}
	char realpath[256];
	get_realpath(new_node->filename,realpath);

	//create directory in storage here
	char tmpDecDir[256];
	strcpy(tmpDecDir,base_path);
	strcat(tmpDecDir,"Decades/");
	strcat(tmpDecDir,new_node->decade);
	if (found_decade == 0){
		mkdir(tmpDecDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);  //only if found_decade is false;
	}
	strcat(tmpDecDir,"/");
	strcat(tmpDecDir,new_node->album);
	if (found_album == 0){
		mkdir(tmpDecDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); //only if found_album is false
	}
	strcat(tmpDecDir,new_node->filename);
	symlink(realpath,tmpDecDir);

	char tmpAlbDir[256];
	strcpy(tmpAlbDir,base_path);
	strcat(tmpAlbDir,"Albums/");
	strcat(tmpAlbDir,new_node->album);
	if (found_album == 0){
		mkdir(tmpAlbDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);  //only if found_album is false
	}
	strcat(tmpAlbDir,new_node->filename);
	symlink(realpath,tmpAlbDir);
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
	//THIS SECTION WILL NEED TO BE IMPROVED.  A LOT
	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));
	printf("PATH: %s\n",path);
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	/*} else if ((strcmp(path,"/Decades")==0) | (strcmp(path,"/Albums")==0)){ //realistically, we will want to do this any time the target a directory and not a file
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;*/
	} else {
		char realpath[strlen(path)+25];
		get_realpath(path,realpath);
		printf("REALPATH: %s\n",realpath);
		//strcpy(realpath,base_path);
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
	//check here whether or not file already exists in metadata structures.  can be done by checking the path (if basename=/,is new file)
	//run this if the file does not actually exist
		//deter
		//add file to metadata structures
		//add directories to hidden file, 
		char tmp2[256];
		strcpy(tmp2,base_path);
		strcat(tmp2,"Decades");
		strcat(tmp2,path);
		printf("SQEE: %s\n",tmp2);
		symlink(realpath,tmp2);

		
		TagLib_File *file;
		file=taglib_file_new(realpath);
		TagLib_Tag *tag;
		tag = taglib_file_tag(file);

		struct mp3_node *data_node=malloc(sizeof(struct mp3_node));
		strcpy(data_node->filename,path);
		strcpy(data_node->album,taglib_tag_album(tag));
		int year_int=taglib_tag_year(tag);
		int dec_int=year_int-(year_int%10);
		sprintf(data_node->decade,"%d",dec_int);
		
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
	/* } else if (strcmp(base,"Decades")==0){//ELSE IF IN DECADES
		decade_node *tmp_decade=decade_head;
		while (tmp_decade != NULL){
			filler(buf,tmp_decade->decade,NULL,0);
		}
		filler(buf,"2010",NULL,0);
		//LIST DECADES
	}else if (strcmp(base,"Albums")==0){//ELSE IF IN ALBUMS (./Albums)
		album_node *tmp_album=album_head;
		while (tmp_album != NULL){
			filler(buf,tmp_album->album,NULL,0);
		}
		//LIST ALL ALBUMS
	} else if (strcmp(dir,"Decades")==0){//ELSE IF IN SUBDIR OF DECADES (./DECADE/{DECADE})
		//LIST APPROPRIATE ALBUMS
		//this will be a function which uses "base" as an input
	} else if (strcmp(dir,"Albums")==0){ // (./Albums/{album})*/
	
	}else {//ELSE (./DECADE/{DECADE}/{Album}) OR (./{music})
		//LIST APPROPRIATE MP3S
		DIR *dp;
		struct dirent *de;

		(void) offset;
		(void) fi;

		char realpath[strlen(path)+25];
		get_realpath(path,realpath);
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

static struct fuse_operations ytfs_oper = {
	.getattr	= ytfs_getattr,
	.readdir	= ytfs_readdir,
	.open		= ytfs_open,
	.read		= ytfs_read,
	.chmod		= ytfs_chmod,
	.write		= ytfs_write,
	.mknod		= ytfs_mknod,
	.readlink 	= ytfs_readlink
};

int main(int argc, char *argv[])
{
	umask(0);
	mkdir("/home/william/.ytfsmusic/",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("/home/william/.ytfsmusic/Decades",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("/home/william/.ytfsmusic/Albums",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	return fuse_main(argc, argv, &ytfs_oper, NULL);
}
