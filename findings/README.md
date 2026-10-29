# 발견한 버그 (targets/v1_2025-08-14)

2025년 8월 14일 버전의 get_next_line에서 퍼저가 찾은 버그 2개입니다. 하나를 고친 사본으로 다시 퍼징해 다음 버그를 찾는 방식으로 진행했습니다.

## Bug 1. EOF 이후 재호출 시 use-after-free

| 항목 | 내용 |
|---|---|
| 검출 | AddressSanitizer: heap-use-after-free (`ft_strchr` ← `read_until_nl` ← `get_next_line`) |
| 재현 입력 | 빈 입력 (`bug1_uaf_empty_input.bin`, 0바이트) |
| 발견까지 | libFuzzer, BUFFER_SIZE=42, 첫 입력에서 즉시 |
| 트리거 | EOF로 `NULL`을 반환한 뒤 `get_next_line`을 한 번 더 호출 (오라클 #5 검사 단계) |

**원인**: EOF 경로에서 `free(stash)` 후 `stash`를 `NULL`로 되돌리지 않았습니다. static 포인터가 해제된 메모리를 계속 가리키므로, 다음 호출의 `if (!stash)` 초기화를 건너뛰고 해제된 버퍼를 `ft_strchr`로 읽습니다.

**수정**: `bug1_fix.diff` (`stash = NULL;` 한 줄)

**의미**: 일반적인 테스트는 `NULL`이 나오면 읽기를 멈추기 때문에 이 경로를 밟지 않습니다. "EOF 이후에도 NULL을 유지해야 한다"는 조건을 오라클에 명시했기 때문에 드러났습니다.

## Bug 2. read마다 이전 stash 누수

| 항목 | 내용 |
|---|---|
| 검출 | LeakSanitizer: direct leak (`ft_strjoin` ← `read_until_nl`) |
| 재현 입력 | `a` 한 글자 (`bug2_leak_one_char.bin`) |
| 발견까지 | Bug 1 수정본에서 libFuzzer 60초 이내 |

**원인**: `stash = ft_strjoin(stash, buf);`가 새 문자열을 만든 뒤 이전 `stash`를 해제하지 않습니다. `read`가 일어날 때마다 이전 버퍼 하나씩 누수됩니다.

**수정**: `bug2_fix.diff` (임시 포인터에 받고 이전 `stash` 해제)

## 두 버그를 고친 뒤

두 수정을 적용한 사본은 BUFFER_SIZE 1 / 42 / 100000 각각 60초(합계 약 145만 회 실행) 동안 오라클 위반, 메모리 오류, 누수가 없었습니다. 최종 제출 버전(`targets/v2_final`)도 같은 두 문제를 이미 고친 구조입니다.

## 참고: 의미 없는 오버플로 가드

8월 버전에는 `(size_t)BUFFER_SIZE > SIZE_MAX - 1`, `len > SIZE_MAX - 1` 같은 검사가 있었습니다. 둘 다 실제로는 항상 거짓이라 아무것도 막지 못합니다. 가드를 "썼다"는 것과 가드가 "동작한다"는 것은 다르다는 점을 이 기록을 정리하며 다시 확인했습니다.
