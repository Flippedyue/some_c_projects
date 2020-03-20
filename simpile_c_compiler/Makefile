result: myparser.y mylexer.l TreeNode.h label.h codes.h
	bison -d  myparser.y
	flex mylexer.l
	g++ myparser.tab.c lex.yy.c TreeNode.cpp label.cpp codes.cpp
	cat input.c|./a.out 

clean:
	rm -f *.o myparser.tab.c myparser.tab.h lex.yy.c y.output y.tab.c y.tab.h res.asm a.out
