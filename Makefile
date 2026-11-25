all: producer  consumer

consumer: consumer.cpp
	g++ -Wall -o consumer consumer.cpp -lrt -lpthread

producer: producer.cpp
	g++ -Wall -o producer producer.cpp -lrt -lpthread

clean:
	rm -f consumer producer

.PHONY: all clean