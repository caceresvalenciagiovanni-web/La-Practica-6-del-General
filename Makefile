Gram=y.tab.c y.tab.h

all: $(Gram) 
	@gcc -o calc y.tab.c symbol.c init.c math.c vector_math.c code.c -lm
	@echo "Compilación de HOC5 completada exitosamente. Iniciando..."
	@./calc

$(Gram): hoc5.y
	@yacc -d hoc5.y

clean:
	@rm -f *.tab.* calc 
	@echo "Proyecto limpio."
