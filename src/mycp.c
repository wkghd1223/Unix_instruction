#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

/*
어떤 옵션을 넣어주었는 지 확인하는 구조체
옵션을 넣어주면 set된다.
서로 양립할 수 없는 옵션(서로 override되는 옵션)은 하나가 set되면 하나는 clear된다.
*/
struct opt {
	int n;
	int i;
	int l;
	int s;
	int v;
	int b;
} opt;
/*
옵션 b를 위한 함수
인자로 받은 문자열 뒤에 '~' 문자를 넣어준다.
*/
char* addWaveString(char* line) {
	int n, i, j = 0;
	n = strlen(line);
	char* buf = (char*)calloc(n + 1, sizeof(char*)); // 새로운 문자열 변수를 할당한다. 문자열의 크기는 기존의 문자열보다 1개 더 큰 n+1만큼 증가시켜준다.

	strcpy(buf, line);
	for (i = n + 1; i > n-1; i--) {
		buf[i] = buf[i - 1];
	}
	buf[n] = '~'; // 문자열 마지막 부분에 '~'를 붙혀준다.

	return buf;
}

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
옵션 i를 쓰면 overwrite를 할 지 말지를 결정하게 된다.
y로 시작하는 단어를 쓰면 1, n로 시작하는 단어를 쓰면 0을 리턴한다.
*/
int tellMeYesOrNo(char* filename) {
	char* s;

	printf("mycp: overwrite '%s'?", filename);
	scanf("%s", s);
	if (s[0] == 'Y' || s[0] == 'y') {
		return 1;
	}
	else if (s[0] == 'N' || s[0] == 'n') {
		return 0;
	}
	return -1;
}
/*
파일이 존재하는 지 확인하는 함수이다.
존재하면 1, 아니면 0을 리턴한다.
*/
int isExistFile(char* filename) {
	FILE* fp;
	if ((fp = fopen(filename, "r")) != NULL) {
		return 1;
	}
	else {
		return 0;
	}
	return 0;
}
/*
기본기능인 파일을 복사하는 함수이다.
src를 dest로 복사한다.
*/
void copyFile(char* src, char* dest) {
	FILE* rfp, * wfp , *bOptionfp;
	DIR * dir ;
	char buf[256], *newDirectory, *oldDirectory;
	struct dirent *dent;
	/*
	옵션 l과 옵션 s는 서로 양립할 수없다.
	*/
	if (opt.l && opt.s) {
		printf("mycp: cannot make both hard and symbolic links");
		exit(1);
	}
	/*
	옵션 l이 set되어 있다면 copy 하는 대신 hard link를 만든다.
	*/
	else if (opt.l) {
		if (link(src, dest) == -1) {
			perror(dest);
			exit(1);
		}
	}
	/*
	옵션 s가 set되어 있다면 copy 하는 대신 symbolic link를 만든다.
	*/
	else if (opt.s) {
		if (symlink(src, dest) == -1) {
			perror(dest);
			exit(1);
		}
	}
	/*
	기본기능.
	2개의 파일 포인터로 각각 src와 dest의 파일을 열고
	src에서 fgets()로 읽은 값을 buf에 담아 dest에 쓴다.
	*/
	else {
		if ((rfp = fopen(src, "r")) == NULL) {
			perror(src);
			exit(1);
		}
		if ((wfp = fopen(dest, "w")) == NULL) {
			perror(dest);
			exit(1);
		}
		// 옵션 b가 set되어있다면 백업파일을 위한 파일포인터를 받는다.
		if (opt.b && ((bOptionfp = fopen(addWaveString(dest), "w")) == NULL)) {
			perror(addWaveString(dest));
			exit(1);
		}
		while (fgets(buf, sizeof(buf), rfp) != NULL) {
			fprintf(wfp, "%s", buf);
			if (opt.b) {
				//옵션 b가 set 되어있다면 백업파일에도 write를 한다.
				fprintf(bOptionfp, "%s", buf); 
			}
		}

		fclose(rfp);
		fclose(wfp);
		if (opt.b) { fclose(bOptionfp); }
	}
	
	/*
	옵션 v가 set 되어있다면 src -> dest인 과정을 출력한다.
	*/
	if (opt.v) {
		printf("'%s' -> '%s' ", src, dest);
		if (opt.b) { printf("(backup: '%s')", addWaveString(dest)); } // 옵션 b가 set되어있다면 backup파일도 표시해준다.
		printf("\n");
	}
}


void CopyFileWithOption(char* src, char* dest) {
	if (!isExistFile(dest)) {
		copyFile(src, dest);
	}
	else {
		if (opt.i) {
			if (tellMeYesOrNo(dest)) {
				copyFile(src, dest);
			}
		}
		else if (opt.n) {
			// do noting
		}
		else {
			copyFile(src, dest);
		}
	}
}
int main(int argc, char* argv[]) {
	char buf[256];
	DIR* dir;
	char path[256];
	int n, i, c;
	extern char* optarg;
	extern int optind;

	while ((c = getopt(argc, argv, "nilsvb")) != -1) {
		switch (c) {
		case 'n': // do not overwrite an existing file
			opt.n = 1;
			if (opt.i == 1) { opt.i = 0; }						// i가 set되어있다면 clear한다.
			break;
		case 'i':// prompt before overwrite
			opt.i = 1;
			if (opt.n == 1) { opt.n = 0; }						// n이 set되어있다면 clear한다.
			break;
		case 'l': // hard link files instead of copying
			opt.l = 1;
			break;
		case 's': // make symbolic links instead of copying
			opt.s = 1;
			break;
		case 'v': // explain what is being done
			opt.v = 1;
			break;
		case 'b': // like --backup but does not accept an argument
			opt.b = 1;
			break;
		default:
			break;
		}
	}

	/*
	3가지 경우로 나누어 코드를 작성하였다.
	1.인자가 2개일때
		case 1. mycp src dest	: 2번째 인자가 파일일 때
		case 2. mycp src Dir	: 2번째 인자가 디렉토리일 때
	2.인자가 3개 이상일 때
		case 3. mycp src1 src2 ... srcN dir

	*/
	if (argc - optind == 2) { // 1.인자가 2개일 때

		// src가 디렉토리라면 종료한다.
		if (isDir(argv[optind])) {
			printf("mycp: -r not specified; omitting directory '%s'\n", argv[optind]);
			exit(1);
		}
		// case 1
		if ((dir = opendir(argv[optind + 1])) == NULL) { // 2번째 인자가 디렉토리가 아닐 경우
			CopyFileWithOption(argv[optind], argv[optind + 1]);
		}
		// case 2 
		else {
			sprintf(path, "%s/%s", argv[optind + 1], argv[optind]);
			CopyFileWithOption(argv[optind], path);
		}
		closedir(dir);
	}
	// case 3
	else if (argc - optind > 2) {
		n = argc - 1;

		if ((dir = opendir(argv[n])) == NULL) {
			perror(argv[n]);
			exit(1);
		}
		// argv[1]은 -n
		for (i = optind; i < n; i++) {
			sprintf(path, "%s/%s", argv[n], argv[i]);
			// src가 디렉토리라면 종료한다.
			if (isDir(argv[i])) {
				printf("mycp: -r not specified; omitting directory '%s'\n", argv[i]);
				exit(1);
			}
			CopyFileWithOption(argv[optind], path);
		}
		closedir(dir);
	}
	else if (argc - optind == 1) { // 인자가 1개일 때
		printf("mycp: missing destination file operand after '%s'\n", argv[optind]);
	}
	else {
		printf("mycp: missing file operand\n");
	}
	return 0;
}
