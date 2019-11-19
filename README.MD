# UHF Ground Station Code V1.0a (Si 4463 Rev. C2)
Forked off Zak's code. Most things are implemented, will have to write a shell program that is compatible with SPACE-HAUC compatible packet structure.

Zak's code was changed so that the radio does not fall back to IDLE mode after a FIFO read and thus allow RX to continue (by adding `setState(SI446X_STATE_RX)`).

Changelog:

1. Implemented dual serial for data and debug. 
1. Implemented IRQ-driven pipe mode enable signal.
1. Implemented byte-by-byte transmission mirroring Endurosat Pipe Mode setting.

TODO:
1. Implement packet structure. (Done)
1. Test TX to Endurosat. (Done)
1. Test RX from Endurosat. (Done)
1. ~Confirm (and implement if necessary) AES-256 encryption and decryption protocols.~
1. Implement shell mode if possible.