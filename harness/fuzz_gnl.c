/* ************************************************************************** */
/*  fuzz_gnl.c : get_next_line 정확성 퍼징 하네스 (libFuzzer / AFL++ 공용)    */
/*  오라클: 반환된 줄을 이어 붙이면 입력과 정확히 같아야 한다                 */
/* ************************************************************************** */

#include <stdint.h>                          /* uint8_t */
#include <stdlib.h>                          /* free, abort */
#include <string.h>                          /* strlen, memcmp, memchr */
#include <unistd.h>                          /* pipe, write, close */
#include "get_next_line.h"                   /* 테스트 대상: get_next_line */

#define MAX_INPUT 4096                       /* 파이프 버퍼(최소 4KB)를 넘지 않게 제한 */

typedef struct s_ctx                         /* 한 입력에 대한 검사 상태 */
{
	const uint8_t	*data;                   /* 원본 입력 */
	size_t			size;                    /* 입력 길이 */
	size_t			off;                     /* 지금까지 맞춰 본 위치 */
	int				last_no_nl;              /* 직전 줄이 '\n' 없이 끝났는지 */
}	t_ctx;

static void	fail(const char *msg)
{
	if (write(2, msg, strlen(msg)) < 0)      /* 어떤 오라클이 깨졌는지 stderr에 출력 */
		abort();                             /* 출력 실패해도 크래시로 보고 */
	abort();                                 /* 크래시 → 퍼저가 입력을 저장 */
}

static int	feed_pipe(const uint8_t *data, size_t size)
{
	int		fds[2];                          /* [0]=읽기, [1]=쓰기 */
	size_t	done;                            /* 지금까지 쓴 바이트 수 */
	ssize_t	w;                               /* 이번 write 결과 */

	if (pipe(fds) == -1)                     /* 파이프 생성 실패 */
		return (-1);                         /* 이 입력은 건너뜀 */
	done = 0;                                /* 쓰기 위치 초기화 */
	while (done < size)                      /* 부분 쓰기 대비 반복 */
	{
		w = write(fds[1], data + done, size - done);  /* 남은 만큼 쓰기 */
		if (w <= 0)                          /* 쓰기 실패 */
		{
			close(fds[0]);                   /* 읽기 끝 정리 */
			close(fds[1]);                   /* 쓰기 끝 정리 */
			return (-1);                     /* 건너뜀 */
		}
		done += (size_t)w;                   /* w > 0 이므로 size_t 변환 안전 */
	}
	close(fds[1]);                           /* EOF를 만들기 위해 쓰기 끝 닫기 */
	return (fds[0]);                         /* GNL이 읽을 fd 반환 */
}

static void	check_line(t_ctx *c, const char *line)
{
	size_t	len;                             /* 반환된 줄 길이 */

	len = strlen(line);                      /* 입력에 '\0' 없으니 strlen 안전 */
	if (len == 0)                            /* 빈 문자열 반환은 명세 위반 */
		fail("[ORACLE] empty line returned\n");
	if (c->last_no_nl)                       /* '\n' 없는 줄 뒤에 또 줄이 옴 */
		fail("[ORACLE] line after unterminated line\n");
	if (len > c->size - c->off)              /* 오버플로 가드: 입력보다 많이 반환 */
		fail("[ORACLE] returned more bytes than input\n");
	if (memcmp(line, c->data + c->off, len) != 0)  /* 내용이 입력과 다름 */
		fail("[ORACLE] content mismatch\n");
	if (memchr(line, '\n', len - 1) != NULL) /* 줄 중간에 '\n' (len>=1 보장) */
		fail("[ORACLE] newline inside line\n");
	if (line[len - 1] != '\n')               /* 마지막 글자가 '\n'이 아님 */
		c->last_no_nl = 1;                   /* 그렇다면 이게 마지막 줄이어야 함 */
	c->off += len;                           /* 맞춰 본 위치 전진 */
}

static void	drain_fd(t_ctx *c, int fd)
{
	char	*line;                           /* GNL 반환값 */

	line = get_next_line(fd);                /* 첫 줄 요청 */
	while (line != NULL)                     /* NULL이 올 때까지 */
	{
		check_line(c, line);                 /* 오라클 검사 */
		free(line);                          /* 누수 방지 (LSan이 감시) */
		line = get_next_line(fd);            /* 다음 줄 */
	}
	if (c->off != c->size)                   /* 입력을 끝까지 돌려주지 않음 */
		fail("[ORACLE] input not fully returned\n");
	line = get_next_line(fd);                /* EOF 이후 재호출 */
	if (line != NULL)                        /* EOF 뒤에도 데이터가 나옴 */
		fail("[ORACLE] data after EOF\n");
}

int	LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	t_ctx	c;                               /* 검사 상태 */
	int		fd;                              /* 파이프 읽기 끝 */

	if (size > MAX_INPUT)                    /* 파이프가 막히지 않게 크기 제한 */
		return (0);
	if (memchr(data, 0, size) != NULL)       /* '\0' 포함 입력은 GNL 범위 밖 */
		return (0);
	fd = feed_pipe(data, size);              /* 입력을 파이프에 넣기 */
	if (fd < 0)                              /* 준비 실패 */
		return (0);
	c.data = data;                           /* 원본 기억 */
	c.size = size;                           /* 길이 기억 */
	c.off = 0;                               /* 위치 초기화 */
	c.last_no_nl = 0;                        /* 플래그 초기화 */
	drain_fd(&c, fd);                        /* 끝까지 읽으며 검사 */
	close(fd);                               /* fd 정리 */
	return (0);                              /* libFuzzer 규약: 항상 0 */
}
