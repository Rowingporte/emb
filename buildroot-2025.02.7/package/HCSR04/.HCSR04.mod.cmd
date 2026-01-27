savedcmd_HCSR04.mod := printf '%s\n'   HCSR04.o | awk '!x[$$0]++ { print("./"$$0) }' > HCSR04.mod
