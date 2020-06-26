**Embedded System Software**
#### P1
  - input process <- main -> output process
  - main process forks two different processes.
  - use inter process communication : shared memory
  - Device control by using either mmap() function or device driver
  
#### P2
  - use two ioctl commands to deliever user's operating options and to enable timer device
  - implement device drivers (fnd, led, dot, text_lcd) and timer function in one module
  - execution : ./app TIMER_INTERVAL[1-100] TIMER_CNT[1-100] TIMER_INIT[0001-8000]
  - timer devicer name : /dev/dev_driver, major number : 242
  - 2020.05.19 : delete howmany variable / assign major number explicitly / tweak text lcd printing / clear outputs after iteration
  - 2020.05.21 : printk for details / commands using iotcl functions
  
#### P3
  - simple stopwatch program using module programming, device driver and interrupt
  - implement device driver which contains fpga_fnd and interrupt in one module
  - execution : ./app
  - device driver name : /dev/stopwatch, major number : 242
  
#### P4
  - simple but cute! android rhythm game
  - 2020.06.23 : start a project! add 1 song to play
  - 2020.06.24 : enable MediaPlayer, add 2 new songs, save my best score, music speed control
  - 2020.06.25 : add new Activity for free play mode
  - 2020.06.26 : add new Activity for track play mode - can record your own songs :smile:
