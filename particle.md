===Install and open Xcode through the apple app store ===

====Install Homebrew===
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

===Install DFU utils===
brew install dfu-util

particle update

particle login

particle compile photon GitHub/smartLamp --saveTo GitHub/smartLamp/firmware.bin

put particle in dfu mode to flash code.

particle flash --usb GitHub/smartLamp/firmware.bin