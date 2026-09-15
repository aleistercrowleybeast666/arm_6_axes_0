.PHONY: all clean rebuild host
all:
	cmake --preset firmware
	cmake --build --preset firmware
clean:
	cmake --build --preset firmware --target clean
rebuild: clean all
host:
	cmake --preset host
	cmake --build --preset host
	ctest --preset host
