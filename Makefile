cover:	cover.c
	gcc -O3 -o cover cover.c

clean:
	@rm -f *~ \#*
	@echo Yup.

realclean:	clean
	@rm -f cover tumble reshape flatten
	@echo Yup.