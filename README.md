# common-ibc

This is a 48-12V 72W non-isolated intermediate bus converter which I plan to use in all of my larger FPGA projects.

It uses a charge pump which has a fixed divide-by-4 output ratio rather than using a regulating buck topology. This
allows high efficiency but requires downstream loads to be able to tolerate a wider range of input voltages.

## Input

The input is +48VDC nominal on a 6-pin Molex Mini-Fit Jr connector, using a pinout compatible with the Mean Well
GST280A48-C6P power brick (and possibly others). The negative terminal of the input power connection is connected to
circuit ground.

While the DC-DC module is rated for 20-60V operation, the input capacitors have a 63V rating. With the
manufacturer-suggested 80% derating, the input supply should not exceed 50.4V on a continuous basis although short term
excursions up to 60V will not cause damage.

The unregulated output voltage is 1/4 of the input, so the load's requirements will constrain the allowed
input voltage range as well.

## Internal Construction

The converter is built around the Murata MYC0409-NA charge pump DC-DC module.

## Outputs

In addition to the high current 12V output (on an 8-pin Molex Mini-Fit Jr connector) the converter provides a 3.3V
standby rail (using the Cyntec MUN12AD03-SE regulating off of the 12V rail), capable of up to 3A, which is intended for
powering sleep/idle circuitry such as soft power on/off, rail sequencing, etc.

Both the primary 12V step-down converter and the 3.3V standby regulator are always enabled, however a load switch on
the main 12V output allows it to be disabled to completely power down the bulk of the downstream device, keeping only
the converter board and the 3V3_SB rail on the downstream device energized. The load switch has an integrated soft
start, preventing high current surges when charging bulk capacitance on the load.

## Monitoring

The onboard STM32L431 microcontroller monitors the PGOOD signals of internal regulators as well as the ALERT output
of the onboard AT30TS74 temperature sensor. It also continually uses an internal ADC to record the input and output
voltage, downstream load sense voltage, and input and output current. All of these sensor readings may be queried by
the load over the I2C management bus.

## Protection

The converter includes multiple levels of protection to minimize the risk of fire or equipment damage.

The most critical interlocks are implemented in hardware while additional protections with lower thresholds are
implemented in software on the management MCU.

If a software interlock triggers, the 12V output is disabled. The management MCU will automatically attempt to restart
the output (as long as the load is still requesting power) at periodic intervals once the fault condition is cleared.

The 3.3V standby output will remain active and the management MCU will remain operational.

### Input overcurrent

The 48V input is fused at 2.5A using a socketed 5x20mm fast-blow fuse.

### Output overcurrent

The 12V primary output is fused at 10A using a soldered-down fast-blow fuse.

The 3.3V standby output is current limited (typical range 3.8 - 5.2A) by the DC-DC converter and does not have an
supplemental fuse.

If the 12V output current exceeds 6.5A, a software interlock is triggered. The output load switch also has an
overcurrent shutdown feature, however the software interlock is intended to trip at a lower level.

### Regulator fault

If any of the internal regulators indicates a problem (PGOOD low) a software interlock is triggered.

### Over-temperature

If the AT30TS74 indicates an out-of-bounds temperature (ALERT low) a software interlock is triggered.

The MCU does not configure the AT30TS74; on power up the default over-temperature limit (80C) is active. This sensor is
located on the same I2C management bus as the MCU and may be reconfigured or queried by the load as required.

### Under / over voltage

If the 12V rail voltage drops below 10V, a software interlock is triggered.

If the 12V rail voltage rises above 14V, a software interlock is triggered.

If the 12V rail voltage rises above 17V a zener clamp on the output will begin to conduct, blowing the output fuse.
