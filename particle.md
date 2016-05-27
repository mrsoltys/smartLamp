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

hold setup for 3 seconds till flashing blue
particle setup wifi

If wifi encreption is wep, you have to jump through some hoops. Covert your password to HEX and then add to the front of the password

00 (or 01, 02, 03 depending on your rotuer's config)

0D (for a 13 character password)

Example: if password is abcdefghijkl use:

Example: 000D6162636465666768696a6b6c