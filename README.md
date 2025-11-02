# X2C Scope Benchmark Demo

**Last updated:** November 2, 2025

This repository contains an MPLAB X project (`X2C-Scope-Blinky-dspic33AK128MC106-Curiosity.X`) designed to showcase how to integrate [X2C Scope](https://x2cscope.github.io/) into a dsPIC33AK application for real-time benchmarking and data visualization. The firmware runs on Microchip's dsPIC33AK128MC106  [Part Number: EV02G02A
](https://www.microchip.com/en-us/development-tool/EV02G02A) Curiosity board  [Part Number: EV74H48A
](https://www.microchip.com/en-us/development-tool/ev74h48a)  and streams live measurements (interrupt timing, sine/cosine generation, `atan2f` execution cost, etc.) to the X2C Scope tool.

The goal is to give firmware developers a ready-made sandbox for measuring algorithm timing inside an ISR and exploring how X2C Scope can be used as an in-circuit oscilloscope / watch window with zero code restarts.

## Project Highlights

- **Real-time signal streaming:** A Timer1 interrupt (1 kHz) feeds X2C Scope with live sine/cosine values, phase angle, and measurement counters.
- **Benchmark instrumentation:** The ISR timestamps the sine/cosine calculation and the standard `atan2f` call, reporting latency in timer ticks, microseconds, and estimated CPU cycles.
- **CPU load visibility:** The project tracks total ISR execution time and expresses it as a percentage of the 1 ms scheduling budget.
- **Runtime configurability:** Frequency set-points and other globals live in RAM so you can edit them from X2C Scope without recompiling.
- **dsPIC33AK-friendly:** Works out-of-the-box with the dsPIC33AK128MC106 Curiosity board, XC-DSC compiler, and libdsp support already included.

## Repository Layout

```
Benchmarking-dsPIC33AK128MC106-withX2Cscope/
├── README.md                <-- you are here
├── X2C-Scope-Blinky-dspic33AK128MC106-Curiosity.X/
│   ├── main.c               <-- Instrumented application code
│   ├── nbproject/           <-- MPLAB X configuration files
│   ├── mcc_generated_files/ <-- Melody-generated drivers (Timer1, UART, etc.)
│   └── ...
└── 
```

Everything you need to rebuild, modify, and run the benchmark resides in `X2C-Scope-Blinky-dspic33AK128MC106-Curiosity.X`.

## Requirements

- **Hardware:** Microchip dsPIC33AK128MC106 + Curiosity development board. ([Part Number: EV02G02A
](https://www.microchip.com/en-us/development-tool/EV02G02A) and [Part Number: EV74H48A
](https://www.microchip.com/en-us/development-tool/ev74h48a))
- **Toolchain:**
  - MPLAB X IDE v6.25 or later
  - XC-DSC compiler v3.21 (or newer)
  - dsPIC33AK-MC_DFP v1.2.125 (already referenced by the project)
- **Host tooling:**
  - X2C Scope for Windows/Linux (download from [x2cscope.github.io](https://x2cscope.github.io/))
  - pyX2Cscope (download from [pypi.org/project/pyx2cscope](https://pypi.org/project/pyx2cscope/))
  - USB-to-serial connection from the Curiosity board to your PC (uses directly the pkob4 present in the curiosity board)

## Setup Instructions

1. **Clone or download** the repository onto your development machine

2. **Open the MPLAB X project**:
   - Launch MPLAB X IDE
   - Choose *File → Open Project...* and select `X2C-Scope-Blinky-dspic33AK128MC106-Curiosity.X`

3. **Check toolchain paths**:
   - MPLAB X should automatically detect the XC-DSC compiler and the dsPIC33AK DFP. If not, point the project configuration to your local installations (`Project Properties → XC-DSC`).

4. **Compile the project**:
   - Use the hammer icon (*Build Project*) or press `F11`
   - The output `dist/default/production/X2C-Scope-Blinky-dspic33AK128MC106-Curiosity.X.production.hex` will be generated

5. **Flash the board**:
   - Connect the Curiosity board via USB and use the *Make and Program Device* button in MPLAB X

6. **Start X2C Scope** on your PC and configure the connection (see next section).

## Connecting X2C Scope

1. Launch X2C Scope and open your device configuration.
2. Set the COM port to match the Curiosity board (UART2 defaults to 115200-8-N-1).
3. Import the `.elf` file located in `dist/default/production/...X.production.elf` so X2C Scope knows the symbol table.
4. Add the following signals to your watch/plot lists:
   - `gScopeSignals.sineValue`
   - `gScopeSignals.cosineValue`
   - `gScopeSignals.trigTimeUs`, `trigCycles`
   - `gScopeSignals.arctangentRad`
   - `gScopeSignals.atanTimeUs`, `atanCycles`
   - `gScopeSignals.isrTimeUs`, `cpuLoadPercent`
   - `gScopeSignals.requestedFrequencyHz` (writeable)

5. Hit *Run* in X2C Scope. You should see the sine wave update in real time and the timing metrics refresh each millisecond.

YouTube Tutorial for X2Cscope set up: 
[VIDEO](https://youtu.be/ITWSejzQ9lc))

## How It Works

- `main.c` sets up Timer1 for 1 kHz and registers `TMR1_TimeoutCallback`.
- Each ISR invocation:
  1. Updates the phase angle and generates `sinf`/`cosf` values.
  2. Times the trig block and the `atan2f` call using raw Timer1 counts.
  3. Converts ticks to microseconds and cycles (`CLOCK_InstructionFrequencyGet()` is used for FCY).
  4. Calls `X2CScope_Update()` so the refreshed `gScopeSignals` structure is sent to the host.

Because all fields are global, X2C Scope can both **read** and **write** them. Adjusting `requestedFrequencyHz`, for example, changes the sine wave frequency on the fly without resetting the MCU.

## Customizing the Benchmark

Want to benchmark your own function? Edit `TMR1_TimeoutCallback`:

```c
const uint32_t myStart = TMR1;
YourFunctionUnderTest();
const uint32_t myEnd = TMR1;
const uint32_t myTicks = ScopeDemo_TickDelta(myStart, myEnd);
```

Use the helper `ScopeDemo_TicksToUs()` and `g_cyclesPerTimerTick` to convert the delta into the format you like and add new fields to `ScopeSignals_t` so X2C Scope can see them.

## Tips for Accurate Timing

- Keep the ISR lean—avoid `printf` or blocking loops.
- Timer1 runs at 100 MHz (10 ns resolution). If you lengthen the ISR period, update `PR1` and rebuild; the helper code automatically recalculates the tick→µs scaling when the MCU boots.
- The project measures *elapsed time* including function-call overhead. If you want instruction-level granularity, place the timers immediately around the code you own.

## Troubleshooting

| Issue | Fix |
|-------|-----|
| No data in X2C Scope | Confirm UART2 wiring and COM port. Check that `SYSTEM_Initialize()` calls `UART2_Initialize()` (generated by MCC).
| Inaccurate timing | Ensure XC-DSC build uses `--double=64` if you rely on double-precision math elsewhere. For `atan2f` (single precision) no extra flags are needed.
| Build fails with missing libs | Verify the XC-DSC installation path matches the one referenced in `nbproject/Makefile-default.mk` (libdsp-elf.a).
| CPU load unexpectedly high | Reduce work in the ISR or lower the Timer1 frequency (adjust PR1 in Melody and rebuild).

## Extending the Demo

- Add additional math kernels (FFT, PID updates) and stream their timing data.
- Log scope data to a CSV from the PC tool for offline analysis.
- Combine this project with Microchip's MCAF or motor-control examples by merging the ISR instrumentation into your control loop.

## References & Useful Links

- [X2C Scope documentation](https://x2cscope.github.io/)
- [XC-DSC Compiler User’s Guide](https://ww1.microchip.com/downloads/en/DeviceDoc/50002441E.pdf)
- [dsPIC33AK128MC106 Curiosity board datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/DS70005475A.pdf)
- [X2C Scope GitHub organization](https://github.com/X2Cscope)

Feel free to fork the repo, add your own benchmarks, and share results! Contributions are welcome—open an issue or submit a pull request with enhancements, bug fixes, or documentation improvements.
