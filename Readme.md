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
  - TODO : delete howmany variable / assign major number explicitly / tweak text lcd printing / clear outputs after iteration / commands using iotcl functions
