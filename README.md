# test-rdtsc -- Sanity tests of Intel x86 rdtsc()

### Description

`test-rdtsc` dows some basic sanity tests of the
`rdtsc` and `rdtscp` instructions.

It tests the combination of hardware and software
that delivers the high-resolution timestamps.

A high-resolution clock must always increase in value.
In particular, for a 64-bit timestamp, the carry from
the low-order 32-bit to the high-order bits must
work correctly.

#### clock frequency

On a hosted system, there is also a scaled high-resolution time
that delivers timestamps in standard units of time.
On modern Linux systems, that would be xxx, which delivers timestamps in units of seconds and nanoseconds.  This can be used to guess the frequency of the TSC.

It is another sanity test.
Since it involves some circular reasoning,
it cannot be used as primary source of information about the hardware.

#### timing and variance

In addition to the sanity tests,
it provides some measure of how many TSC ticks it takes for each sample,
its arithmetic mean, standard deviation and %coefficient of variance.

This can be invalid, due to scheduling by the operating system.
But, in practice, the tests are quick enough that you can get a series
of samples, without being preempted.


## License

See the file `LICENSE.md`

-- Guy Shaw

   gshaw@acm.org

