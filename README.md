# STM32 엘리베이터 제어 시스템

NUCLEO-F411RE로 구현한 3층 엘리베이터 모형입니다. 층 버튼·UART 명령과 센서 입력에 따라 이동하고, 이동 중 비상 발생 시 다음 감지 층에서 정지하도록 제어합니다.

**C · STM32 HAL · STM32CubeIDE · Timer ISR · ADC DMA · PWM · UART · I2C**

## 시연

[전체 동작](https://drive.google.com/file/d/1EE3fmTnKbiAE042-x2fC5OMQD9OuJrPx/view) · [비상 처리](https://drive.google.com/file/d/13oAqV4IlNijQL13RI33TjWsxp0VWVCnp/view) · [UART 1→3층](https://drive.google.com/file/d/1yS5ObFpkTDXuxGA_ZArS5HB3Yzja3XVP/view) · [UART 3→2층](https://drive.google.com/file/d/1XWFjfef61wtSbKiE_9OcrOv1D8Ed3XYd/view)

## 담당 역할

| 구분 | 담당 |
| --- | --- |
| **본인** | 엘리베이터 제어, 로비 호출, 비상 처리, 모터, LCD/FND, RTC, 사운드 |
| 팀원 | UART 통신, GUI 개발, DHT11 센서 연결 |

## 핵심 구현과 코드

| 구현 | 설계 포인트 | 코드 |
| --- | --- | --- |
| 층 이동·비상 정지 | 상태와 이동 방향을 관리하고, 센서로 현재 층을 갱신하여 다음 층 정지 예약 처리 | [elevator.c](firmware/Core/Src/ap/service/elevator/elevator.c) |
| 인터럽트 역할 분리 | TIM2 이동 제어 2ms, TIM5 비상 입력 확인 1ms, TIM4 FND 갱신 1ms | [ap_main.c](firmware/Core/Src/ap/ap_main.c) |
| 로비 호출 | 초음파 거리·조도 히스테리시스·중복 호출 방지 | [lobby.c](firmware/Core/Src/ap/service/lobby/lobby.c) |
| 사운드 | ISR의 요청을 메인 루프로 전달하고 경과 시간에 따라 재생 단계 갱신 | [sound.c](firmware/Core/Src/ap/service/sound/sound.c) |

타이머 수치는 설정 주기입니다. 상세 실행 흐름과 상태 처리는 [아키텍처 문서](docs/architecture.md)에 정리했습니다.

## 트러블슈팅

- **비상 확인 간격:** 메인 루프의 통신·대기 영향을 줄이기 위해 비상 확인과 이동 제어를 타이머 ISR로 분리했습니다.
- **비상 정지 층 판단:** 2층 통과 플래그에 의존하던 방식에서 현재 층 갱신과 `stopAtNextFloor` 정지 예약으로 변경했습니다.
- **ISR의 버튼 처리:** 지연 함수를 호출하는 일반 버튼 처리와 구분해 비상 버튼에 연속 샘플 디바운스를 적용했습니다.

[문제·원인·변경 내용 자세히 보기](docs/troubleshooting.md)

## 프로젝트 구성

- `firmware/`: CubeIDE 프로젝트, 애플리케이션·드라이버·HAL/CMSIS
- `docs/`: [핀맵](docs/pinmap.md), [UART 인터페이스](docs/uart.md), [제약과 개선 후보](docs/verification.md)

CubeIDE에서 `firmware/`를 기존 프로젝트로 가져옵니다. 프로젝트 이름은 `20261109_elevatorFinal`이며, 보드/MCU는 **NUCLEO-F411RE / STM32F411RE**입니다. PC GUI 소스는 포함되어 있지 않습니다.
