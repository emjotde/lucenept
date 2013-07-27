all: query index

query: query.cpp
	g++ query.cpp -o query -llucene++ -lpthread -lboost_system -O6

index: index.cpp
	g++ index.cpp -o index -llucene++ -lpthread -lboost_system -O6

clean:
	rm -rf index query