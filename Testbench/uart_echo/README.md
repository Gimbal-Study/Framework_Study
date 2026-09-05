# UART2 text echo testbench

기존 소스는 변경하지 않습니다. `build.ps1`은 기존 Makefile과 별개로 모든 오브젝트와 펌웨어를 이 폴더의 `build`에 생성합니다. 기존 `Core/main.c`를 사용하고, `Test/uart_test.c` 대신 `uart_echo_test.c`를 링크합니다. 두 테스트 파일을 동시에 링크하면 `uart_test_main`이 중복 정의됩니다.

## 빌드 / 실행

프로젝트 루트의 PowerShell에서:

```powershell
& ./Testbench/uart_echo/build.ps1
```

다운로드할 파일은 `Testbench/uart_echo/build/uart_echo.elf`입니다. 기존 `rom_0x08000000.elf`는 이 테스트벤치가 아닙니다. 스크립트는 보드 다운로드를 수행하지 않습니다. IDE를 사용한다면 별도 빌드 설정에서 기존 테스트 파일을 제외하고 새 테스트 파일을 추가하세요.

ComPortMaster: 해당 COM 포트, 115200 baud, 8 data bits, None parity, 1 stop bit, flow control 없음. 로컬 에코 및 자동 송신 줄바꿈은 끄고 시작하세요. 포트를 연 뒤 보드를 리셋하면 기존 main의 `mcal_uart Test` 메시지가 표시됩니다.

## 동작

1. 기존 ISR이 수신 큐에 데이터를 저장합니다.
2. 메인에서 매번 최대 64바이트를 처리합니다. 큐가 비면 바로 반환합니다.
3. 1바이트 읽기가 성공하면 그 바이트를 송신합니다. 문자열도 바이트 순서대로 즉시 돌아옵니다.
4. 현재 read가 종료 NUL을 덧붙이므로 수신 임시 배열은 2바이트입니다.
5. 현재 ISR이 LF 뒤에 삽입하는 NUL은 출력하지 않습니다. 실제 PC에서 보낸 NUL도 구별할 수 없으므로 버립니다.
6. `rx_cnt`, `Uart2_Rx_Expired`, `strlen`, printf, 지연은 사용하지 않습니다.

## 예상 결과

| 입력 | 응답 (시작 메시지 제외) |
|---|---|
| ASCII `a` | 즉시 `a` |
| ASCII `asdf` | 즉시 `asdf` |
| HEX `61` | `a` |
| HEX `61 0A` | `61 0D 0A` |
| HEX `61 0D 0A` | `61 0D 0D 0A` |
| HEX `00` | 출력 없음 |

LF가 CRLF로 바뀌는 것은 기존 `Uart_Send_Byte`의 동작입니다. CRLF 입력에서는 CR이 중복됩니다. 이 테스트는 일반 텍스트 에코이며 바이트 단위 무변환 바이너리 루프백은 아닙니다.

## 디버거 변수

- `tb_rx_bytes`: 성공적으로 큐에서 읽은 바이트 수 (ISR이 넣은 NUL 포함).
- `tb_tx_bytes`: MCAL write에 성공적으로 전달한 바이트 수 (드라이버가 넣는 CR 제외, 물리적 전송 완료 보장은 아님).
- `tb_skipped_nuls`: 버린 NUL 수.
- `tb_last_read`: 큐가 비면 기존 드라이버의 ERROR 값이 남는 것이 정상입니다.
- `tb_last_write`, `tb_tx_failed`: 송신 오류 확인. 실패 시 추가 소비를 멈추며 재시작하려면 리셋합니다.

## 범위와 한계

기존 UART ISR, 큐, 송신 함수를 그대로 사용합니다. 따라서 하드웨어 오류 처리, 큐 동기화/오버플로 문제, write 내부 TIM2 시작 부수 동작과 무한 TXE 대기는 해결하지 않습니다. 현재 timeout 인자도 실제 시간 제한을 제공하지 않습니다. 길이 1의 read/write와 텍스트 흐름을 확인하는 테스트이며 다중 바이트 read, timeout, 최대 속도 무손실 수신의 검증은 아닙니다.

긴 메시지 및 연속 입력 시험은 단일 문자와 짧은 문자열 검증 후 진행하세요. ISR에 출력이나 브레이크포인트를 넣으면 수신 타이밍이 달라질 수 있습니다.
