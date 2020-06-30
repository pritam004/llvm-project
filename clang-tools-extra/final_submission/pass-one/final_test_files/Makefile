
all :program1 program2 program3 program4 program5 program6 program7
program1: test1.cpp
	$(file)/pass-one test1.cpp >res.cpp
	$(file)/pass-two res.cpp >res1.cpp
	$(file)/pass-three res1.cpp >result1.cpp
	$(file)/clang result1.cpp -o result1.o

program2: test2.cpp
	$(file)/pass-one test2.cpp >res.cpp
	$(file)/pass-two res.cpp >res1.cpp
	$(file)/pass-three res1.cpp >result2.cpp
	$(file)/clang result2.cpp -o result2.o
program3: test3.cpp
	$(file)/pass-one test3.cpp >res.cpp
	$(file)/pass-two res.cpp >res1.cpp
	$(file)/pass-three res1.cpp >result3.cpp
	$(file)/clang result3.cpp -o result3.o
program4: test4.cpp
	$(file)/pass-one test4.cpp >res.cpp
	$(file)/pass-two res.cpp >res1.cpp
	$(file)/pass-three res1.cpp >result4.cpp
	$(file)/clang result4.cpp -o result4.o
program5: test5.cpp
	$(file)/pass-one test5.cpp >res.cpp
	$(file)/pass-two res.cpp >res1.cpp
	$(file)/pass-three res1.cpp >result5cpp
	$(file)/clang result5.cpp -o result5.o
program6: test6.cpp
	$(file)/pass-one test6.cpp >res.cpp
	$(file)/pass-two res.cpp >res1.cpp
	$(file)/pass-three res1.cpp >result6.cpp
	$(file)/clang result6.cpp -o result6.o

program7: test7.cpp
	$(file)/pass-one test7.cpp >res.cpp
	$(file)/pass-two res.cpp >res1.cpp
	$(file)/pass-three res1.cpp >result7.cpp
	$(file)/clang result6.cpp -o result7.o









