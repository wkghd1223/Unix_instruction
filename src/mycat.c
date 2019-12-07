#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
/*
어떤 옵션을 넣어주었는 지 확인하는 구조체
옵션을 넣어주면 set된다.
서로 양립할 수 없는 옵션(서로 override되는 옵션)은 하나가 set되면 하나는 clear된다.
*/
struct opt {
	int b;
	int n;
	int E;
	int v;
	int T;
};

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
옵션 T를 구현한 함수.
문자열을 받아 모든 탭을 ^I문자로 바꾸어준 후 문자열을 리턴한다.
*/
char* OptionT(char* line) {
	int n = strlen(line);
	int i, j;
	char* buf;
	for (i = 0; i < n; i++) {
		if (line[i] == '\t') {
			buf = (char*)calloc(n + 1, sizeof(char)); // 새로운 문자열 변수를 할당한다. 문자열의 크기는 기존의 문자열보다 1개 더 큰 n+1만큼 증가시켜준다.
			strcpy(buf, line);

			for (j = n; j > i + 1; j--) {
				buf[j] = buf[j - 1];
			}
			buf[i] = '^';
			buf[i + 1] = 'I';

			n++;
			line = buf;
		}
	}
	return line;
}
/*
옵션 E를 구현한 함수
문자열 마지막에 $를 붙히고 리턴한다..
*/
char* OptionE(char* line) {
	int n, i, j = 0;
	n = strlen(line);
	char* buf = (char*)calloc(n + 1, sizeof(char*)); // 새로운 문자열 변수를 할당한다. 문자열의 크기는 기존의 문자열보다 1개 더 큰 n+1만큼 증가시켜준다.

	for (i = 0; i < n; i++) {
		if (line[i] == '\n') {
			j = i;
		}
	}
	if (j == 0) { return line; }

	strcpy(buf, line);
	for (i = n + 1; i > j; i--) {
		buf[i] = buf[i - 1];
	}
	buf[n - 1] = '$'; // 문자열 마지막 부분에 '$'를 붙혀준다.

	return buf;
}
/*
파일포인터로 파일을 열고 fgets로 버퍼에 한줄씩 담아 출력한다.
*/
int main(int argc, char* argv[]) {
	FILE* fp;
	char buf[256];
	char* line = (char*)calloc(256, sizeof(char));
	int c;
	int i = 1, lineNum;
	struct opt opt = {
		.b = 0, .E = 0,.n = 0, .v = 0 , .T = 0
	};
	extern char* optarg;
	extern int optind;
	// v는 구현 못함
	while ((c = getopt(argc, argv, "AbEnetTv")) != -1) {
		switch (c) {
		case 'A': // equivalent to -vET
			opt.v = 1;
			opt.E = 1;
			opt.T = 1;
			break;
		case 'b': // number nonempty output lines
			opt.b = 1;
			if (opt.n) { opt.n = 0; }
			break;
		case 'E': // display $ at end of each line
			opt.E = 1;
			break;
		case 'e': // equivalent to -vE
			opt.v = 1;
			opt.E = 1;
			break;
		case 'n': // number all output lines
			opt.n = 1;
			if (opt.b) { opt.b = 0; }
			break;
		case 't': // equivalent to -vT
			opt.v = 1;
			opt.T = 1;
			break;
		case 'T': // display TAB characters as ^I
			opt.T = 1;
			break;
		case 'v': // use ^ and  M- notation, except for LFD a TAB
			opt.v = 1;
			break;
		default:
			break;
		}
	}

	// 인자가 없을때 함수를 종료한다.
	if (argc == optind) {
		return 0;
	}
	// 모든 인자에 대해 수행한다.
	for (i = optind; i < argc; i++) {
		
		// argv가 디렉토리라면 다음 인자로 넘어간다.
		if (isDir(argv[i])) {
			printf("mycat: %s: Is a directory\n", argv[i]);
			continue;
		}
		if ((fp = fopen(argv[i], "r")) == NULL) {
			perror(argv[i]);
			exit(1);
		}

		lineNum = 1;
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			strcpy(line, buf);
			// 옵션 n과 b는 양립할 수 없기에 if - else if 로 묶어주었다.
			// 옵션 n이 set되어있다면 각 줄마다 lineNum을 증가시켜주고 각 line앞에 lineNum을 추가하여 출력한다.
			if (opt.n) {
				sprintf(line, "    %d  %s", lineNum, buf);
				lineNum++;
			}
			// 옵션 b가 set되어있다면 공백이 아닌 각 줄마다 lineNum을 증가시켜주고 각 line앞에 lineNum을 추가하여 출력한다.
			else if (opt.b) {
				if (strcmp(buf, "\n")) {
					sprintf(line, "    %d  %s", lineNum, buf);
					lineNum++;
				}
			}

			// 옵션 E가 set되어있다면 line변수에 OptionE()를 사용하여 새로운 문자열을 담는다.
			if (opt.E) {
				line = OptionE(line);
			}
			// 옵션 T가 set되어있다면 line변수에 OptionT()를 사용하여 새로운 문자열을 담는다.
			if (opt.T) {
				line = OptionT(line);
			}
			printf("%s", line);
		}
	}
	fclose(fp);
	return 0;
}
