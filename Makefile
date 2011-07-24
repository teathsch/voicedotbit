LIBS = `pkg-config --libs libcurl gtkmm-2.4 sqlite3 libcrypto` -lboost_thread -lboost_program_options
CFLAGS = `pkg-config --cflags libcurl gtkmm-2.4 sqlite3 libcrypto` -I./json_spirit_v4.03

all:
	g++ $(CFLAGS) -c main.cpp -o main.o
	g++ main.o -o voicedotbit $(LIBS)

clean:
	rm -f *.o voicedotbit
