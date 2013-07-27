all: query index

query: query.cpp
	g++ query.cpp -o query -llucene++ -lpthread -lboost_system -O3

index: index.cpp
	g++ index.cpp -o index -llucene++ -lpthread -lboost_system -O3

clean:
	rm -rf index query