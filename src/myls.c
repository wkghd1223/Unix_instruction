#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
/*
어떤 옵션을 넣어주었는 지 확인하는 구조체
옵션을 넣어주면 set된다.
서로 양립할 수 없는 옵션(서로 override되는 옵션)은 하나가 set되면 하나는 clear된다.
*/
struct opt {
	int a;
	int d;
	int f;
	int i;
	int m;
	int r;
}opt;

/*
인자로 들어 온 filename이 디렉토리인지 확인하는 함수
디렉토리라면 1을 리턴한다.
*/
int isDir(char* filename) {
	struct stat statbuf;
	if (stat(filename, &statbuf) != 0)
       return 0;
	return S_ISDIR(statbuf.st_mode);
}

/*
ls의 기본기능 및 옵션을 구현한 함수
어떤 옵션이 set되어있는 지에 따라 출력하는 범위 및 동작이 달라진다.
기본 동작 순서
1. 인자로 들어온 filename이 디렉토리 인지 검사한다.
	a.  디렉토리라면 opendir로 filename을 연다.
		옵션에 따라 readdir, scandir등의 함수로 파일 내용을 검색하여 출력한다.
	b.  디렉토리가 아니라면 opendir로 현재디렉토리 (".")를 연다.
		readdir로 현재 디렉토리의 파일을 읽으며 filename과 같은 파일이 있다면 출력한다.
*/
void printLists(char* filename) {
	
	struct stat st;
	DIR* dir;
	struct dirent* dent, ** namelist;
	int n = 0, i = 0;
	char* path;
	// if filename is a directory
	// 옵션 d 가 set된다면 opendir로 열지 않고 파일이 들어왔을 때 처럼 취급한다.
	if (((dir = opendir(filename)) != NULL) && !opt.d) { 
		// 옵션 f 가 set된다면 scandir로 정렬하며 파일을 읽지않고 그냥 파일을 읽는다.
		if (opt.f) { 
			while ((dent = readdir(dir)) != NULL) {
				sprintf(path, "%s/%s", filename, dent->d_name);
				stat(path, &st);
				if (opt.i) { printf("%d ", (int)st.st_ino); } // 옵션 i 가 set된다면 이름을 출력하기 전 inode값을 출력한다.

				printf("%s", dent->d_name); // 기본기능. 파일의 이름을 출력
				if (opt.m) { printf(","); } // 옵션 m 이 set된다면 출력되는 파일의 이름 사이에 , 를 출력한다.
				printf("\t"); // 기본기능. 파일 이름 사이에 탭 출력
			}
		}
		else {
			n = scandir(filename, &namelist, NULL, alphasort); // 기본기능. 파일을 알파벳 순서로 출력
			
			for (i = opt.r ? n-1 : 0 ; opt.r ? i > 0 : i < n; opt.r ? i-- :i++ ) {
				// 옵션 a 가 set된다면 .으로 시작하는(숨김파일)파일도 출력한다.
				if (namelist[i]->d_name[0] == '.' && !opt.a) { continue; } 
				else {
					sprintf(path, "%s/%s", filename, namelist[i]->d_name);
					stat(path, &st);
					if (opt.i) { printf("%d ", (int)st.st_ino); } // 옵션 i 가 set된다면 이름을 출력하기 전 inode값을 출력한다.

					printf("%s", namelist[i]->d_name);
					if (opt.m) { printf(","); }
					printf("\t");

				}
			}
			for (i = 0; i < n; i++) {
				free(namelist[i]);
			}
			free(namelist);
		}
		// closedir(dir);
	}
	// if filename is a file
	else {
		dir = opendir(".");
		while ((dent = readdir(dir)) != NULL) {
			if (!strcmp(dent->d_name, filename)) {
				sprintf(path, "%s/%s", filename, dent->d_name);
				stat(path, &st);
				if (opt.i) { printf("%d ", (int)st.st_ino); } // 옵션 i 가 set된다면 이름을 출력하기 전 inode값을 출력한다.
				printf("%s", dent->d_name);
				if (opt.m) { printf(","); } // 옵션 m 이 set된다면 출력되는 파일의 이름 사이에 , 를 출력한다.
				printf("\t");
			}
		}
		// closedir(dir);
	}
	
	// closedir(dir);
}
int main(int argc, char* argv[]) {

	int n, i, c;

	extern char* optarg;
	extern int optind;

	while ((c = getopt(argc, argv, "adfimr")) != -1) {
		switch (c) {
		case 'a': // do not ignore entries starting with .
			opt.a = 1;
			break;
		case 'd': // list directories themselves, not their  contents
			opt.d = 1;
			break;
		case 'f': // do not sort
			opt.f = 1;
			break;
		case 'i': // print the index number of each file 
			opt.i = 1;
			break;
		case 'm': // fill width with a comma separated list of entries
			opt.m = 1;
			break;
		case 'r': 
			opt.r = 1;
			break;
		default:
			break;
		}
	}
	
	if (argc == optind) { // 인자가 없다면 기본으로 현재 디렉토리를 argv에 넣어준다.
		argv[optind] = "."; 
		argc++;
	}  

	if (argc - optind <= 1) { // 인자가 1개일 때 
		printLists(argv[optind]);
		printf("\n");
	}
	/*
	인자가 2개 이상일 때 디렉토리가 있다면
	디렉토리 안 파일들이 출력되기 전 '디렉토리명 : ' 이 출력된다.
	*/
	else { 
		for (i = optind; i < argc; i++) {
			if (!isDir(argv[i]) || opt.d) {
				printLists(argv[i]);
				if (argc - i == 1) { printf("\n"); }
			}
		}

		for (i = optind; i < argc; i++) {
			if (isDir(argv[i]) && !opt.d) {
				if (argc - optind > 1) printf("%s:\n", argv[i]); // "디렉토리명 :"
				printLists(argv[i]);
				printf("\n");
			}
		}
	}

	return 0;
}
