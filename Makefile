all:
	gcc -o exec project_A_3887.c -lpthread -w -lm
	#gcc -o exec 1st_ass_csd3887.c
	# -lm is for pow function

clean:
	rm exec

