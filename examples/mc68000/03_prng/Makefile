PROJECT_NAME = prng

all: $(PROJECT_NAME).decb

$(PROJECT_NAME).decb: base.s ../../../rom/mc68000/definitions.i base.s main.s
	vasmm68k_mot -no-opt -m68000 -Fbin -L $(PROJECT_NAME).list -o $(PROJECT_NAME).decb base.s

clean:
	rm $(PROJECT_NAME).decb $(PROJECT_NAME).list
