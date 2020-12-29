Import("env")
import os

os.system("rm "+env['PROJECT_PACKAGES_DIR']+"/framework-arduinoteensy/libraries/VirtualWire/util/crc.h")

