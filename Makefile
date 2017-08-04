
all: airplaneledd

airplaneledd:
	gcc -o airplaneledd airplaneledd.c
	strip airplaneledd

clean:
	rm airplaneledd

install: airplaneledd
	install -Dm755 airplaneledd /usr/bin/airplaneledd
	install -Dm644 airplaneledd.service /usr/lib/systemd/system/airplaneledd.service


