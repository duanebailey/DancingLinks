all:	cover tumble reshape flatten shik

cover:	cover.c
	gcc -O3 -o cover cover.c

shik:	shik.c
	gcc -g -o shik shik.c

tumble:	tumble.c
	gcc -o tumble tumble.c

reshape:	reshape.c
	gcc -o reshape reshape.c

flatten:	flatten.c
	gcc -o flatten flatten.c

clean:
	@rm -f *~ \#*
	@echo Yup.

realclean:	clean
	@rm -f cover tumble reshape flatten
	@echo Yup.