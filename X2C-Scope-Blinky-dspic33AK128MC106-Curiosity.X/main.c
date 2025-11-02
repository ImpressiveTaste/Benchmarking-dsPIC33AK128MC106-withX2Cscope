/*
 * [2025] Microchip Technology Inc. and its subsidiaries.
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * You are responsible for complying with 3rd party license terms
 * applicable to your use of 3rd party software (including open source
 * software) that may accompany Microchip software. SOFTWARE IS "AS IS."
 * NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS
 * SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 * MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT
 * WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY
 * KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
 * MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
 * FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S
 * TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT
 * EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR
 * THIS SOFTWARE.
 */

#include "mcc_generated_files/X2Cscope/X2Cscope.h"
#include "mcc_generated_files/system/system.h"
#include "mcc_generated_files/system/clock.h"

#include <math.h>
#include <stdint.h>
/* Demonstration constants -------------------------------------------------- */
#define SCOPE_TWO_PI_F                6.28318530717958647692f
#define SCOPE_PI_F                     3.14159265358979323846f
#define SCOPE_DEFAULT_TIMER_TICK_US   0.01f
#define SCOPE_DEFAULT_WAVEFORM_HZ     1.0f

typedef struct
{
    /* Waveform state visible in X2C Scope */
    float phaseAngleRad;
    float sineValue;
    float cosineValue;
    float trigTimeUs;
    float arctangentRad;
    float atanTimeUs;

    /* Timing and frequency information */
    float requestedFrequencyHz;
    float actualFrequencyHz;
    float sampleRateHz;
    float timerTickUs;
    float samplePeriodUs;
    float isrTimeUs;
    float cpuLoadPercent;

    /* Raw counters exposed for quick inspection */
    uint32_t sampleCounter;
    uint32_t isrTicks;
    uint32_t trigTicks;
    uint32_t atanTicks;
    uint32_t isrCycles;
    uint32_t trigCycles;
    uint32_t atanCycles;
} ScopeSignals_t;

/* Global scope-visible signals (edited live from X2C Scope) */
volatile ScopeSignals_t gScopeSignals;

/* Internal bookkeeping for timing conversions */
static float g_timer1TickUs = SCOPE_DEFAULT_TIMER_TICK_US;
static float g_timer1PeriodTicks = 1.0f;
static float g_angleStepRad = 0.0f;
static float g_sampleRateHz = 1000.0f;
static uint32_t g_timer1PeriodCounts = 1UL;
static float g_cyclesPerTimerTick = 1.0f;

static inline uint32_t ScopeDemo_TickDelta(uint32_t startTick, uint32_t endTick);
static inline float ScopeDemo_TicksToUs(uint32_t ticks);
static void ScopeDemo_UpdateMetrics(uint32_t isrTicks);
static void ScopeDemo_UpdateAngleStep(void);
static void ScopeDemo_Initialize(void);

int main(void)
{
    SYSTEM_Initialize();
    ScopeDemo_Initialize();

    while (1)
    {
        /* Stream live data to the host while the CPU idles */
        X2CScope_Communicate();
    }
}

void TMR1_TimeoutCallback(void)
{
    const uint32_t isrStartTick = TMR1;

    gScopeSignals.sampleCounter++;

    ScopeDemo_UpdateAngleStep();

    float angle = gScopeSignals.phaseAngleRad + g_angleStepRad;
    if (angle > SCOPE_TWO_PI_F)
    {
        angle -= SCOPE_TWO_PI_F;
    }
    else if (angle < 0.0f)
    {
        angle += SCOPE_TWO_PI_F;
    }
    gScopeSignals.phaseAngleRad = angle;

    const uint32_t trigStartTick = TMR1;
    const float sineValue = sinf(angle);
    const float cosineValue = cosf(angle);
    const uint32_t trigEndTick = TMR1;
    const uint32_t trigTicks = ScopeDemo_TickDelta(trigStartTick, trigEndTick);
    gScopeSignals.trigTicks = trigTicks;
    gScopeSignals.trigTimeUs = ScopeDemo_TicksToUs(trigTicks);
    gScopeSignals.trigCycles = (uint32_t)(trigTicks * g_cyclesPerTimerTick);
    gScopeSignals.sineValue = sineValue;
    gScopeSignals.cosineValue = cosineValue;

    const uint32_t atanStartTick = TMR1;
    gScopeSignals.arctangentRad = atan2f(sineValue, cosineValue);
    const uint32_t atanEndTick = TMR1;
    const uint32_t atanTicks = ScopeDemo_TickDelta(atanStartTick, atanEndTick);
    gScopeSignals.atanTicks = atanTicks;
    gScopeSignals.atanTimeUs = ScopeDemo_TicksToUs(atanTicks);
    gScopeSignals.atanCycles = (uint32_t)(atanTicks * g_cyclesPerTimerTick);

    X2CScope_Update();

    const uint32_t isrEndTick = TMR1;
    const uint32_t isrTicks = ScopeDemo_TickDelta(isrStartTick, isrEndTick);

    ScopeDemo_UpdateMetrics(isrTicks);
}

