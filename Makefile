# Makefile to build and upload ardunio sketch

INO=arduino
INOFLAGS=--board arduino:avr:uno --port /dev/ttyACM0 --pref build.path=./build
SRC=src/sketch.ino


build: $(SRC)
	$(INO) --verify $(INOFLAGS) --verbose-build $(SRC)

burn: $(SRC)
	$(INO) --upload $(INOFLAGS) $(SRC)

clean:
	rm -r ./build/

