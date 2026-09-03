#!/bin/sh

# Gimbal Framework validation harness
#
# Purpose:
#   - 금지된 영역 변경 확인
#   - Test Harness 기본 파일 확인
#   - Build 명령 실행
#   - 선택적으로 Test 실행
#
# Usage:
#
#   ./harness/validate.sh
#
# Optional environment variables:
#
#   BUILD_CMD="..."  ./harness/validate.sh
#   TEST_CMD="..."   ./harness/validate.sh
#
# Example:
#
#   BUILD_CMD="make -C Debug" ./harness/validate.sh
#
# NOTE:
# STM32CubeIDE 프로젝트의 실제 Build Command는
# 현재 프로젝트 설정을 확인한 뒤 BUILD_CMD로 지정한다.
# Debug/Release의 generated makefile을 직접 수정하지 않는다.

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECT_ROOT=$(CDPATH= cd -- "${SCRIPT_DIR}/.." && pwd)

BUILD_CMD=${BUILD_CMD:-}
TEST_CMD=${TEST_CMD:-}

PASS_COUNT=0
FAIL_COUNT=0
SKIP_COUNT=0

pass()
{
    printf '[PASS] %s\n' "$1"
    PASS_COUNT=$((PASS_COUNT + 1))
}

fail()
{
    printf '[FAIL] %s\n' "$1"
    FAIL_COUNT=$((FAIL_COUNT + 1))
}

skip()
{
    printf '[SKIP] %s\n' "$1"
    SKIP_COUNT=$((SKIP_COUNT + 1))
}

run_check()
{
    description=$1
    shift

    if "$@"; then
        pass "${description}"
    else
        fail "${description}"
    fi
}

check_required_files()
{
    missing=0

    for file in \
        "AGENTS.md" \
        "skills/architecture.md" \
        "skills/testing.md" \
        "harness/permissions.md"
    do
        if [ ! -f "${PROJECT_ROOT}/${file}" ]; then
            printf '  missing: %s\n' "${file}"
            missing=1
        fi
    done

    [ "${missing}" -eq 0 ]
}

check_test_harness_files()
{
    # Test Harness 구현이 아직 진행 중일 수 있으므로
    # 파일이 없으면 validation 자체를 실패시키지 않고 SKIP으로 처리한다.
    required="
Test/test.h
Test/test_assert.h
Test/test_runner.h
Test/test_runner.c
"

    missing=0

    for file in ${required}
    do
        if [ ! -f "${PROJECT_ROOT}/${file}" ]; then
            printf '  not found: %s\n' "${file}"
            missing=1
        fi
    done

    return "${missing}"
}

check_forbidden_git_changes()
{
    if ! command -v git >/dev/null 2>&1; then
        return 2
    fi

    if ! git -C "${PROJECT_ROOT}" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        return 2
    fi

    changed_files=$(
        {
            git -C "${PROJECT_ROOT}" diff --name-only
            git -C "${PROJECT_ROOT}" diff --cached --name-only
            git -C "${PROJECT_ROOT}" ls-files --others --exclude-standard
        } | sort -u
    )

    forbidden=$(
        printf '%s\n' "${changed_files}" |
        grep -E '^(CMSIS/|Debug/|Release/|\.git/)' || true
    )

    if [ -n "${forbidden}" ]; then
        printf '%s\n' "${forbidden}" | sed 's/^/  forbidden change: /'
        return 1
    fi

    return 0
}

check_test_register_access()
{
    if [ ! -d "${PROJECT_ROOT}/Test" ]; then
        return 2
    fi

    # 대표적인 STM32 Peripheral Register 직접 접근 패턴을 찾는다.
    #
    # 이 검사는 완전한 정적 분석기가 아니라 빠른 Guard 역할이다.
    # False Positive가 발생하면 해당 코드를 직접 검토한다.
    matches=$(
        grep -R -n -E \
            '(GPIO[A-Z]->|USART[0-9]+->|TIM[0-9]+->|SPI[0-9]+->|I2C[0-9]+->|DMA[0-9]*->|RCC->)' \
            "${PROJECT_ROOT}/Test" \
            --include='*.c' \
            --include='*.h' \
            2>/dev/null || true
    )

    if [ -n "${matches}" ]; then
        printf '%s\n' "${matches}" | sed 's/^/  direct register access: /'
        return 1
    fi

    return 0
}

run_build()
{
    if [ -z "${BUILD_CMD}" ]; then
        return 2
    fi

    (
        cd "${PROJECT_ROOT}"
        sh -c "${BUILD_CMD}"
    )
}

run_tests()
{
    if [ -z "${TEST_CMD}" ]; then
        return 2
    fi

    (
        cd "${PROJECT_ROOT}"
        sh -c "${TEST_CMD}"
    )
}

printf '%s\n' '========================================'
printf '%s\n' ' Gimbal Framework Harness Validation'
printf '%s\n' '========================================'

run_check "Required harness files" check_required_files

if check_test_harness_files; then
    pass "Test Harness core files"
else
    skip "Test Harness core files are not fully implemented yet"
fi

if check_forbidden_git_changes; then
    pass "No forbidden directory changes"
else
    status=$?
    if [ "${status}" -eq 2 ]; then
        skip "Git change validation unavailable"
    else
        fail "Forbidden directory change detected"
    fi
fi

if check_test_register_access; then
    pass "No direct MCU register access in Test/"
else
    status=$?
    if [ "${status}" -eq 2 ]; then
        skip "Test/ register-access check unavailable"
    else
        fail "Direct MCU register access detected in Test/"
    fi
fi

if run_build; then
    pass "Project build"
else
    status=$?
    if [ "${status}" -eq 2 ]; then
        skip "Project build: BUILD_CMD is not configured"
    else
        fail "Project build"
    fi
fi

if run_tests; then
    pass "Related tests"
else
    status=$?
    if [ "${status}" -eq 2 ]; then
        skip "Related tests: TEST_CMD is not configured"
    else
        fail "Related tests"
    fi
fi

printf '\n'
printf '%s\n' '--------------- Summary ----------------'
printf 'PASS : %s\n' "${PASS_COUNT}"
printf 'FAIL : %s\n' "${FAIL_COUNT}"
printf 'SKIP : %s\n' "${SKIP_COUNT}"
printf '%s\n' '----------------------------------------'

if [ "${FAIL_COUNT}" -ne 0 ]; then
    exit 1
fi

exit 0
