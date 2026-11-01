
Sensor list  
Optical Sensors - We are using the PAW3395 mouse sensor
Rotary encoders - We are using the E6A2 
IMU  
Limit switches  

## Mouse X movement tracker

`myscreamingiseternal.c++` is a Windows console program that tracks raw mouse
movement system-wide and prints cumulative relative X/Y movement counts. It
does not show a tracker window; it uses an invisible message-only receiver
required by Windows raw input.

Build from a Visual Studio Developer Command Prompt:

```text
cl /EHsc /W4 /TP Controls\myscreamingiseternal.c++
```

Run `myscreamingiseternal.exe` from the Developer Command Prompt. Move the mouse
anywhere to see the cumulative X/Y totals in that terminal. Press Ctrl+C to
stop tracking.
