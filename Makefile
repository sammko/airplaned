
all: airplaneledd

airplaneledd:
	gcc -o airplaned airplaned.c
	strip airplaned

clean:
	rm airplaneledd

install: airplaneledd
	install -Dm755 airplaned /usr/bin/airplaned
	install -Dm644 airplaned@.service /usr/lib/systemd/system/airplaned@.service


