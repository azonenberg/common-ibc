# common-ibc

This is a 48-12V 72W non-isolated intermediate bus converter which I plan to use in all of my larger FPGA projects.

It uses a charge pump which has a fixed divide-by-4 output ratio rather than using a regulating buck topology. This
allows high efficiency but requires downstream loads to be able to tolerate a wider range of input voltages.

The architecture is scalable to 2 or 4 parallel converter modules, allowing 144W (12A) or 288W (24A) IBCs based on the
same chipset to be constructed in a straightforward manner based on a scaling of this design, however there are no
near-term plans to create such a converter since none of my near term project plans require that much power.

## Input

The input is +48VDC nominal on a 6-pin Molex Mini-Fit Jr connector, using a pinout compatible with the Mean Well
GST280A48-C6P power brick (and possibly others). The negative terminal of the input power connection is connected to
circuit ground.

The permissible operating range is 40 to 56VDC (corresponding to 10 to 14V output). An input protection circuit will
disable the converter if the input is out of this range.

The unregulated output voltage is 1/4 of the input, so if the load has tighter requirements for accuracy of the 12V
rail then this will further constrain the input supply voltage.

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

The 3.3V standby output will remain active and the management MCU will remain operational while a software interlock is
tripped.

### Input overcurrent

The 48V input is fused at 2.5A using a socketed 5x20mm fast-blow fuse.

### Output overcurrent

The 12V primary output is fused at 10A using a soldered-down fast-blow fuse.

The 3.3V standby output is current limited (typical range 3.8 - 5.2A) by the DC-DC converter and does not have an
supplemental fuse.

If the 12V output current exceeds 6.5A, a software interlock is triggered. The output load switch also has a
short circuit protection feature, however the software interlock is intended to trip at a lower level.

### Regulator fault

If any of the internal regulators indicates a problem (PGOOD low) a software interlock is triggered.

### Over-temperature

If the AT30TS74 indicates an out-of-bounds temperature (ALERT low) a software interlock is triggered.

The MCU does not configure the AT30TS74; on power up the default over-temperature limit (80C) is active. This sensor is
located on the same I2C management bus as the MCU and may be reconfigured or queried by the load as required; the
IBC will activate the interlock if the programmed temperature range is exceeded.

### Input under / over voltage

The input is monitored by a TPS16630 protection controller, which also provides soft-start for the input bulk
capacitance. If the input voltage drops below 40V or rises above 56V (corresponding to 10-14V output) the system will
shut down completely (input power cut off upstream of the charge pump, disabling the microcontroller, 3.3V standby
rail, and all internal circuitry past the protection controller).

The input has a Schottky diode to ground in reverse polarity. If the input goes significantly negative (reverse
polarity connection) the diode will conduct and blow the input fuse, hopefully protecting the load and possibly even
the power supply from damage.

As an additional protection against catastrophic overvoltage at the input, a zener clamp will trip at approximately
58V, blowing the input fuse.

### Output under / over voltage

Since the output voltage is normally a fixed 4:1 scaling of the input, output voltage drifting beyond this range is
likely indicative of a failure of the charge pump module.

* If the 12V rail voltage drops below 10V, a software interlock is triggered.
* If the 12V rail voltage rises above 14V, a software interlock is triggered.
* If the 12V rail voltage rises above 17V a zener clamp on the output will begin to conduct, blowing the output fuse.
