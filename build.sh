#!/bin/bash
#
# To flash, run this script with the photon in DFU mode
# To enter DFU mode:
#   1. Hold the "SETUP" button
#   2. While still holding SETUP, press and release the "RESET" button
#   3. When the status LED is blinking yellow, release the "SETUP" button
#

particle compile photon && particle flash --usb $(ls -ltr *.bin |tail -1 |cut -d" " -f9) && particle serial monitor