static void ScopeDemo_Initialize(void)
{
    gScopeSignals.phaseAngleRad = 0.0f;
    gScopeSignals.sineValue = 0.0f;
    gScopeSignals.cosineValue = 1.0f;
    gScopeSignals.trigTimeUs = 0.0f;
    gScopeSignals.arctangentRad = 0.0f;
    gScopeSignals.atanTimeUs = 0.0f;
    gScopeSignals.requestedFrequencyHz = SCOPE_DEFAULT_WAVEFORM_HZ;
    gScopeSignals.actualFrequencyHz = SCOPE_DEFAULT_WAVEFORM_HZ;
    gScopeSignals.sampleRateHz = 0.0f;
    gScopeSignals.timerTickUs = SCOPE_DEFAULT_TIMER_TICK_US;
    gScopeSignals.samplePeriodUs = 0.0f;
    gScopeSignals.isrTimeUs = 0.0f;
    gScopeSignals.cpuLoadPercent = 0.0f;
    gScopeSignals.sampleCounter = 0U;
    gScopeSignals.isrTicks = 0U;
    gScopeSignals.trigTicks = 0U;
    gScopeSignals.atanTicks = 0U;
    gScopeSignals.isrCycles = 0U;
    gScopeSignals.trigCycles = 0U;
    gScopeSignals.atanCycles = 0U;

    const uint32_t periodCounts = (uint32_t)(PR1 + 1UL);
    g_timer1PeriodCounts = (periodCounts > 0U) ? periodCounts : 1UL;
    g_timer1PeriodTicks = (float)g_timer1PeriodCounts;
    g_timer1TickUs = 1000.0f / g_timer1PeriodTicks;
    gScopeSignals.timerTickUs = g_timer1TickUs;
    gScopeSignals.samplePeriodUs = g_timer1TickUs * g_timer1PeriodTicks;
    g_sampleRateHz = (gScopeSignals.samplePeriodUs > 0.0f)
        ? (1000000.0f / gScopeSignals.samplePeriodUs)
        : 0.0f;
    gScopeSignals.sampleRateHz = g_sampleRateHz;

    const float cpuFreqHz = (float)CLOCK_InstructionFrequencyGet();
    const float cyclesPerUs = cpuFreqHz / 1000000.0f;
    g_cyclesPerTimerTick = cyclesPerUs * g_timer1TickUs;

    ScopeDemo_UpdateAngleStep();
}

static inline uint32_t ScopeDemo_TickDelta(uint32_t startTick, uint32_t endTick)
{
    if (endTick >= startTick)
    {
        return endTick - startTick;
    }

    return (uint32_t)((g_timer1PeriodCounts - startTick) + endTick);
}

static inline float ScopeDemo_TicksToUs(uint32_t ticks)
{
    return (float)ticks * g_timer1TickUs;
}

static void ScopeDemo_UpdateMetrics(uint32_t isrTicks)
{
    gScopeSignals.isrTicks = isrTicks;
    gScopeSignals.isrTimeUs = ScopeDemo_TicksToUs(isrTicks);
    gScopeSignals.isrCycles = (uint32_t)(isrTicks * g_cyclesPerTimerTick);
    gScopeSignals.timerTickUs = g_timer1TickUs;
    gScopeSignals.samplePeriodUs = g_timer1TickUs * g_timer1PeriodTicks;
    gScopeSignals.sampleRateHz = g_sampleRateHz;

    const float loadPercent = (g_timer1PeriodTicks > 0.0f)
        ? ((float)isrTicks / g_timer1PeriodTicks) * 100.0f
        : 0.0f;
    gScopeSignals.cpuLoadPercent = loadPercent;
}

static void ScopeDemo_UpdateAngleStep(void)
{
    if (g_sampleRateHz > 0.0f)
    {
        g_angleStepRad = (SCOPE_TWO_PI_F * gScopeSignals.requestedFrequencyHz) / g_sampleRateHz;
        gScopeSignals.actualFrequencyHz = (g_angleStepRad * g_sampleRateHz) / SCOPE_TWO_PI_F;
    }
    else
    {
        g_angleStepRad = 0.0f;
        gScopeSignals.actualFrequencyHz = 0.0f;
    }
}
