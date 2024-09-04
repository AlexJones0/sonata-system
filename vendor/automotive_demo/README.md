# sonata-automotive-demo

This repo contains the common library code that is used to implement the
automotive demo applications for the Sonata board. The library is designed
to be hardware-independent, where access to necessary hardware, MMIO and
functionality is implemented through the use of a set of callback functions
that any implementer will provide. This allows the demo to be implemented
for two environments required for showcasing the demo - one on the Sonata
board running CHERIoT-RTOS with CHERIoT enabled, and another running 
baremetal on Sonata without CHERIoT enabled, to show the difference in the
demo behaviour with CHERI running and not running.
