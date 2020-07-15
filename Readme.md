**Embedded System Software**
 - 임베디드 소프트웨어를 개벌하는데 필요한 여러 가지 기법을 공부하여 시스템 프로그래밍 기술들을 익힘

#### P1
  - 개발 보드 상에서 리눅스에서 제공하는 다양한 API들을 사용하여 보두 상의 디바이스들을 제어하는 프로그램 개발
  - input process <- main -> output process
  - main process forks two different processes.
  - use inter process communication : shared memory
  - Device control by using either mmap() function or device driver
  
#### P2
  - 리눅스에서 시스템 콜을 신규로 생성하며, 간단한 모듈 및 디바이스 드라이버 프로그램 개발
  - use two ioctl commands to deliever user's operating options and to enable timer device
  - implement device drivers (fnd, led, dot, text_lcd) and timer function in one module
  - execution : ./app TIMER_INTERVAL[1-100] TIMER_CNT[1-100] TIMER_INIT[0001-8000]
  - timer devicer name : /dev/dev_driver, major number : 242
  - 2020.05.19 : delete howmany variable / assign major number explicitly / tweak text lcd printing / clear outputs after iteration
  - 2020.05.21 : printk for details / commands using iotcl functions
  
#### P3
  - 리눅스 상에서 인터럽트 기능을 이용한 스톱 워치 개발
  - simple stopwatch program using module programming, device driver and interrupt
  - implement device driver which contains fpga_fnd and interrupt in one module
  - execution : ./app
  - device driver name : /dev/stopwatch, major number : 242
  
#### P4
  - 안드로이드 운영체제에 대한 구조를 이해하고 안드로이드 프레임워크 내부 프로그램 및 안드로이드 응용 프로그램 개발 기술 적용하여 프로젝트 완성
  - simple but cute! android rhythm game
  - 2020.06.23 : start a project! add 1 song to play
  - 2020.06.24 : enable MediaPlayer, add 2 new songs, save my best score, music speed control
  - 2020.06.25 : add new Activity for free play mode
  - 2020.06.26 : add new Activity for track play mode - can record your own songs :smile:
