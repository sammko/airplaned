
all: airplaned

airplaned:
	gcc -o airplaned airplaned.c
	strip airplaned

clean:
	rm airplaned

install: airplaned
	install -Dm755 airplaned /usr/bin/airplaned
	install -Dm644 airplaned@.service /usr/lib/systemd/system/airplaned@.service


