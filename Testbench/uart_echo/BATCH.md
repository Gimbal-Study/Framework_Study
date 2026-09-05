# 가변 길이 텍스트 에코

빌드: 프로젝트 루트에서 `& ./Testbench/uart_echo/build.ps1 -Batch`

펌웨어: `build-batch/uart_echo.elf`. 기본 버전의 build 폴더와 구분됩니다. 보드 다운로드는 자동으로 수행하지 않습니다. 기존 main 및 MCAL 소스는 그대로 링크하고 기존 Test/uart_test.c 대신 uart_batch_test.c만 선택합니다.

첫 수신을 확인하면 인터럽트를 허용한 상태로 2ms 동안 데이터를 모읍니다. 그 뒤 실제 큐 점유 개수를 계산하고 최대 128바이트를 한 번의 mcal_uart_read로 읽습니다. NUL을 제외한 실제 출력 개수를 한 번의 mcal_uart_write에 전달합니다. ISR이 LF 뒤에 넣는 NUL 때문에 read_len과 write_len이 다를 수 있습니다. 실제 입력 NUL도 제외하는 텍스트 전용 테스트입니다.

2ms는 고정 수집 시간이며 마지막 바이트로부터 측정한 idle timeout이 아닙니다. Send 클릭의 패킷 경계를 뜻하지 않습니다. 짧은 asdf가 모두 이 시간 안에 도착하면 read_len=4, write_len=4입니다. a만 도착하면 1, 1입니다. asdf+LF 및 ISR의 NUL이 모두 도착하면 6, 5입니다. LF는 기존 송신 드라이버에서 CRLF로 변환됩니다. 긴 입력은 여러 배치로 나뉘며, 가까운 여러 Send 입력은 합쳐질 수 있습니다. 정확히 Send 한 번마다 한 호출이 필요하면 종료 구분자나 길이 헤더 프로토콜을 별도로 정해야 합니다.

공개 MCAL API에는 수신 개수 조회 함수가 없으므로 테스트 전용 available_count가 uart2_q 인덱스를 잠깐의 인터럽트 차단으로 함께 읽습니다. 저장된 PRIMASK를 복원하며 대기와 송신 동안에는 차단하지 않습니다. 직접 데이터를 꺼내거나 DR을 읽지는 않습니다. STM32F411 및 현재 queue_t 구조에 종속되며 동일 큐의 다른 소비자와 동시에 실행하면 안 됩니다. 기존 드라이버의 큐 동기화/오버런, 송신 TIM2 부수 동작, 미구현 timeout 제약은 그대로입니다.

디버거: tb_last_read_len / tb_last_write_len은 마지막 배치 길이, tb_batches는 처리 배치 수, tb_received / tb_transmitted는 누적 큐 수신 및 MCAL 전달 바이트 수입니다. 추가 CR의 선로 바이트 수는 포함하지 않습니다. tb_failed=1이면 읽기 또는 송신 실패로 중지된 상태입니다. 브레이크포인트로 수신 중 정지하면 타이밍에 영향을 줄 수 있습니다.

ComPortMaster: 115200 / 8N1 / 흐름 제어 없음, 로컬 에코와 자동 송신 줄바꿈을 끄고 a, asdf, 길이가 다른 문자열 순서로 확인하세요. 여러 바이트가 수집 시간 안에 도착했을 때 len>1이 됩니다. 이 빌드는 보드에서의 동작이나 무손실 처리량을 보증하지 않습니다.
