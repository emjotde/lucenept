all: query index

query: query.cpp LuceneIndex.o
	g++ query.cpp LuceneIndex.o -o query -llucene++ -lpthread -lboost_system -O6

index: index.cpp
	g++ index.cpp -o index -llucene++ -lpthread -lboost_system -O6

LuceneIndex.o: LuceneIndex.cpp LuceneIndex.h
	g++ -c LuceneIndex.cpp -llucene++ -lpthread -lboost_system -O6
clean:
	rm -rf index query