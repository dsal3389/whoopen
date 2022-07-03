#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


#define LOG_ERROR(op, msg) "error:" #op ": " #msg "\n"
#define LOG_ERRNO(op) LOG_ERROR(op, "%s"), strerror(errno)


struct process_info{
	char * path;
	char * pid;
};

struct process_info _last_find;


void fatal(char *fmt, ...){
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(EXIT_FAILURE);
}

int isnumber(const char *str){
	const char *c = str;

	while(*c != 0){
		if(!(*c >= '0' && *c <= '9')) // check if char in range of ascii 0-9
			return 0;
		c++;
	}
	return 1;
}

int is_supported_path(char *path){
	struct stat path_stat;

	if(stat(path, &path_stat) != 0)
		fatal(LOG_ERRNO("stat"));

	return (
		path_stat.st_mode & S_IFREG ||
		path_stat.st_mode & S_IFSOCK ||
		path_stat.st_mode & S_IFBLK
	);
}

/* 
 * checks if the given process opened the given filepath, if yes return a string to the process filedescriptor path,
 * if not, return NULL
 */
char *process_opened(const char * pid, const char *path){
	static char proc_fd_dir_path[256];
	char resolved_link_path[PATH_MAX + 1];
	size_t resolved_path_size;
	ssize_t path_size;

	DIR *fddir;
	struct dirent *entry;

	path_size = snprintf(proc_fd_dir_path, sizeof(proc_fd_dir_path) - 1, "/proc/%s/fd/", pid);
	proc_fd_dir_path[path_size] = 0;

	fddir = opendir(proc_fd_dir_path);
	if(fddir == NULL)
		return NULL;
	
	while((entry=readdir(fddir)) != NULL){
		if(entry -> d_name[0] == '.') // there should'nt be any files starts with . accept . .. which will be skipped 
			continue;

		// this concat will take the process fd folder and appened to it the current fd entry, for example
		// /proc/1/fd/ + 23 = /proc/1/fd/23
		strncpy(&proc_fd_dir_path[path_size], entry -> d_name, sizeof(proc_fd_dir_path) - 1 - path_size);
		resolved_path_size = readlink(proc_fd_dir_path, resolved_link_path, PATH_MAX);
		if(resolved_path_size == -1)
			continue;

		resolved_link_path[resolved_path_size] = 0;

		if(strncmp(path, resolved_link_path, PATH_MAX) == 0){
			return proc_fd_dir_path;
		}
	}

	return NULL;
}

struct process_info *proc_who_opened(const char *path){
	struct dirent * entry;
	static DIR *procdir;
	char * proc_path;

	if(procdir == NULL){ // if its not the inital function call
		procdir = opendir("/proc");
		if(procdir == NULL)
			fatal(LOG_ERRNO("opendir(/proc)"));
	}

	while((entry=readdir(procdir)) != NULL){
		if(!(entry -> d_type & DT_DIR && isnumber(entry -> d_name))){ // if entry type is not dir and dir name is not all numbers
			continue;
		}	

		proc_path = process_opened(entry -> d_name, path);
		if(proc_path == NULL){
			continue;
		}

		// those are safe because their memory is staticlly located	
		_last_find.pid = entry -> d_name;
		_last_find.path = proc_path;
		return &_last_find;
	}

	closedir(procdir);
	return NULL;
}

void run(char *path){
	char absolute_path_buffer[PATH_MAX + 1];
	char *absolute_path;
	struct process_info *proc;

	if(path[0] != '/'){
		absolute_path = realpath(path, absolute_path_buffer);

		if(absolute_path == NULL)
			fatal(LOG_ERRNO("realpath"));
	} else {
		absolute_path = path;
	}

	while((proc=proc_who_opened(absolute_path)) != NULL){
		printf("%s@%s\n", proc -> pid, proc -> path);
	}
}

int main(int argc, char *argv[]){
	if(argc < 2 || strcmp(argv[1], "--help") == 0)
		fatal(LOG_ERROR("usage", "%s <filepath>"), argv[0]);
	
	if(!is_supported_path(argv[1]))
		fatal(LOG_ERROR("is_supported_path", "given file is not supported file type"));

	run(argv[1]);
}

